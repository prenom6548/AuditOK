#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_MAINWINDOW_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_MAINWINDOW_H

#include <tuple>
#include <QWidget>
#include <QPushButton>
#include <QMap>
#include <QTreeWidget>
#include <QPaintEvent>
#include <QPair>
#include <kprogressbar.h>
#include "text_label.h"
#include "icon_label.h"
#include "diagnosis_group_widget.h"
#include "diagnosis_entry_widget.h"
#include "repair_specific_error_widget.h"
#include "defines.h"

namespace fault_diagnosis {

class DescribeGuard;

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    void SetMode(enum Mode mode);
    void SetSpecificError(RepairSpecificErrorWidget *widget, enum SpecificErrorModule module, QString path, QString error_code, const QString &error_data);
    void SetSpecificDiagnosisType(enum DiagnosisType diagnosis_type);
    bool Running() const;

Q_SIGNALS:
    void sig_DiagnosticsList(fault_diagnosis::Mode mode, QVariant data = QVariant());
    void sig_Check(fault_diagnosis::CheckEntry check_entry);
    void sig_Repair(fault_diagnosis::RepairEntry repair_entry);
    void sig_StartDiagnosis(enum fault_diagnosis::Mode mode);
    void sig_Return();

public Q_SLOTS:
    void on_DiagnosticsListFinished(fault_diagnosis::CategoryDiagnosticEntry category_diagnostic_entrys);
    void on_StartDiagnosis();
    void on_Checked(QString diagnosis_mark, fault_diagnosis::DiagnosticResult diagnostic_result);
    void on_CheckFinished(QString diagnosis_mark, QString entry_mark, bool ok);
    void on_Fixed(QString diagnosis_mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void on_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result);
    //void on_CheckProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    //void on_RepairProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);

private Q_SLOTS:
    void on_Expanded(bool is_expanded);
    void on_SubitemEntryWidgetSelectChange();
    void on_DiagnosisEntryWidgetSelectChange();
    void on_StartRepair();
    void on_Cancel();
    void on_SpecificErrorRepair(QString error_code);

private:
    friend class DescribeGuard;

    enum class Status {
        kNoStatus,
        kDiagnosising,
        kDiagnosisCanceling,
        kDiagnosisCancelComplete,
        kDiagnosisComplete,
        kRepairing,
        kRepairCanceling,
        kRepairCancelComplete,
        kRepairComplete
    };

    void SwitchToDiagnosing();
    void SwitchToDiagnosisCanceling();
    void SwitchToDiagnosisCancelComplete();
    void SwitchToDiagnosisComplete();
    void SwitchToRepairing();
    void SwitchToRepairCanceling();
    void SwitchToRepairCancelComplete();
    void SwitchToRepairComplete();
    void Reset();
    void UpdateStatusDetailLabel();
    void NextDiagnosis();
    // 分类，基础检测项 item，基础检测项 widget
    std::tuple<QString, QTreeWidgetItem *, DiagnosisEntryWidget *> GetDiagnosisEntry(const QString &diagnosis_mark, const QString &diagnosis_entry_mark);
    QPair<QTreeWidgetItem *, DiagnosisEntryWidget *> GetDiagnosisEntryWithCategory(const QString &category, const QString &diagnosis_mark, const QString &diagnosis_entry_mark);
    DiagnosisGroupWidget *GetDiagnosisGroupWidget(const QString &category);
    bool CategoryExistError(const QString &category);
    void NextRepair();
    bool CategoryFixAll(const QString &category);
    QString CategoryIcon(const QString &category);
    int AllErrors() const;
    QString SystemVersion() const;
    QString SystemArch() const;
    // 修复指定错误情况下的处理函数
    void HandleSpecificErrorChecked(const DiagnosticResult &diagnostic_result);
    void HandleSpecificErrorCheckFinished(const QString entry_mark, bool ok);
    void HandleSpecificErrorFixed(bool ok);
    void HandleSpecificErrorRepairFinish(bool ok);

    QPushButton *      status_icon_btn_;
    K::TextLabel *     status_describe_label_;
    K::TextLabel *     status_detail_label_;
    QPushButton *      cancel_btn_;
    QPushButton *      return_btn_;
    QPushButton *      diagnosis_btn_;
    QPushButton *      repair_btn_;
    kdk::KProgressBar *progress_bar_;
    QTreeWidget *      diagnosis_information_widget_;

    bool cancel_;                             // 记录是否要取消
    int  need_diagnosis_total_;               // 用于记录需要诊断的总项数
    int  diagnosed_count_;                    // 用于记录总共已经诊断的数量
    int  diagnosis_group_diagnosed_count_;    // 用于记录每一个诊断组已诊断的数量
    int  diagnostic_error_count_;             // 用于记录诊断出的错误数量
    int  diagnosed_success_count_;            // 用于记录诊断成功的诊断项数量，用于埋点功能
    int  diagnosed_fail_count_;               // 用于记录诊断失败的诊断项数量，用于埋点功能
    QHash<QString, QList<DiagnosticEntry>> need_diagnosis_;

    int error_count_;            // 用于记录总的错误数量
    int need_repair_total_;      // 用于记录需要修复的总项数
    int fixed_count_;            // 用于记录修复完成的数量
    int fixed_success_count_;    // 用于记录修复成功的数量，用于埋点功能
    int fixed_fail_count_;       // 用于记录修复失败的数量，用于埋点功能
    QHash<QString, QList<RepairEntry>> need_repairs_;

    enum MainWindow::Status      status_;
    enum Mode                    mode_;                             // 目前只用于根据模式来获取对应的诊断项信息
    enum DiagnosisType           diagnosis_type_;                   // 用于记录诊断指定类型模式下的诊断类型
    QString                      kSystemVersion;                    // 系统版本，用于埋点功能
    QString                      kSystemArch;                       // 系统架构，用于埋点功能
    enum SpecificErrorModule     specific_error_module_;            // 用于记录修复指定错误的组件
    QString                      specific_error_report_path_;       // 用于记录修复指定错误时的上报日志路径
    QString                      specific_error_code_;              // 用于记录修复指定错误的错误码
    QString                      specific_error_data_;              // 用于记录修复指定错误的错误数据
    // TODO: 此处改为 std::weak_ptr，不需要在该类中维护其声生命周期
    RepairSpecificErrorWidget    *specific_error_widget_;           // 用于记录修复指定错误的界面指针
    CategoryDiagnosticEntry      category_diagnostic_entrys_;       // 后端返回的分类和对应的所有诊断项
};

class DescribeGuard final {
public:
    DescribeGuard(MainWindow &mainwindow, const QString &category)
        : mainwindow_(mainwindow),
          category_(category)
    {

    }
    ~DescribeGuard() {
        mainwindow_.UpdateStatusDetailLabel();

        // 更新诊断组已检测项的文案
        auto *diagnosis_group_widget = mainwindow_.GetDiagnosisGroupWidget(category_);
        if (diagnosis_group_widget) {
            diagnosis_group_widget->SetTotal(mainwindow_.diagnosis_group_diagnosed_count_);
        }

        // 更新进度
        if (mainwindow_.need_diagnosis_total_) {
            mainwindow_.progress_bar_->setValue(static_cast<double>(mainwindow_.diagnosed_count_) / mainwindow_.need_diagnosis_total_ * 100);
        }
    }

private:
    MainWindow &mainwindow_;
    QString     category_;
};

}

#endif
