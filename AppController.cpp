#include "AppController.h"
#include <QDebug>
#include <QQmlProperty>
#include <QEvent>
#include <QKeyEvent>
#include <QRegularExpression>
#include <regex>
#include <thread>
#include <chrono>
#include <windows.h>

std::unordered_map<QString, QString> AppController::triggerKeyToMacroCode;
HHOOK AppController::keyboardHook = nullptr;
AppController *AppController::instance = nullptr; // Define the instance pointer

AppController::AppController(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent), engine(engine) {
    // Install the event filter on the application instance
    qApp->installEventFilter(this);

    // Set up the global keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (!keyboardHook) {
        qDebug() << "Failed to install hook!";
    }

    instance = this; // Set the instance pointer
}

AppController::~AppController() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

void AppController::EnableHook() {
    if (!keyboardHook) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    }
}

void AppController::DisableHook() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }
}

LRESULT CALLBACK AppController::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        QString key = QKeySequence(pKeyboard->vkCode | GetKeyState(VK_CONTROL) & 0x8000 | GetKeyState(VK_SHIFT) & 0x8000 | GetKeyState(VK_MENU) & 0x8000).toString();

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            qDebug() << "Key Pressed Globally:" << key;
            if (triggerKeyToMacroCode.find(key) != triggerKeyToMacroCode.end()) {
                qDebug() << "Global Macro found for key:" << key << "with macro code:" << triggerKeyToMacroCode[key];
                // Execute the macro using the instance pointer
                QString macroCode = triggerKeyToMacroCode[key];
                auto sequence = instance->parseMacroCode(macroCode); // Use instance to call non-static member function
                instance->executeMacro(sequence); // Use instance to call non-static member function
                return 1; // Block the key event
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

bool AppController::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        QString key = QKeySequence(keyEvent->modifiers() | keyEvent->key()).toString();
        qDebug() << "Key Pressed:" << key;
        if (triggerKeyToMacroCode.find(key) != triggerKeyToMacroCode.end()) {
            qDebug() << "Macro found for key:" << key << "with macro code:" << triggerKeyToMacroCode[key];
            handleKeyPress(keyEvent);
            return true; // Event handled
        } else {
            qDebug() << "No macro found for key:" << key;
        }
    }
    return QObject::eventFilter(obj, event); // Standard event processing for other keys
}

void AppController::handleKeyPress(QKeyEvent *event) {
    QString key = QKeySequence(event->modifiers() | event->key()).toString();
    qDebug() << "Handling key press for key:" << key;
    if (triggerKeyToMacroCode.find(key) != triggerKeyToMacroCode.end()) {
        QString macroCode = triggerKeyToMacroCode[key];
        qDebug() << "Executing macro code:" << macroCode;
        auto sequence = parseMacroCode(macroCode);
        executeMacro(sequence);
    }
}

std::pair<QString, QString> AppController::extractTriggerKey(const QString &macroCode) {
    QRegularExpression regex(R"(([^->]+)->(.+))");
    QRegularExpressionMatch match = regex.match(macroCode);
    if (match.hasMatch()) {
        QString triggerKey = match.captured(1).trimmed();
        QString actions = match.captured(2).trimmed();
        return {triggerKey, actions};
    }
    return {"", macroCode}; // If the format is incorrect, return the whole macroCode as actions
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

        auto [triggerKey, actions] = extractTriggerKey(macroCode.toString());

        qDebug() << "Macro Name:" << macroName.toString()
                 << ", Macro Code:" << macroCode.toString()
                 << ", Trigger Key:" << triggerKey;
    }
}

std::vector<std::pair<char, int>> AppController::parseMacroCode(const QString &macroCode) {
    auto [triggerKey, actions] = extractTriggerKey(macroCode);
    std::vector<std::pair<char, int>> sequence;
    std::regex pattern(R"(([A-Z])\((\d+)\))");
    std::string code = actions.toStdString();
    auto matches = std::sregex_iterator(code.begin(), code.end(), pattern);
    auto end = std::sregex_iterator();

    for (std::sregex_iterator i = matches; i != end; ++i) {
        std::smatch match = *i;
        char key = match.str(1)[0];
        int delay = std::stoi(match.str(2));
        sequence.emplace_back(key, delay);
    }

    return sequence;
}

void AppController::executeMacro(const std::vector<std::pair<char, int>> &sequence) {
    // Disable the hook to prevent recursive triggering
    DisableHook();

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
    EnableHook();
}

void AppController::updateMacros(QAbstractListModel *listModel) {
    triggerKeyToMacroCode.clear();  // Clear existing mappings

    for (int i = 0; i < listModel->rowCount(); ++i) {
        QModelIndex index = listModel->index(i);
        QVariant macroCode = listModel->data(index, listModel->roleNames().key("macroCode"));

        auto [triggerKey, actions] = extractTriggerKey(macroCode.toString());

        if (!triggerKey.isEmpty() && !actions.isEmpty()) {
            qDebug() << "Storing macro for key:" << triggerKey << "with actions:" << actions;
            triggerKeyToMacroCode[triggerKey] = actions;
        }
    }

    // Optionally print the updated elements
    printListModelElements(listModel);
}
