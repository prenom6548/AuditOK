#include "complete_machine_test.h"
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <QDBusMessage>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

namespace fault_diagnosis_test {

static const QString DBUS_PATH = "/com/kylin/kydoctor";
static const QString DBUS_INTERFACE = "com.kylin.kydoctor";

CompleteMachineTest::CompleteMachineTest(QObject *obj, QDBusConnection &conn)
    : QDBusAbstractAdaptor(obj),
      conn_(conn)
{

}

CompleteMachineTest::~CompleteMachineTest() {

}

QString CompleteMachineTest::GetCheckList() {
    std::string json = R"(
[
    {
        "class_a": "网络检测",
        "class_b": [
            {
                "Name": "网络稳定性",
                "Describe": "检测已连接网络稳定性",
                "class_b_id": "0x001"
            },
            {
                "Name": "网络连通性",
                "Describe": "检测内外网连通性是否正常",
                "class_b_id": "0x002"
            },
            {
                "Name": "网络服务",
                "Describe": "检测网络服务是否正常",
                "class_b_id": "0x003",
                "class_c": [
                    {
                        "Name": "服务状态",
                        "Describe": "检测网络服务是否被获取"
                    },
                    {
                        "Name": "服务配置",
                        "Describe": "检测网络服务是否正常开启"
                    },
                    {
                        "Name": "IP 地址",
                        "Describe": "检测 IP 地址是否正确"
                    }
                ]
            },
            {
                "Name": "网络设置",
                "Describe": "检测网络设置是否正常",
                "class_b_id": "0x004",
                "class_c": [
                    {
                        "Name": "有线网络",
                        "Describe": "检测有线网络是否正常开启"
                    },
                    {
                        "Name": "无线局域网",
                        "Describe": "检测无线局域网是否正常开启"
                    },
                    {
                        "Name": "host 文件",
                        "Describe": "检测 host 文件配置是否正常"
                    }
                ]
            },
            {
                "Name": "网络硬件配置",
                "Describe": "检测网线是否插好、网卡启动是否正常",
                "class_b_id": "0x005",
                "class_c": [
                    {
                        "Name": "网卡驱动",
                        "Describe": "检测网卡驱动是否正常"
                    },
                    {
                        "Name": "硬件连接",
                        "Describe": "检测硬件连接是否正常"
                    },
                    {
                        "Name": "网卡电源",
                        "Describe": "检测网卡电源是否正常"
                    }
                ]
            }
        ]
    }
]
)";
    return QString::fromStdString(json);
}

QString CompleteMachineTest::GetCheckListNonUniversal([[maybe_unused]]qint64 error_code) {
    std::string json = R"(
[
    {
        "Category": "软件商店错误修复",
        "Types": [
            {
                "Type": "0xF1",
                "Name": "软件包依赖错误",
                "Describe": "检测软件商店安装软件包时的依赖问题"
            },
            {
                "Type": "0xF2",
                "Name": "dpkg 错误",
                "Describe": "检测 dpkg 错误"
            }
        ]
    }
]
)";
    return QString::fromStdString(json);
}

QString CompleteMachineTest::CheckItem(qint64 mark) {
    // 随机睡 0 - 3 秒
    srand(time(NULL));
    int sec{0};
    while (sec == 0) {
        sec = rand() % 4;
    }

    QTimer::singleShot(sec * 1000, this, [&, mark]() {


        QMap<qint64, QString> maps;
        std::string x001 = R"(
{
    "Overview": "检测了 1 项，1 项正确，0 项错误",
    "class_a": "网络检测",
    "class_b": "网络稳定性",
    "class_b_id": "0x001",
    "ErrorCode": "",
    "Describe": "",
    "Details": [
    ]
}
)";
        maps[0x001] = QString::fromStdString(x001);

        std::string x002 = R"(
{
    "Overview": "检测了 1 项，0 项正确，1 项错误",
    "class_a": "网络检测",
    "class_b": "网络连通性",
    "class_b_id": "0x002",
    "ErrorCode": "0x00000000000002e1",
    "Describe": "1",
    "Details": [
    ]
}
)";
        maps[0x002] = QString::fromStdString(x002);

        std::string x003 = R"(
{
    "Overview": "检测了 3 项，1 项正确，2 项错误",
    "class_a": "网络检测",
    "class_b": "网络服务",
    "class_b_id": "0x003",
    "ErrorCode": "",
    "Describe": "",
    "Details": [
        {
            "class_c": "服务状态",
            "ErrorCode": "0x00000000000003e1",
            "Describe": "0"
        },
        {
            "class_c": "IP 地址",
            "ErrorCode": "0x00000000000003e2",
            "Describe": "0"
        }
    ]
}
)";
        maps[0x003] = QString::fromStdString(x003);


        std::string x004 = R"(
{
    "Overview": "检测了 3 项，0 项正确，3 项错误",
    "class_a": "网络检测",
    "class_b": "网络设置",
    "class_b_id": "0x004",
    "ErrorCode": "",
    "Describe": "",
    "Details": [
        {
            "class_c": "有线网络",
            "ErrorCode": "0x00000000000004e1",
            "Describe": "0"
        },
        {
            "class_c": "无线局域网",
            "ErrorCode": "0x00000000000004e2",
            "Describe": "0"
        },
        {
            "class_c": "host 文件",
            "ErrorCode": "0x00000000000004e3",
            "Describe": "host 文件格式错误，请检查 host 文件内容"
        }
    ]
}
)";
        maps[0x004] = QString::fromStdString(x004);

        std::string x005 = R"(
{
    "Overview": "检测了 3 项，2 项正确，1 项错误",
    "class_a": "网络检测",
    "class_b": "网络硬件配置",
    "class_b_id": "0x005",
    "ErrorCode": "",
    "Describe": "",
    "Details": [
        {
            "class_c": "网卡电源",
            "ErrorCode": "0x00000000000005e1",
            "Describe": "测试超长文本，啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊"
        }
    ]
}
)";
        maps[0x005] = QString::fromStdString(x005);

        // 构建 D-Bus 信号
        auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "CheckItem");
        message << maps[mark];
        conn_.send(message);

        // 发送检测完成信号
        auto check_finish_message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "CheckItemOver");
        check_finish_message << true;
        conn_.send(check_finish_message);
    });

    return "";
}

QString CompleteMachineTest::CheckItemNonUniversal(qint64 mark, [[maybe_unused]]QString data) {
    srand(time(NULL));
    int sec{0};
    while (sec == 0) {
        sec = rand() % 4;
    }

    // 随机睡 0 - 3 秒
    QTimer::singleShot(sec * 1000, this, [this, mark]() {
        std::map<qint64, std::string> maps;
        std::string x1100010200000001 = R"(
{
    "Type": "0x1100010200000001",
    "Overview": "检测了 1 项，1 项正确，0项错误",
    "Details": [
        {
            "Name": "Upgrade",
            "ErrorCode": "0x1100010200000001",
            "Describe": "缺少必要的依赖文件",
            "Context": "",
            "CanRepair": "4",
            "NetworkState": "1"
        }
    ]
}
)";
        maps.insert({0x1100010200000001, x1100010200000001});

        std::string x1000010200000001 = R"(
{
    "Type": "0x1000010200000001",
    "Overview": "检测 1 项，1 项正确，0 项错误",
    "Details": [
        {
            "Name": "Software",
            "ErrorCode": "0x1000010200000001",
            "Describe": "安装包依赖冲突",
            "Context": "Delete:kylin-os-manager kydoctor-server\r\nDowngrading:kylin-video kylin-ipmsg",
            "CanRepair": "5",
            "NetworkState": "0"
        }
    ]
}
)";
        maps.insert({0x1000010200000001, x1000010200000001});

        auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "CheckItemNonU");
        message.setArguments({QString::fromStdString(maps[mark])});
        conn_.send(message);

        auto finish_message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "CheckItemNonUOver");
        message.setArguments({true});
        conn_.send(finish_message);
    });

    return "";
}

bool CompleteMachineTest::RepairItem(QString repair_items) {
    QTimer::singleShot(0, this, [&, repair_items]() {
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(repair_items.toLocal8Bit(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCritical() << "Repair json parse fail: " << error.errorString();
            EmitRepairOver(false);
            return;
        }

        if (!document.isArray()) {
            qCritical() << "Repair json is not a array.";
            EmitRepairOver(false);
            return;
        }

        const auto array = document.array();
        for (int i = 0; i < array.size(); i++) {
            if (!array.at(i).isObject()) {
                continue;
            }
            const auto object = array.at(i).toObject();
            if (object.contains("ErrorCode")) {
                int sec{0};
                srand(time(NULL));
                while (sec == 0) {
                    sec = rand() % 4;
                }
                sleep(sec);

                bool ok;
                auto error_code = object["ErrorCode"].toString().toULongLong(&ok, 16);
                if (!ok) {
                    qCritical() << "Error code QString to int fail." << repair_items;
                    continue;
                }
                // 测试不进行手动修复的修复失败场景
                if (error_code == 0x00000000000003e1) {
                    EmitSigFixed(error_code, false, "测试修复失败场景", 0, "");
                    continue;
                }
                // 测试将自动修复的错误项转换成手动修复
                //if (error_code == 0x01E1) {
                //    EmitSigFixed(error_code, false, "测试修复失败，转换成手动修复", 0xE000000000000000, "测试手动修复");
                //    continue;
                //}
                EmitSigFixed(error_code, true, "", 0, "");
            }
        }
        EmitRepairOver(true);
    });

    return true;
}

void CompleteMachineTest::EmitSigFixed(qint64 error_code, bool ok, const QString &reason, qint64 soluton_type, const QString &solution_context) {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "SigFixed");
    message << error_code;
    message << ok;
    message << reason;
    message << soluton_type;
    message << solution_context;
    conn_.send(message);
}

void CompleteMachineTest::EmitSigProgress(qint64 entry_mark, qint32 progress) {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "SignalProgress");
    message << entry_mark;
    message << progress;
    conn_.send(message);
}

void CompleteMachineTest::EmitRepairOver(bool ok) {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "RepairOver");
    message << ok;
    conn_.send(message);
}

}
