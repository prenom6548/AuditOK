#include <QCoreApplication>
#include <QDebug>
#include "complete_machine_test.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    auto conn = QDBusConnection::systemBus();
    if (!conn.registerService("com.kylin.kydoctor")) {
        qCritical() << "Register D-Bus service fail.";
        return -1;
    }
    QObject obj;
    fault_diagnosis_test::CompleteMachineTest complete_machine_test(&obj, conn);
    if (!conn.registerObject("/com/kylin/kydoctor", &complete_machine_test, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qCritical() << "Register D-Bus object fail.";
        return -1;
    }

    return app.exec();
}
