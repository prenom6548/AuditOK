#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_SUBENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_SUBENTRY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <kborderlessbutton.h>
#include <QPushButton>
#include "text_label.h"
#include "prompt.h"
#include "defines.h"

namespace fault_diagnosis {

class DiagnosisSubentryWidget: public QWidget {
    Q_OBJECT
public:
    enum class Status {
        kNoState,
        kDiagnosising,
        kNormal,
        kException,
        kRepairing,
        kFixed,
        kRepairFailed,
    };

    DiagnosisSubentryWidget(const DiagnosticItem &diagnostic_item, QWidget *parent = nullptr);
    ~DiagnosisSubentryWidget();
    void SetStatus(Status status);
    void SetCheckedState(Qt::CheckState state);
    void SetCheckedEnable(bool enable);
    void SetDiagnosisResult(bool ok, const QString &error_code = "", const QString &describe = "");
    void SetRepairResult(bool ok);
    QString Mark() const;
    bool AllowSelect() const;
    bool Selected() const;
    bool ExistError() const;
    enum DiagnosisSubentryWidget::Status Status() const;
    QString ErrorCode() const;
    QString Name() const;

Q_SIGNALS:
    void sig_SelectChange();

private:
    DiagnosticItem          diagnostic_item_;
    QLabel *                select_seat_label_;
    QVBoxLayout *           select_seat_vbox_layout_;
    QCheckBox *             select_checkbox_;
    K::TextLabel *          name_label_;
    K::TextLabel *          desc_label_;
    K::TextLabel *          status_label_;
    kdk::KBorderlessButton *contact_technical_service_btn_;
    kdk::KBorderlessButton *go_clean_up_btn_;
    Prompt *                prompt_label_;
    enum Status             status_;

    // 记录错误信息
    QString                 error_code_;
    QString                 error_desc_;
};

}

#endif
