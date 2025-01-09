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
    ~AppController(); // Declare the destructor
    bool eventFilter(QObject *obj, QEvent *event) override;
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static void EnableHook();
    static void DisableHook();

public slots:
    void onKbMacrosWindowLoaded();
    void handleKeyPress(QKeyEvent *event);

private:
    void printListModelElements(QAbstractListModel *listModel);
    void executeMacro(const std::vector<std::pair<char, int>> &sequence);
    std::vector<std::pair<char, int>> parseMacroCode(const QString &macroCode);
    std::pair<QString, QString> extractTriggerKey(const QString &macroCode);
    void updateMacros(QAbstractListModel *listModel);

    QQmlApplicationEngine *engine;
    static std::unordered_map<QString, QString> triggerKeyToMacroCode;
    static HHOOK keyboardHook;
    static AppController *instance; // Add a static instance pointer
};

#endif // APPCONTROLLER_H
