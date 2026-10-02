#include <stdexcept>
#include <QDebug>
#include "complete_machine.h"
#include "diagnosis_manage.h"

namespace fault_diagnosis {

DiagnosisManage::DiagnosisManage(QObject *parent)
    : QObject(parent)
{
    Register(QSharedPointer<CompleteMachine>(new CompleteMachine(this)));
}

void DiagnosisManage::Register(const QSharedPointer<Diagnosis> &diagnosis) {
    auto mark = diagnosis->Mark();
    if (manager_.find(mark) != manager_.end())
        throw std::runtime_error("Diagnosis mark repeat");
    manager_[mark] = diagnosis;
    connect(diagnosis.get(), &Diagnosis::sig_CheckProgress, this, &DiagnosisManage::sig_CheckProgress);
    connect(diagnosis.get(), &Diagnosis::sig_RepairProgress, this, &DiagnosisManage::sig_RepairProgress);
    connect(diagnosis.get(), &Diagnosis::sig_CheckFinish, this, &DiagnosisManage::sig_CheckFinish);
    connect(diagnosis.get(), &Diagnosis::sig_Fixed, this, &DiagnosisManage::sig_Fixed);
    connect(diagnosis.get(), &Diagnosis::sig_RepairFinish, this, &DiagnosisManage::sig_RepairFinish);
}

const QMap<QString, QSharedPointer<Diagnosis>> &DiagnosisManage::Diagnoses() {
    return manager_;
}

QSharedPointer<Diagnosis> DiagnosisManage::DiagnosisWithMark(const QString &mark) {
    auto iter = manager_.find(mark);
    if (iter == manager_.end())
        return nullptr;
    else
        return iter.value();
}

}
