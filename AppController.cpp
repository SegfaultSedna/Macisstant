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

// Function to convert virtual key code to QString
QString VirtualKeyToQString(DWORD vkCode) {
    // Get the scan code for the virtual key
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);

    // Create a buffer to hold the key name
    char keyName[128];
    // Get the key name
    int length = GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));
    if (length > 0) {
        return QString::fromLocal8Bit(keyName, length);
    } else {
        return QString("Unknown");
    }
}

LRESULT CALLBACK AppController::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        // If the CTRL is pressed (the high order bit is set), we add it to the key string
        QString key = GetKeyState(VK_CONTROL) & 0x8000 ? "Ctrl+" : "";

        if(GetKeyState(VK_SHIFT) & 0x8000)
            key += "Shift+";

        if(GetKeyState(VK_MENU) & 0x8000)
            key += "Alt+";

        key += VirtualKeyToQString(pKeyboard->vkCode);

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
                    instance->executeMacro(macroCode);
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

            QObject* macroItemModelCopy = item->findChild<QObject*>("macroItemModelCopy");
            if (macroItemModelCopy) {
                qDebug() << "macroItemModelCopy found";

                QAbstractListModel* listModel = qobject_cast<QAbstractListModel*>(macroItemModelCopy);
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
                    qDebug() << "Unable to cast macroItemModelCopy to QAbstractListModel*";
                }
            } else {
                qDebug() << "macroItemModelCopy not found";
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



// Have 2 fix:
// - if the user puts macro for e.g. on CTRL+S it doesn't do the right things because the CTRL is pressed (from the user side)
void AppController::executeMacro(QString& QSTR_actions) {
    // Disable the hook to prevent recursive triggering
    DisableHook();
    isMacroExecuting = true;

    std::string actions = QSTR_actions.toStdString();
    int x = 0;
    bool parsingDelay = false;

    INPUT inputALT = {0}, inputSHIFT = {0}, inputCTRL = {0};

    // Initialize the inputs
    inputALT.type = inputSHIFT.type = inputCTRL.type = INPUT_KEYBOARD;
    inputALT.ki.wVk = VK_MENU; // Virtual key code for Alt
    inputSHIFT.ki.wVk = VK_SHIFT; // Virtual key code for Shift
    inputCTRL.ki.wVk = VK_CONTROL; // Virtual key code for Ctrl

    // Map of key inputs and their corresponding actions
    std::unordered_map<std::string, INPUT*> keyMap = {
        {"ALT", &inputALT},
        {"SHIFT", &inputSHIFT},
        {"CTRL", &inputCTRL}
    };

    while (x < actions.length()) {
        std::string actionfull = "";
        char actionKey = 0;
        std::string delaystr = "";
        int delay = 0;

        while (x < actions.length() && actions[x] != ')') {
            if (actions[x] == '(') {
                actionKey = actions[x-1];
                parsingDelay = true;
            } else if (parsingDelay) {
                delaystr += actions[x];
            } else {
                actionfull += actions[x];
            }
            x++;
        }

        /*if (x < actions.length() && actions[x] == ')') {
            actionfull += actions[x++];
        }*/

        // Convert delay string to integer
        if (!delaystr.empty()) {
            delay = std::stoi(delaystr);
        }

        // Press the keys if they are found in the action string
        for (const auto& keyAction : keyMap) {
            if (actionfull.find(keyAction.first) != std::string::npos) {
                SendInput(1, keyAction.second, sizeof(INPUT));
            }
        }

        // Simulate key press
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VkKeyScan(actionKey);
        SendInput(1, &input, sizeof(INPUT));

        // Simulate key release
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        // Release the keys if they were pressed
        for (const auto& keyAction : keyMap) {
            if (actionfull.find(keyAction.first) != std::string::npos) {
                keyAction.second->ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, keyAction.second, sizeof(INPUT));
            }
        }

        // Wait for the specified delay
        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }

        x++;
        parsingDelay = false;
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
