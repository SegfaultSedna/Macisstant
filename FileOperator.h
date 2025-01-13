#ifndef FILEOPERATOR_H
#define FILEOPERATOR_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class FileOperator : public QObject
{
    Q_OBJECT
public:
    explicit FileOperator(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE bool saveToFile(const QString &filePath, const QString &data) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << filePath << "Error:" << file.errorString();
            return false;
        }

        QTextStream out(&file);
        out << data;
        file.close();
        qDebug() << "File saved successfully to" << filePath;
        return true;
    }

    Q_INVOKABLE QVariantList importFromFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << filePath << "Error:" << file.errorString();
            return QVariantList();
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
        if (!doc.isArray()) {
            qDebug() << "Invalid JSON format in file:" << filePath;
            return QVariantList();
        }

        QJsonArray jsonArray = doc.array();
        QVariantList dataList;
        for (const QJsonValue &value : jsonArray) {
            if (value.isObject()) {
                dataList.append(value.toObject().toVariantMap());
            }
        }

        return dataList;
    }
};
#endif // FileOperator_H
