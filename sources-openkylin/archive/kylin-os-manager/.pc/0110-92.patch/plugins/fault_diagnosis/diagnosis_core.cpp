#include "diagnosis_core.h"
#include <QDebug>
#include <QHash>

namespace fault_diagnosis {

DiagnosisCore::DiagnosisCore()
    : diagnosis_manager_(nullptr)
{

}

void DiagnosisCore::Init() {
    if (diagnosis_manager_ == nullptr) {
        diagnosis_manager_ = new DiagnosisManage(this);
        connect(diagnosis_manager_, &DiagnosisManage::sig_CheckProgress, this, &DiagnosisCore::sig_CheckProgress);
        connect(diagnosis_manager_, &DiagnosisManage::sig_RepairProgress, this, &DiagnosisCore::sig_RepairProgress);
        connect(diagnosis_manager_, &DiagnosisManage::sig_Checked, this, &DiagnosisCore::sig_Checked);
        connect(diagnosis_manager_, &DiagnosisManage::sig_CheckFinish, this, &DiagnosisCore::sig_CheckFinished);
        connect(diagnosis_manager_, &DiagnosisManage::sig_Fixed, this, &DiagnosisCore::on_Fixed);
        connect(diagnosis_manager_, &DiagnosisManage::sig_RepairFinish, this, &DiagnosisCore::on_RepairFinish);
    }
}

// 承担根据模式筛选诊断项的功能
void DiagnosisCore::on_DiagnosticsList(fault_diagnosis::Mode mode, QVariant data) {
    Init();
    Database::Instance();

    CategoryDiagnosticEntry category_diagnostic_entrys;

    auto diagnoses = diagnosis_manager_->Diagnoses();
    for (const auto &d: diagnoses) {
        QString diagnosis_mark = d->Mark();

        // 根据 mode 对诊断实例进行筛选
        switch (mode) {
            case Mode::kDiagnosisSpecificTypeMode:
                [[fallthrough]];
            case Mode::kNormalMode: {
                if (diagnosis_mark == "complete_machine") {
                    break;
                } else {
                    continue;
                }
            }
            case Mode::kRepairSpecificErrorMode: {
                if (diagnosis_mark == "specific_error") {
                    break;
                } else {
                    continue;
                }
            }
            default: {
                continue;
            }
        }

        QList<DiagnosticCategory> categories;
        d->DiagnosticsList(categories, data);

        // 对每个诊断实例所支持的诊断项，按照分类合并
        for (const auto &c: categories) {
            for (const auto &i: c.base_diagnostic_items) {
                DiagnosticEntry diagnostic_entry;
                diagnostic_entry.diagnosis_mark = diagnosis_mark;
                diagnostic_entry.item = i;
                category_diagnostic_entrys[c.category].push_back(diagnostic_entry);
            }
        }
    }

    // 筛选诊断项
    if (mode == Mode::kDiagnosisSpecificTypeMode) {
        auto type = data.value<enum DiagnosisType>();
        switch (type) {
            case DiagnosisType::kNetwork: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != "网络检测") {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kPerformanceAndComponents: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != "系统组件与性能检测") {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            case DiagnosisType::kApplication: {
                for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end();) {
                    if (iter.key() != "应用程序检测") {
                        iter = category_diagnostic_entrys.erase(iter);
                    } else {
                        iter++;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    Q_EMIT sig_DiagnosticsListFinished(category_diagnostic_entrys);
}

void DiagnosisCore::on_Check(fault_diagnosis::CheckEntry check_entry) {
    Init();
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(check_entry.diagnosis_mark);
    if (!diagnosis) {
        throw std::runtime_error("diagnosis mark is not exist");
    }

    if (!diagnosis->Check(check_entry.entry_mark, check_entry.data)) {
        Q_EMIT sig_CheckFinished(check_entry.diagnosis_mark, check_entry.entry_mark, false);
    }
}

void DiagnosisCore::on_Repair(fault_diagnosis::RepairEntry repair_entry) {
    // 由于 SpecificError 和 CompleteMachine 实例是相同的修复逻辑
    // 所以此处做标记的替换
    auto diagnosis_mark = repair_entry.diagnosis_mark;
    if (diagnosis_mark == "specific_error") {
        diagnosis_mark = "complete_machine";
    }
    auto diagnosis = diagnosis_manager_->DiagnosisWithMark(diagnosis_mark);
    if (!diagnosis) {
        throw std::runtime_error("diagnosis mark is not exist");
    }

    QList<RepairItem> repair_items;
    for (const auto &error: repair_entry.errors) {
        RepairItem repair_item;
        repair_item.error_code = error;
        repair_items.push_back(repair_item);
    }

    if (!diagnosis->Repair(repair_entry.entry_mark, repair_items)) {
        on_RepairFinish(repair_entry.diagnosis_mark, repair_entry.entry_mark, false);
    }
}

void DiagnosisCore::on_RepairFinish(QString mark, QString base_diagnostic_mark, bool ok) {
    RepairEntryResult repair_entry_result;
    repair_entry_result.diagnosis_mark = mark;
    repair_entry_result.entry_mark = base_diagnostic_mark;
    repair_entry_result.ok = ok;
    if (!ok) {
        repair_entry_result.reason = tr("Repair fail");
    }

    Q_EMIT sig_RepairFinished(repair_entry_result);
}

void DiagnosisCore::on_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context) {
    Q_EMIT sig_Fixed(mark, error_code, ok, reason, solution_type, solution_context);
}

}
