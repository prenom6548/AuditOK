#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_ENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_ENTRY_WIDGET_H

#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <QVBoxLayout>
#include <ktoolbutton.h>
#include <kborderlessbutton.h>
#include "text_label.h"
#include "prompt.h"
#include "defines.h"

namespace fault_diagnosis {

class DiagnosisEntryWidget: public QWidget {
    Q_OBJECT
public:
    enum class Status {
        kNoState,
        kDiagnosising,
        kNormal,
        kException,
        kRepairing,
        kFixed,
        KReapriFailed,
    };

    DiagnosisEntryWidget(const DiagnosticEntry &diagnostic_entry, QWidget *parent = nullptr);
    ~DiagnosisEntryWidget();
    void SetStatus(enum Status status);
    void SetCheckedState(Qt::CheckState state);
    void SetCheckedEnable(bool enable);
    void SetDiagnosisResult(bool ok, const QString &error_code = "", const QString &describe = "");
    void SetRepairResult(bool ok);
    void SetExpanded(bool is_expand);
    bool ExistError() const;
    bool Selected();
    QString DiagnosisMark() const;
    QString Mark() const;
    QString Name() const;
    QString ErrorCode() const;
    enum DiagnosisEntryWidget::Status Status() const;
    void Disable();

Q_SIGNALS:
    void sig_Expanded(bool is_expanded);
    void sig_SelectChange();

private Q_SLOTS:
    void on_UpdateExpandOrCloseIcon();

private:
    DiagnosticEntry         diagnostic_entry_;
    enum Status             status_;
    QLabel *                select_seat_label_;
    QVBoxLayout *           select_seat_vbox_layout_;
    QCheckBox *             select_checkbox_;
    K::TextLabel *          name_label_;
    K::TextLabel *          desc_label_;
    K::TextLabel *          status_label_;
    kdk::KBorderlessButton *contact_technical_service_btn_;
    kdk::KBorderlessButton *go_clean_up_btn_;
    Prompt *                prompt_label_;
    QLabel *                expand_or_close_seat_label_;
    QVBoxLayout *           expand_or_close_seat_vbox_layout_;
    kdk::KToolButton *      expand_or_close_btn_;
    bool                    is_expand_;    // 记录是否处于展开模式

    // 记录错误码
    QString                 error_code_;
    QString                 error_desc_;
};

}

#endif
