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
    void ChckItem(QString result);
    void CheckItemOver(bool ok);
    void CheckItemNonU(QString resutl);
    void CheckItemNonUOver(bool ok);
    void SigFixed(qint64 error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void RepairOver(bool);
    void SignalProgress(qint64 entry_mark, qint32 progress);

public Q_SLOTS:
    QString GetCheckList();
    QString GetCheckListNonUniversal(qint64 error_code);
    QString CheckItem(qint64 mark);
    QString CheckItemNonUniversal(qint64 mark, QString data);
    bool RepairItem(QString repair_items);

private:
    void EmitSigFixed(qint64 error_code, bool ok, const QString &reason, qint64 solution_type, const QString &solution_context);
    void EmitSigProgress(qint64 entry_mark, qint32 progress);
    void EmitRepairOver(bool ok);

    QDBusConnection &conn_;
};

}

#endif
