#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QAbstractListModel>
#include <QKeyEvent>
#include <vector>
#include <utility>
#include <unordered_map>
#include <windows.h>

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QQmlApplicationEngine *engine, QObject *parent = nullptr);
    ~AppController();
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static void EnableHook();
    static void DisableHook();

public slots:
    void onKbMacrosWindowLoaded();

private:
    void printListModelElements(QAbstractListModel *listModel);
    void executeMacro(QString& QSTR_actions);
    std::vector<std::pair<char, int>> parseActions(const QString &macroCode);
    std::pair<QString, QString> seperateTriggerAndActions(const QString &macroCode);
    void updateMacros(QAbstractListModel *listModel);
    void initializeWindowHandle();
    QQmlApplicationEngine *engine;
    static std::unordered_map<QString, QString> macroList;
    static HHOOK keyboardHook;
    static AppController *instance;
    static HWND appWindowHandle;
    static bool isMacroExecuting; // Flag to track macro execution
};

#endif // APPCONTROLLER_H
