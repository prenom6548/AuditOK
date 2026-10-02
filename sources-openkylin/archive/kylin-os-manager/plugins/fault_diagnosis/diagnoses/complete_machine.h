#include <QSharedPointer>
#include <QtDBus/QDBusInterface>
#include "diagnosis.h"

namespace fault_diagnosis {

class CompleteMachine: public Diagnosis {
    Q_OBJECT
public:
    CompleteMachine(QObject *parent = nullptr);
    ~CompleteMachine();
    virtual QString Mark() override;
    virtual bool DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories, QVariant data = QVariant()) override;
    virtual bool Check(const QString &base_diagnostic_mark, QVariant data = QVariant()) override;
    virtual bool Repair(const QString &base_diagnostic_mark, const QList<RepairItem> &repair_items) override;

private Q_SLOTS:
    void on_CheckProgress(qint64 base_diagnostic_mark, qint32 progress);
    void on_RepairProgress(qint64 base_diagnostic_mark, qint32 progress);
    void on_Checked(QString check_result);
    void on_CheckFinish(bool ok);
    void on_Fixed(qint64 error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void on_RepairFinish(bool ok);

private:
    void Connect();
    void Disconnect();

    QString current_diagnosis_base_diagnostic_mark_;
    QString current_repair_base_diagnostic_mark_;
    QDBusInterface *interface_;
};

}
