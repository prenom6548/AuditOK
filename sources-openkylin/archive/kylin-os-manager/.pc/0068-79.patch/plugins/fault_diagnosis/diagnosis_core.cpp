#include <QDebug>
#include "diagnosis_core.h"

namespace fault_diagnosis {

DiagnosisCore::DiagnosisCore()
    : diagnosis_manager_(new DiagnosisManage(this))
{
    connect(diagnosis_manager_, &DiagnosisManage::sig_CheckProgress, this, &DiagnosisCore::sig_CheckProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_RepairProgress, this, &DiagnosisCore::sig_RepairProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_Fixed, this, &DiagnosisCore::sig_Fixed);
    connect(diagnosis_manager_, &DiagnosisManage::sig_CheckFinish, this, &DiagnosisCore::on_CheckFinish);
    connect(diagnosis_manager_, &DiagnosisManage::sig_RepairFinish, this, &DiagnosisCore::on_RepairFinish);
}

void DiagnosisCore::on_DiagnosticsList() {
    Database::Instance();

    CategoryDiagnosticEntry category_diagnostic_entrys;

    auto diagnoses = diagnosis_manager_->Diagnoses();
    for (const auto &d: diagnoses) {
        QString diagnosis_mark = d->Mark();
        QList<DiagnosticCategory> categories;
        d->DiagnosticsList(categories);

        // 对每个诊断实例所支持的诊断项，按照分类合并
        for (const auto &c: categories) {
            for (const auto &i: c.diagnostic_items) {
                DiagnosticEntry diagnostic_entry;
                diagnostic_entry.diagnosis_mark = diagnosis_mark;
                diagnostic_entry.item = i;
                category_diagnostic_entrys[c.category].push_back(diagnostic_entry);
            }
        }
    }

    Q_EMIT sig_DiagnosticsListFinished(category_diagnostic_entrys);
}

void DiagnosisCore::on_Check(fault_diagnosis::CheckEntry check_entry) {
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(check_entry.diagnosis_mark);
    if (!diagnosis)
        throw std::runtime_error("diagnosis mark is not exist");

    current_check_entry_ = check_entry;
    if (!diagnosis->Check(check_entry.entry_mark)) {
        DiagnosticResult diagnostic_result;
        diagnostic_result.overview = tr("Diagnosis error");
        on_CheckFinish(false, diagnostic_result);
    }
}

void DiagnosisCore::on_Repair(fault_diagnosis::RepairEntry repair_entry) {
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(repair_entry.diagnosis_mark);
    if (!diagnosis)
        throw std::runtime_error("diagnosis mark is not exist");

    QList<RepairItem> repair_items;
    for (auto iter = repair_entry.errors.cbegin(); iter != repair_entry.errors.cend(); iter++) {
        RepairItem repair_item;
        auto solution_record = iter.value();
        repair_item.error_code = solution_record.error_code;
        repair_items.push_back(repair_item);
    }

    current_repair_entry_ = repair_entry;
    if (!diagnosis->Repair(repair_items)) {
        on_RepairFinish(false);
    }
}

void DiagnosisCore::on_CheckFinish(bool ok, fault_diagnosis::DiagnosticResult diagnostic_result) {
    CheckEntryResult check_entry_result;
    check_entry_result.ok = ok;
    check_entry_result.check = current_check_entry_;
    check_entry_result.result = diagnostic_result;
    if (!diagnostic_result.details.empty()) {
        for (const auto &e: diagnostic_result.details) {
            auto error_code = e.error_code;
            Database::SolutionRecord solution_record;
            if (!Database::SolutionRecordWithErrorCode(error_code, solution_record)) {
                qCritical() << "Fail to query the solution to the error code";
                continue;
            }
            check_entry_result.solution_records.insert(error_code, solution_record);
        }
    }

    // 清空当前检测项
    current_check_entry_.category = "";
    current_check_entry_.diagnosis_mark = "";
    current_check_entry_.entry_mark = -1;

    Q_EMIT sig_CheckFinished(check_entry_result);
}

void DiagnosisCore::on_RepairFinish(bool ok) {
    RepairEntryResult repair_entry_result;
    repair_entry_result.ok = ok;
    if (!ok) {
        repair_entry_result.reason = tr("Repair fail");
    }
    repair_entry_result.repair = current_repair_entry_;

    // 清空当前修复项
    current_repair_entry_.category = "";
    current_repair_entry_.diagnosis_mark = "";
    current_repair_entry_.entry_mark = -1;
    current_repair_entry_.errors.clear();

    Q_EMIT sig_RepairFinished(repair_entry_result);
}

}
