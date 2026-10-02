#include "blacklist.h"
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

void Blacklist::Add(QString exec_name) {
    QByteArray marshall_data;

    QFile file(Path());
    if (file.exists()) {
        do {
            if (!file.open(QIODevice::ReadOnly)) {
                qCritical() << "Add blacklist open(read) file fail";
                break;
            }
            auto unmarshall_data = file.readAll();
            file.close();

            QJsonParseError error;
            auto doc = QJsonDocument::fromJson(unmarshall_data, &error);
            if (doc.isNull()) {
                qCritical() << "Parse blacklist fail: " << error.errorString();
                break;
            }
            if (!doc.isObject()) {
                qCritical() << "Blacklist format error";
                break;
            }

            auto object = doc.object();
            object[exec_name] = CurrentDataTime();

            QJsonDocument marshall_doc(object);
            marshall_data = marshall_doc.toJson();
        } while (0);
    }

    if (marshall_data.isEmpty()) {
        // 文件不存在或者文件内容错误的情况
        QJsonObject object;
        object.insert(exec_name, CurrentDataTime());

        QJsonDocument marshall_doc(object);
        marshall_data = marshall_doc.toJson();
    }

    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Add blacklist open(write) file fail";
        return;
    }

    auto size = file.write(marshall_data);
    if (size != marshall_data.size()) {
        qCritical() << "Add blacklist write file fail";
    }
    file.close();
}

void Blacklist::Remove(QString exec_name) {
    QFile file(Path());
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Remove blacklist open file fail";
        return;
    }
    auto unmarshall_data = file.readAll();
    file.close();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(unmarshall_data, &error);
    if (doc.isEmpty()) {
        qCritical() << "Remove blacklist parse fail: " << error.errorString();
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Remove blacklist format error";
        return;
    }

    auto object = doc.object();
    auto iter = object.find(exec_name);
    if (iter == object.end()) {
        return;
    }
    object.erase(iter);

    QJsonDocument marshall_doc(object);
    auto marshall_data = marshall_doc.toJson();

    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Remove blacklist open(write) file fail";
        return;
    }

    auto size = file.write(marshall_data);
    if (size != marshall_data.size()) {
        qCritical() << "Remove blacklist write file fail";
    }
    file.close();
}

bool Blacklist::Contain(QString exec_name) {
    QFile file(Path());
    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Contain blacklist open file fail";
        return false;
    }
    auto unmarshall_data = file.readAll();
    file.close();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(unmarshall_data);
    if (doc.isNull()) {
        qCritical() << "Contain blacklist parse fail: " << error.errorString();
        return false;
    }
    if (!doc.isObject()) {
        qCritical() << "Contain blacklist format error";
        return false;
    }

    auto object = doc.object();
    auto iter = object.find(exec_name);
    if (iter == object.end()) {
        return false;
    }

    // 判断是否超过 7 天
    auto date = iter.value().toString();
    auto record_date = QDateTime::fromString(date, "yyyy-MM-dd HH:mm:ss");
    auto current_date = QDateTime::currentDateTime();
    if (record_date.secsTo(current_date) > 7 * 24 * 60 * 60) {
        // 超过 7 天
        return false;
    }

    return true;
}

QString Blacklist::Path() {
    return QString("%1/.kylin-os-manager/global_feedback_blacklist.json")
           .arg(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation));
}

QString Blacklist::CurrentDataTime() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}
