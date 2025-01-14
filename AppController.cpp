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
std::unordered_map<QString, std::list<int>> parsedActionMap;
HHOOK AppController::keyboardHook = nullptr;
AppController* AppController::instance = nullptr;
HWND AppController::appWindowHandle = nullptr;
bool AppController::isMacroExecuting = false; // Initialize the flag
std::unordered_map<std::string, DWORD> specialKeys = {
    {"F1", VK_F1},
    {"F2", VK_F2},
    {"F3", VK_F3},
    {"F4", VK_F4},
    {"F5", VK_F5},
    {"F6", VK_F6},
    {"F7", VK_F7},
    {"F8", VK_F8},
    {"F9", VK_F9},
    {"F10", VK_F10},
    {"F11", VK_F11},
    {"F12", VK_F12},
    {"ESC", VK_ESCAPE},
    {"TAB", VK_TAB},
    {"CAPSLOCK", VK_CAPITAL},
    {"SCROLLLOCK", VK_SCROLL},
    {"PRINTSCREEN", VK_SNAPSHOT},
    {"PAUSE", VK_PAUSE},
    {"INSERT", VK_INSERT},
    {"DELETE", VK_DELETE},
    {"HOME", VK_HOME},
    {"END", VK_END},
    {"PAGEUP", VK_PRIOR},
    {"PAGEDOWN", VK_NEXT},
    {"UP", VK_UP},
    {"DOWN", VK_DOWN},
    {"LEFT", VK_LEFT},
    {"RIGHT", VK_RIGHT}
};


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
            if (parsedActionMap.find(key) != parsedActionMap.end()) {
                qDebug() << "Global Macro found for key:" << key << "with macro code:" << macroList[key];
                // Ensure the application is not in focus before executing the macro
                if (foregroundWindow != appWindowHandle) {
                    //qDebug() << "Executing macro for key:" << key;
                    //QString macroCode = macroList[key];
                    instance->executeMacro(parsedActionMap.find(key)->second);
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
void AppController::executeMacro(std::list<int> actions) {
    // Disable the hook to prevent recursive triggering
    DisableHook();
    isMacroExecuting = true;

    // 0x01 = ALT, 0x02 = SHIFT, 0x03 = CTRL
    DWORD modifiers = 0;
    bool keyPressed = false;

    INPUT inputALT = {0}, inputSHIFT = {0}, inputCTRL = {0};

    // Initialize the inputs
    inputALT.type = inputSHIFT.type = inputCTRL.type = INPUT_KEYBOARD;
    inputALT.ki.wVk = VK_MENU; // Virtual key code for Alt
    inputSHIFT.ki.wVk = VK_SHIFT; // Virtual key code for Shift
    inputCTRL.ki.wVk = VK_CONTROL; // Virtual key code for Ctrl

    // CTRL+SHIFT+D(20)K(30)A(10)
    for(auto action: actions) {

        switch (action) {
            case VK_MENU:
                modifiers |= 0x01;
                SendInput(1, &inputALT, sizeof(INPUT));
                continue;
            case VK_SHIFT:
                SendInput(1, &inputALT, sizeof(INPUT));
                modifiers |= 0x02;
                continue;
            case VK_CONTROL:
                SendInput(1, &inputALT, sizeof(INPUT));
                modifiers |= 0x03;
                continue;
            default:
                break;
        }

        if(!keyPressed) {
            // Simulate key press
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = action;
            SendInput(1, &input, sizeof(INPUT));

            // Simulate key release
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }

        if(keyPressed) {
            if(modifiers & 0x01) {
                inputALT.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &inputALT, sizeof(INPUT));
                inputALT.ki.dwFlags = 0;
            }
            if(modifiers & 0x02) {
                inputSHIFT.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &inputSHIFT, sizeof(INPUT));
                inputSHIFT.ki.dwFlags = 0;
            }
            if(modifiers & 0x03) {
                inputCTRL.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &inputCTRL, sizeof(INPUT));
                inputCTRL.ki.dwFlags = 0;
            }

            keyPressed = false;
            modifiers = 0;
            if(action > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(action));
            continue;
        }

        keyPressed = true;
    }

    // Re-enable the hook after execution
    isMacroExecuting = false;
    EnableHook();
}

std::list<int> seperateAndConvertActions(QString& QSTR_actions) {
    std::list<int> result;
    bool parsingDelay = false;
    bool modifier = false;
    int x = 0;
    // CTRL+D(20)K(30)A(10)
    std::string actions = QSTR_actions.toStdString();
    std::string actionKey = "";
    std::string modifierKey = "";
    std::string delaystr = "";

    for (int x = 0; x < actions.length(); x++) {
        if (actions[x] == ')') {
            result.push_back(std::stoi(delaystr));
            delaystr = "";
            parsingDelay = false;
            continue;
        }

        if (parsingDelay) {
            delaystr += actions[x];
            continue;
        }

        if (actions[x] == '+') {
            result.push_back(actionKey == "CTRL" ? VK_CONTROL : actionKey == "SHIFT" ? VK_SHIFT : VK_MENU);
            actionKey = "";
            continue;
        }

        if (actions[x] == '(') {
            if (specialKeys.find(actionKey) != specialKeys.end()) {
                result.push_back(specialKeys[actionKey]);
            }
            else {
                result.push_back(actionKey[0]);
            }
            actionKey = "";
            parsingDelay = true;
            continue;
        }

        actionKey += actions[x];

    }

    return result;
}

void AppController::updateMacros(QAbstractListModel *listModel) {
    parsedActionMap.clear();  // Clear existing mappings

    for (int i = 0; i < listModel->rowCount(); ++i) {
        QModelIndex index = listModel->index(i);
        QVariant macroCode = listModel->data(index, listModel->roleNames().key("macroCode"));

        std::pair<QString, QString> macro = seperateTriggerAndActions(macroCode.toString());
        parsedActionMap[macro.first] = seperateAndConvertActions(macro.second);
        /* macro.first -> triggerkey, macro.second -> the actions
        if (!macro.first.isEmpty() && !macro.second.isEmpty()) {
            qDebug() << "Storing macro for key:" << macro.first << "with actions:" << macro.second;
            macroList[macro.first] = macro.second;
        }*/
    }

    // Optionally print the updated elements
    printListModelElements(listModel);
}
