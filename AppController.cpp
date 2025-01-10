#include <QApplication>
#include <QWindow>
#include <QTimer>
#include <QMainWindow>
#include "AppController.h"
#include <QDebug>
#include <QQmlProperty>
#include <QEvent>
#include <QKeyEvent>
#include <QRegularExpression>
#include <thread>
#include <chrono>
#include <windows.h>

std::unordered_map<QString, QString> AppController::macroList;
HHOOK AppController::keyboardHook = nullptr;
AppController* AppController::instance = nullptr;
HWND AppController::appWindowHandle = nullptr;
bool AppController::isMacroExecuting = false; // Initialize the flag

AppController::AppController(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent), engine(engine) {
    // Set up the global keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (!keyboardHook) {
        qDebug() << "Failed to install hook!";
    }

    instance = this;
    // Connect application state changed signal
    // Initialize the window handle once
    QTimer::singleShot(0, this, &AppController::initializeWindowHandle);
}

AppController::~AppController() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

void AppController::EnableHook() {
    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
        qDebug() << "Hook enabled";
    } else {
        qDebug() << "Hook already enabled";
    }
}

void AppController::DisableHook() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
        qDebug() << "Hook disabled";
    } else {
        qDebug() << "Hook already disabled";
    }
}

void AppController::initializeWindowHandle() {
    // Retrieve the window handle for the application's main window
    const QList<QWindow *> topLevelWindows = qApp->topLevelWindows();
    if (!topLevelWindows.isEmpty()) {
        QWindow *window = topLevelWindows.first();
        if (window) {
            appWindowHandle = reinterpret_cast<HWND>(window->winId());
            qDebug() << "App window handle initialized:" << appWindowHandle;
        }
    }
}

LRESULT CALLBACK AppController::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        QString key = QKeySequence(pKeyboard->vkCode | GetKeyState(VK_CONTROL) & 0x8000 | GetKeyState(VK_SHIFT) & 0x8000 | GetKeyState(VK_MENU) & 0x8000).toString();

        // Check if the foreground window is the application's window
        HWND foregroundWindow = GetForegroundWindow();
        if (foregroundWindow == appWindowHandle || isMacroExecuting) {
            //qDebug() << "Ignoring key event in application window or during macro execution";
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam); // Ignore key events
        }

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            qDebug() << "Key Pressed Globally:" << key;
            if (macroList.find(key) != macroList.end()) {
                qDebug() << "Global Macro found for key:" << key << "with macro code:" << macroList[key];
                // Ensure the application is not in focus before executing the macro
                if (foregroundWindow != appWindowHandle) {
                    qDebug() << "Executing macro for key:" << key;
                    QString macroCode = macroList[key];
                    auto sequence = instance->parseActions(macroCode);
                    instance->executeMacro(sequence);
                }
                return 1; // Block the key event
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

std::pair<QString, QString> AppController::seperateTriggerAndActions(const QString &macroCode) {
    QString triggerKey;
    QString actions;
    bool sw = true;
    for(int i = 0; i < macroCode.length(); i++) {
        if(macroCode[i] == '-' ) {
            i++;
            sw = false;
            continue;
        }

        if(sw)
            triggerKey += macroCode[i];
        else
            actions += macroCode[i];

    }
    return {triggerKey, actions};
}

void AppController::onKbMacrosWindowLoaded() {
    qDebug() << "kbMacrosWindowLoaded signal received.";

    if (!engine) {
        qDebug() << "Failed to get QQmlApplicationEngine.";
        return;
    }

    QObject* rootObject = engine->rootObjects().first();
    QObject* loader = rootObject->findChild<QObject*>("kbMacrosWindowLoader");

    if (loader) {
        // Read the "item" property from the loader, which holds the loaded item
        QObject* item = qvariant_cast<QObject*>(QQmlProperty::read(loader, "item"));

        if (item) {
            // Access the properties or call methods on the item
            QString objectName = item->objectName();
            qDebug() << "Loaded item objectName:" << objectName;

            QObject* macroItemModel = item->findChild<QObject*>("macroItemModel");
            if (macroItemModel) {
                qDebug() << "macroItemModel found";

                QAbstractListModel* listModel = qobject_cast<QAbstractListModel*>(macroItemModel);
                if (listModel) {
                    // Connect signals to detect changes in the ListModel
                    connect(listModel, &QAbstractListModel::rowsInserted, this, [this, listModel]() {
                        qDebug() << "Items appended to model";
                        updateMacros(listModel);
                    });

                    connect(listModel, &QAbstractListModel::rowsRemoved, this, [this, listModel]() {
                        qDebug() << "Items removed from model";
                        updateMacros(listModel);
                    });

                    connect(listModel, &QAbstractListModel::dataChanged, this, [this, listModel]() {
                        qDebug() << "Items changed in model";
                        updateMacros(listModel);
                    });

                    // Initial update of the ListModel elements
                    qDebug() << "Initial ListModel elements:";
                    updateMacros(listModel);
                } else {
                    qDebug() << "Unable to cast macroItemModel to QAbstractListModel*";
                }
            } else {
                qDebug() << "macroItemModel not found";
            }
        } else {
            qDebug() << "No item loaded";
        }
    } else {
        qDebug() << "Loader not found";
    }
}

void AppController::printListModelElements(QAbstractListModel *listModel) {
    qDebug() << "Printing ListModel elements:";
    for (int i = 0; i < listModel->rowCount(); ++i) {
        QModelIndex index = listModel->index(i);
        QVariant macroName = listModel->data(index, listModel->roleNames().key("macroName"));
        QVariant macroCode = listModel->data(index, listModel->roleNames().key("macroCode"));

        auto [triggerKey, actions] = seperateTriggerAndActions(macroCode.toString());

        qDebug() << "Macro Name:" << macroName.toString()
                 << ", Macro Code:" << macroCode.toString()
                 << ", Trigger Key:" << triggerKey;
    }
}

std::vector<std::pair<char, int>> AppController::parseActions(const QString &macroCode) {
    std::vector<std::pair<char, int>> actionSeq;
    std::string s = macroCode.toStdString();
    std::string numStr;
    char key;
    transform(s.begin(), s.end(), s.begin(),
              ::tolower);
    bool sw = true;

    for(int i = 0; i < s.length(); i++) {
        if(s[i] == '(') {
            sw = false;
            continue;
        }

        if(s[i] == ')') {
            sw = true;
            actionSeq.emplace_back(key, std::stoi(numStr));
            numStr = "";
            continue;
        }

        if(sw) {
            key = s[i];
        } else {
            numStr += s[i];
        }
    }

    return actionSeq;
}

void AppController::executeMacro(const std::vector<std::pair<char, int>> &sequence) {
    // Disable the hook to prevent recursive triggering
    DisableHook();
    isMacroExecuting = true;

    for (const auto &action : sequence) {
        char key = action.first;
        int delay = action.second;

        // Simulate key press
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VkKeyScan(key);
        SendInput(1, &input, sizeof(INPUT));

        // Simulate key release
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        // Wait for the specified delay
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Re-enable the hook after execution
    isMacroExecuting = false;
    EnableHook();
}

void AppController::updateMacros(QAbstractListModel *listModel) {
    macroList.clear();  // Clear existing mappings

    for (int i = 0; i < listModel->rowCount(); ++i) {
        QModelIndex index = listModel->index(i);
        QVariant macroCode = listModel->data(index, listModel->roleNames().key("macroCode"));

        std::pair<QString, QString> macro = seperateTriggerAndActions(macroCode.toString());

        // macro.first -> triggerkey, macro.second -> the actions
        if (!macro.first.isEmpty() && !macro.second.isEmpty()) {
            qDebug() << "Storing macro for key:" << macro.first << "with actions:" << macro.second;
            macroList[macro.first] = macro.second;
        }
    }

    // Optionally print the updated elements
    printListModelElements(listModel);
}
