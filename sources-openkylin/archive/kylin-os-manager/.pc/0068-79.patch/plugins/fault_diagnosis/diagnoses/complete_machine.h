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
    virtual bool DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories) override;
    virtual bool Check(qint64 mark) override;
    virtual bool Repair(const QList<RepairItem> &repair_items) override;

private Q_SLOTS:
    void on_CheckProgress(qint64 item_mark, qint32 progress);
    void on_RepairProgress(qint64 item_mark, qint32 progress);
    void on_CheckFinish(QString check_result);
    void on_CheckFinish(bool ok);
    void on_Fixed(qint64 error_code, bool ok, QString reason);

private:
    QSharedPointer<QDBusInterface> interface_;
};

}
