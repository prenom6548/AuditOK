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
        "Category": "硬件检测",
		"Types": [
            {
				"Type": "0x01",
				"Name": "磁盘检测",
				"Describe": "检测磁盘"
			},
			{
				"Type": "0x02",
				"Name": "网卡检测",
				"Describe": "检测网卡"
			},
			{
				"Type": "0x03",
				"Name": "屏幕检测",
				"Describe": "检测屏幕"
			}
		]
	},
	{
		"Category": "内核检测",
		"Types": [
            {
				"Type": "0x11",
				"Name": "内核漏洞检测",
				"Describe": "检测内核漏洞"
			},
			{
				"Type": "0x12",
				"Name": "内核版本检测",
				"Describe": "检测内核版本"
			},
			{
				"Type": "0x13",
				"Name": "内核模块检测",
				"Describe": "检测内核模块"
			}
		]
	},
	{
		"Category": "系统服务检测",
		"Types": [
            {
				"Type": "0x21",
				"Name": "网络服务检测",
				"Describe": "检测网络服务"
			},
			{
				"Type": "0x22",
				"Name": "蓝牙服务检测",
				"Describe": "检测蓝牙服务"
			},
			{
				"Type": "0x23",
				"Name": "声音服务检测",
				"Describe": "检测声音服务"
			}
		]
	},
	{
		"Category": "系统应用检测",
		"Types": [
            {
				"Type": "0x31",
				"Name": "压缩应用检测",
				"Describe": "检测压缩应用"
			},
			{
				"Type": "0x32",
				"Name": "扫描应用检测",
				"Describe": "检测扫描应用"
			},
			{
				"Type": "0x33",
				"Name": "打印机检测",
				"Describe": "检测打印机"
			}
		]
	},
	{
		"Category": "应用软件检测",
		"Types": [
            {
				"Type": "0x41",
				"Name": "麒麟管家检测",
				"Describe": "检测麒麟管家"
			},
			{
				"Type": "0x42",
				"Name": "启动助手检测",
				"Describe": "检测启动助手"
			},
			{
				"Type": "0x43",
				"Name": "远程协助检测",
				"Describe": "检测远程协助"
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
        std::string x01 = R"(
{
    "Overview": "检测了 20 项，17 项正确，3 项错误",
    "Details": [
        {
            "Name": "磁道检测",
            "ErrorCode": "0x01e1",
            "Describe": "磁道损坏"
        },
        {
            "Name": "磁头检测",
            "ErrorCode": "0x01e2",
            "Describe": "磁头损坏"
        },
        {
            "Name": "盘面检测",
            "ErrorCode": "0x01e3",
            "Describe": "盘面损坏"
        }
    ]
}
)";
        maps[0x01] = QString::fromStdString(x01);

        std::string x02 = R"(
{
    "Overview": "检测了 10 项，10 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x02] = QString::fromStdString(x02);

        std::string x03 = R"(
{
    "Overview": "检测了 10 项，8 项正确，2 项错误",
    "Details": [
        {
            "Name": "面板检测",
            "ErrorCode": "0x03e1",
            "Describe": "面板损坏"
        },
        {
            "Name": "光点检测",
            "ErrorCode": "0x03e2",
            "Describe": "光点损坏"
        }
    ]
}
)";
        maps[0x03] = QString::fromStdString(x03);

        // ------------ 内核检测 ------------

        std::string x11 = R"(
{
    "Overview": "检测了 5 项，5 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x11] = QString::fromStdString(x11);

        std::string x12 = R"(
{
    "Overview": "检测了 1 项，1 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x12] = QString::fromStdString(x12);

        std::string x13 = R"(
{
    "Overview": "检测了 30 项，30 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x13] = QString::fromStdString(x13);

        // ------------ 系统服务检测 ------------
        std::string x21 = R"(
{
    "Overview": "检测了 120 项，115 项正确，5 项错误",
    "Details": [
        {
            "Name": "DHCP 检测",
            "ErrorCode": "0x21e1",
            "Describe": "DHCP 配置错误"
        },
        {
            "Name": "IP 检测",
            "ErrorCode": "0x21e2",
            "Describe": "IP 地址配置错误"
        },
        {
            "Name": "DNS 检测",
            "ErrorCode": "0x21e3",
            "Describe": "DNS 配置错误"
        },
        {
            "Name": "HOST 文件检测",
            "ErrorCode": "0x21e4",
            "Describe": "HOST 文件配置错误"
        },
        {
            "Name": "MASK 检测",
            "ErrorCode": "0x21e5",
            "Describe": "MASK 配置错误"
        }
    ]
}
)";
        maps[0x21] = QString::fromStdString(x21);

        std::string x22 = R"(
{
    "Overview": "检测了 67 项，66 项正确，1 项错误",
    "Details": [
        {
            "Name": "蓝牙配置检测",
            "ErrorCode": "0x22e1",
            "Describe": "蓝牙配置错误"
        }
    ]
}
)";
        maps[0x22] = QString::fromStdString(x22);

        std::string x23 = R"(
{
    "Overview": "检测了 45 项，43 项正确，2 项错误",
    "Details": [
        {
            "Name": "声音模块检测",
            "ErrorCode": "0x23e1",
            "Describe": "声音模块检测错误"
        },
        {
            "Name": "声音配置检测",
            "ErrorCode": "0x23e2",
            "Describe": "声音配置检测错误"
        }
    ]
}
)";
        maps[0x23] = QString::fromStdString(x23);

        // ---------------- 系统应用检测（包含手动修复） ------------
        std::string x31 = R"(
{
    "Overview": "检测了 1 项，1 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x31] = QString::fromStdString(x31);

        std::string x32 = R"(
{
    "Overview": "检测了 1 项，1 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x32] = QString::fromStdString(x32);

        std::string x33 = R"(
{
    "Overview": "检测了 114 项，99 项正确，5 项错误",
    "Details": [
        {
            "Name": "网络检测",
            "ErrorCode": "0x33e1",
            "Describe": "网络配置错误"
        },
        {
            "Name": "USB 检测",
            "ErrorCode": "0x33e2",
            "Describe": "USB 错误"
        },
        {
            "Name": "蓝牙检测",
            "ErrorCode": "0x33e3",
            "Describe": "蓝牙连接错误"
        },
        {
            "Name": "墨水检测",
            "ErrorCode": "0x33e4",
            "Describe": "没有墨水，请添加墨水"
        },
        {
            "Name": "纸张检测",
            "ErrorCode": "0x33e5",
            "Describe": "没有纸张,请添加纸张"
        }
    ]
}
)";
        maps[0x33] = QString::fromStdString(x33);

        // ------------ 应用软件检测 ------------
        std::string x41 = R"(
{
    "Overview": "检测了 60 项，60 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x41] = QString::fromStdString(x41);

        std::string x42 = R"(
{
    "Overview": "检测了 10 项，10 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x42] = QString::fromStdString(x42);

        std::string x43 = R"(
{
    "Overview": "检测了 30 项，30 项正确，0 项错误",
    "Details": [
    ]
}
)";
        maps[0x43] = QString::fromStdString(x43);

        // 构建 D-Bus 信号
        auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "CheckItem");
        message << maps[mark];
        conn_.send(message);
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
                EmitSigFixed(error_code, true, "");
            }
        }
        EmitRepairOver(true);
    });

    return true;
}

void CompleteMachineTest::EmitSigFixed(qint64 error_code, bool ok, const QString &reason) {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "SigFixed");
    message << error_code;
    message << ok;
    message << reason;
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
