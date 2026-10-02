#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_ENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_ENTRY_WIDGET_H

#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <ktoolbutton.h>
#include <QVBoxLayout>
#include "text_label.h"
#include "defines.h"

namespace fault_diagnosis {

class DiagnosisEntryWidget: public QWidget {
    Q_OBJECT
public:
    enum class Status {
        kNoState,
        kNormal,
        kException,
        kDiagnosising,
        kFixed
    };

    DiagnosisEntryWidget(const DiagnosticEntry &diagnostic_entry, QWidget *parent = nullptr);
    ~DiagnosisEntryWidget();
    void SetStatus(Status status);
    void SetDiagnosticResult(const CheckEntryResult &check_entry_result);
    void SetCheckedState(Qt::CheckState state);
    void SetCheckedEnable(bool enable);
    QString DiagnosisMark() const;
    qint64 Mark() const;
    bool ExistError() const;
    void Reset();
    bool Selected();
    void SetExpanded(bool is_expand);

Q_SIGNALS:
    void sig_Expanded(bool is_expanded);
    void sig_SelectChange();

private Q_SLOTS:
    void on_UpdateExpandOrCloseIcon();

private:
    DiagnosticEntry diagnostic_entry_;
    QLabel *select_seat_label_;
    QVBoxLayout *select_seat_vbox_layout_;
    QCheckBox *select_checkbox_;
    K::TextLabel *name_label_;
    K::TextLabel *diagnosis_result_label_;
    K::TextLabel *status_label_;
    QLabel *expand_or_close_seat_label_;
    QVBoxLayout *expand_or_close_seat_vbox_layout_;
    kdk::KToolButton *expand_or_close_btn_;
    bool is_expand_;    // 记录错误细节是否展开
    CheckEntryResult check_entry_result_;
};

}

#endif
