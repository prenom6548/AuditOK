#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_REGISTER_CENTER_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_REGISTER_CENTER_H

#include <QMap>
#include <QSharedPointer>
#include "diagnosis.h"

namespace fault_diagnosis {

class DiagnosisManage: public QObject {
    Q_OBJECT
public:
    DiagnosisManage(QObject *parent = nullptr);
    const QMap<QString, QSharedPointer<Diagnosis>> &Diagnoses();
    QSharedPointer<Diagnosis> DiagnosisWithMark(const QString &mark);

Q_SIGNALS:
    void sig_CheckProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_RepairProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_Fixed(QString diagnosis_mark, qint64 error_code, bool ok, QString reason);

private:
    void Register(const QSharedPointer<Diagnosis> &diagnosis);

    QMap<QString, QSharedPointer<Diagnosis>> manager_;
};

}

#endif
