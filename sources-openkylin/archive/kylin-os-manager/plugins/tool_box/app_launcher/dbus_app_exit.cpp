#include "dbus_app_exit.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QScreen>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include "kom_buriedpoint.h"
#include "user_experience_dialog.h"

extern int g_collecting;

DBusAppExit::DBusAppExit(const QString &uuid)
    : uuid_(uuid)
{
    GetAppInformation();
}

void DBusAppExit::on_Exit(QString uuid, QString data) {
    if (uuid != uuid_) {
        return;
    }

    g_collecting = true;

    ExitGuard exit_guard;

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data.toLocal8Bit(), &error);
    if (doc.isNull()) {
        qCritical() << "D-Bus app finished data parse fail: " << error.errorString();
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "D-Bus app finished data format error";
        return;
    }

    const auto obj = doc.object();
    const auto exit_code_iter = obj.find("ret");
    if (exit_code_iter == obj.end()) {
        qCritical() << "D-Bus app finished data not contain [code] field.";
        return;
    }

    auto exit_code = exit_code_iter.value().toInt();
    if (exit_code) {
        QString desc;
        const auto desc_iter = obj.find("desc");
        if (desc_iter != obj.end()) {
            desc = desc_iter.value().toString();
        }
        qCritical() << "D-Bus app finished fail: " << desc;
    } else {
        // 如果小工具运行成功，弹窗收集用户体验信息
        if (app_information_.app_type == Category::kTroubleshooting) {
            K::UserExperienceDialog user_experience_dialog;

            // 禁止在任务栏显示图标
            const KWindowInfo window_info(user_experience_dialog.winId(), NET::WMState);
            if (!window_info.hasState(NET::SkipTaskbar) || !window_info.hasState(NET::SkipPager)) {
                KWindowSystem::setState(user_experience_dialog.winId(), NET::SkipTaskbar | NET::SkipPager);
            }

            user_experience_dialog.SetFeedBackCallback([]() {
                QProcess process;
                process.setProgram("kylin-os-manager");
                process.setArguments({"--jumpTab=ServiceSupport", "--pluginMessage=ProblemFeedback"});
                process.startDetached();
            });
            user_experience_dialog.SetFixedCallback([this]() {
                QMap<QString, QString> custom_property = {
                        {"name", app_information_.name},
                        {"uuid", uuid_},
                };
                kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kExperienceInformationCollection,
                                                kom::BuriedPoint::EventCode::kFaultRepairToolRepairSuccessful,
                                                custom_property);
            });

            // 移动屏幕中间
            const auto primary_screen = QGuiApplication::primaryScreen()->geometry();
            const auto x = primary_screen.width() / 2 - user_experience_dialog.width() / 2;
            const auto y = primary_screen.height() / 2 - user_experience_dialog.height() / 2;
            user_experience_dialog.move(x, y);
            user_experience_dialog.exec();
        }
    }
}

void DBusAppExit::GetAppInformation() {
    auto message = QDBusMessage::createMethodCall(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, "QueryAllApps");
    auto reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        return;
    }
    auto args = reply.arguments();
    if (args.empty()) {
        return;
    }

    auto json_data = args.front().toString();
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(json_data.toLocal8Bit(), &error);
    if (doc.isNull()) {
        qCritical() << "Get app information json format error";
        return;
    }
    if (!doc.isArray()) {
        qCritical() << "Get app information json is not a array";
        return;
    }

    const auto apps = doc.array();
    for (const auto &i: apps) {
        if (!i.isObject()) {
            continue;
        }

        const auto app = i.toObject();
        auto uuid_iter = app.find("uuid");
        if (uuid_iter == app.end()) {
            continue;
        }
        if (uuid_iter.value().toString() != uuid_) {
            continue;
        }

        // 找到了当前 app 的元素
        auto name_iter = app.find("name");
        if (name_iter != app.end()) {
            app_information_.name = name_iter.value().toString();
        }

        auto app_type_iter = app.find("app_type");
        if (app_type_iter != app.end()) {
            const auto category = app_type_iter.value().toString();
            if (category == "Feature") {
                app_information_.app_type = Category::kFeature;
            } else if (category == "Debug") {
                app_information_.app_type = Category::kDebug;
            } else if (category == "Troubleshooting") {
                app_information_.app_type = Category::kTroubleshooting;
            } else {
                app_information_.app_type = Category::kOther;
            }
        }

        break;
    }
}
