#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_MAINWINDOW_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_MAINWINDOW_H

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
#include "error_entry_widget.h"
#include "defines.h"

namespace fault_diagnosis {

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

Q_SIGNALS:
    void sig_DiagnosticsList();
    void sig_Check(fault_diagnosis::CheckEntry check_entry);
    void sig_Repair(fault_diagnosis::RepairEntry repair_entry);
    void sig_Return();

public Q_SLOTS:
    void on_DiagnosticsListFinished(fault_diagnosis::CategoryDiagnosticEntry category_diagnostic_entrys);
    void on_StartDiagnosis();
    void on_CheckFinished(fault_diagnosis::CheckEntryResult check_entry_result);
    void on_Fixed(QString diagnosis_mark, qint64 error_code, bool ok, QString reason);
    void on_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result);
    //void on_CheckProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    //void on_RepairProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);

private Q_SLOTS:
    void on_Expanded(bool is_expanded);
    void on_ErrorEntryWidgetSelectChange();
    void on_DiagnosisEntryWidgetSelectChange();
    void on_StartRepair();
    void on_Cancel();

private:
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
    QPair<QTreeWidgetItem *, DiagnosisEntryWidget *> GetDiagnosisEntryWithCategory(const QString &category, const QString &diagnosis_mark, qint64 diagnosis_entry_mark);
    DiagnosisGroupWidget *GetDiagnosisGroupWidget(const QString &category);
    ErrorEntryWidget *GetErrorEntryWidget(const QString &diagnosis_mark, qint64 error_code);
    bool CategoryExistError(const QString &category);
    void NextRepair();
    bool CategoryFixAll(const QString &category);
    QString CategoryIcon(const QString &category);
    int AllErrors() const;

    QPushButton *status_icon_btn_;
    K::TextLabel *status_describe_label_;
    K::TextLabel *status_detail_label_;
    QPushButton *cancel_btn_;
    QPushButton *return_btn_;
    QPushButton *diagnosis_btn_;
    QPushButton *repair_btn_;
    kdk::KProgressBar *progress_bar_;
    QTreeWidget *diagnosis_information_widget_;

    MainWindow::Status status_;
    CategoryDiagnosticEntry category_diagnostic_entrys_;    // 后端返回的分类和对应的所有诊断项
    QMap<QString, QList<DiagnosticEntry>> need_diagnosis_;
    int need_diagnosis_total_;    // 用于记录需要诊断的总项数
    int diagnosed_count_;    // 用于记录总共已经诊断的数量
    int diagnostic_error_count_;    // 用于记录诊断出的错误数量
    QMap<QString, QList<RepairEntry>> need_repairs_;
    int error_count_;    // 用于记录总的错误数量
    int diagnosis_group_diagnosed_count_;    // 用于记录每一个诊断组已诊断的数量
    bool cancel_;    // 记录是否要取消
    int need_repair_total_;    // 用于记录需要修复的总项数
    int fixed_count_;    // 用于记录修复完成的数量
};

}

#endif
