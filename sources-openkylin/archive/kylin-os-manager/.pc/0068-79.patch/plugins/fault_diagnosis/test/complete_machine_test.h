#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_TEST_COMPLETE_MACHINE_TEST_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_TEST_COMPLETE_MACHINE_TEST_H

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>

namespace fault_diagnosis_test {

class CompleteMachineTest: public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kydoctor")
public:
    CompleteMachineTest(QObject *obj, QDBusConnection &conn);
    ~CompleteMachineTest();

Q_SIGNALS:
    void SigFixed(qint64 error_code, bool ok, QString reason);
    void SignalProgress(qint64 entry_mark, qint32 progress);
    void ChckItem(QString);
    void CheckItemOver(bool);
    void RepairOver(bool);

public Q_SLOTS:
    QString GetCheckList();
    QString CheckItem(qint64 mark);
    bool RepairItem(QString repair_items);

private:
    void EmitSigFixed(qint64 error_code, bool ok, const QString &reason);
    void EmitSigProgress(qint64 entry_mark, qint32 progress);
    void EmitRepairOver(bool ok);

    QDBusConnection &conn_;
};

}

#endif
