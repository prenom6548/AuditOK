#include <QDebug>
#include "diagnosis_core.h"

namespace fault_diagnosis {

DiagnosisCore::DiagnosisCore()
    : diagnosis_manager_(new DiagnosisManage(this))
{
    connect(diagnosis_manager_, &DiagnosisManage::sig_CheckProgress, this, &DiagnosisCore::sig_CheckProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_RepairProgress, this, &DiagnosisCore::sig_RepairProgress);
    connect(diagnosis_manager_, &DiagnosisManage::sig_Fixed, this, &DiagnosisCore::sig_Fixed);
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

    CheckEntryResult check_entry_result;
    check_entry_result.check = check_entry;

    DiagnosticResult diagnostic_result;
    if (diagnosis->Check(check_entry.entry_mark, diagnostic_result)) {
        check_entry_result.ok = true;
        check_entry_result.result = diagnostic_result;
    } else {
        check_entry_result.ok = false;
        check_entry_result.result.overview = tr("Diagnosis error");
    }

    if (!diagnostic_result.details.isEmpty()) {
        for (const auto &e: diagnostic_result.details) {
            auto error_code = e.error_code;
            Database::SolutionRecord solution_record;
            if (!Database::SolutionRecordWithErrorCode(error_code, solution_record)) {
                qCritical() << "Fail to query the solution to the error code";
                continue;
            }
            check_entry_result.solution_records[error_code] = solution_record;
        }
    }

    Q_EMIT sig_CheckFinished(check_entry_result);
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
        repair_item.solution_type = solution_record.solution_type;
        repair_item.crc = solution_record.crc;
        repair_item.cmdline = solution_record.cmdline;
        repair_item.package_list = solution_record.package_list;
        repair_item.other = solution_record.other;
        repair_items.push_back(repair_item);
    }

    RepairEntryResult repair_entry_result;
    repair_entry_result.repair = repair_entry;
    if (diagnosis->Repair(repair_items)) {
        repair_entry_result.ok = true;
    } else {
        repair_entry_result.ok = false;
        repair_entry_result.reason = tr("Repair fail");
    }

    Q_EMIT sig_RepairFinished(repair_entry_result);
}

}
