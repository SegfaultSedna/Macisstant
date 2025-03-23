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
std::unordered_multimap<QString, std::list<int>> parsedActionMap;
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
    wchar_t keyName[128];
    // Get the key name
    int length = GetKeyNameTextW(scanCode << 16, keyName, sizeof(keyName) / sizeof(wchar_t));
    if (length > 0) {
        QString keyString = QString::fromWCharArray(keyName, length);
        // Check if the string is lowercase and convert it to uppercase if necessary
        if (keyString != keyString.toUpper()) {
            keyString = keyString.toUpper();
        }
        return keyString;
    } else {
        return QString("Unknown");
    }
}

LRESULT CALLBACK AppController::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        // Pass through injected events (from SendInput)
        if (pKeyboard->flags & LLKHF_INJECTED) {
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }

        QString key = GetKeyState(VK_CONTROL) & 0x8000 ? "CTRL+" : "";
        if (GetKeyState(VK_SHIFT) & 0x8000)
            key += "SHIFT+";
        if (GetKeyState(VK_MENU) & 0x8000)
            key += "ALT+";
        key += VirtualKeyToQString(pKeyboard->vkCode);

        HWND foregroundWindow = GetForegroundWindow();
        if (foregroundWindow == appWindowHandle) {
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            qDebug() << "Key Pressed Globally:" << key;
            auto range = parsedActionMap.equal_range(key);
            if (range.first != range.second) {
                qDebug() << "Global Macros found for key:" << key;
                for (auto it = range.first; it != range.second; ++it) {
                    // Spawn a thread for each macro
                    std::thread([this_macro = it->second]() {
                        instance->executeMacro(this_macro);
                    }).detach();
                }
                return 1; // Block the key immediately
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
                 << ", Trigger Key:" << triggerKey << "\n";
    }
}



// Have 2 fix:
// - if the user puts macro for e.g. on CTRL+S it doesn't do the right things because the CTRL is pressed (from the user side)
void AppController::executeMacro(std::list<int> actions) {
    // Remove DisableHook() and EnableHook() calls
    // Optionally suppress user modifiers (see Step 3)

    DWORD modifiers = 0;
    bool keyPressed = false;

    INPUT inputALT = {0}, inputSHIFT = {0}, inputCTRL = {0};
    inputALT.type = inputSHIFT.type = inputCTRL.type = INPUT_KEYBOARD;
    inputALT.ki.wScan = MapVirtualKey(VK_MENU, MAPVK_VK_TO_VSC);
    inputSHIFT.ki.wScan = MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC);
    inputCTRL.ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputALT.ki.dwFlags = inputSHIFT.ki.dwFlags = inputCTRL.ki.dwFlags = KEYEVENTF_SCANCODE;

    for (auto action : actions) {
        switch (action) {
        case VK_MENU:
            modifiers |= 0x01;
            SendInput(1, &inputALT, sizeof(INPUT));
            continue;
        case VK_SHIFT:
            modifiers |= 0x02;
            SendInput(1, &inputSHIFT, sizeof(INPUT));
            continue;
        case VK_CONTROL:
            modifiers |= 0x04;
            SendInput(1, &inputCTRL, sizeof(INPUT));
            continue;
        default:
            break;
        }

        if (!keyPressed) {
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = MapVirtualKey(action, MAPVK_VK_TO_VSC);
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &input, sizeof(INPUT));

            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }

        if (keyPressed) {
            if (modifiers & 0x01) {
                inputALT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                SendInput(1, &inputALT, sizeof(INPUT));
                inputALT.ki.dwFlags = KEYEVENTF_SCANCODE;
            }
            if (modifiers & 0x02) {
                inputSHIFT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                SendInput(1, &inputSHIFT, sizeof(INPUT));
                inputSHIFT.ki.dwFlags = KEYEVENTF_SCANCODE;
            }
            if (modifiers & 0x04) {
                inputCTRL.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                SendInput(1, &inputCTRL, sizeof(INPUT));
                inputCTRL.ki.dwFlags = KEYEVENTF_SCANCODE;
            }

            keyPressed = false;
            modifiers = 0;
            if (action > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(action));
            continue;
        }

        keyPressed = true;
    }
}

std::list<int> separateAndConvertActions(QString& QSTR_actions) {
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
            if(actions[x-1] == '(') {
                result.push_back(0);
            } else {
                result.push_back(std::stoi(delaystr));
            }
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
    parsedActionMap.reserve(32);
    parsedActionMap.clear();  // Clear existing mappings

    for (int i = 0; i < listModel->rowCount(); ++i) {
        QModelIndex index = listModel->index(i);
        QVariant macroCode = listModel->data(index, listModel->roleNames().key("macroCode"));

        std::pair<QString, QString> macro = seperateTriggerAndActions(macroCode.toString());
        parsedActionMap.insert(std::make_pair(macro.first, separateAndConvertActions(macro.second)));
        /* macro.first -> triggerkey, macro.second -> the actions
        if (!macro.first.isEmpty() && !macro.second.isEmpty()) {
            qDebug() << "Storing macro for key:" << macro.first << "with actions:" << macro.second;
            macroList[macro.first] = macro.second;
        }*/
    }

    // Optionally print the updated elements
    printListModelElements(listModel);
}
