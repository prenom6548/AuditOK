#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_ERROR_ENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_ERROR_ENTRY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <kborderlessbutton.h>
#include "text_label.h"
#include "defines.h"

namespace fault_diagnosis {

class ErrorEntryWidget: public QWidget {
    Q_OBJECT
public:
    enum class Status {
        kException,
        kRepairing,
        kFixed
    };

    ErrorEntryWidget(const DiagnosticError &diagnostic_error, const Database::SolutionRecord &solution_record, QWidget *parent = nullptr);
    ~ErrorEntryWidget();
    qint64 ErrorCode() const;
    bool Selected();
    bool ManualRepair();
    void SwitchToManualRepair(qint64 solution_type, const QString &solution_context);
    bool SetCheckState(Qt::CheckState state);
    void SetCheckEnable(bool enable);
    const Database::SolutionRecord &SolutionRecord() const;
    void SetStatus(Status status);
    ErrorEntryWidget::Status GetStatus() const;
    void SetDescribe(const QString &describe);
    void ShowError() const;
    QString Name() const;

Q_SIGNALS:
    void sig_SelectChange();

public Q_SLOTS:
    void on_ManualRepair() const;

private:
    QString CreateTextHtml() const;
    QString CreateImageHtml() const;

    DiagnosticError          diagnostic_error_;
    Database::SolutionRecord solution_record_;
    int                      solution_category_;
    QCheckBox *              select_checkbox_;
    K::TextLabel *           name_label_;
    K::TextLabel *           describe_label_;
    kdk::KBorderlessButton * manual_repair_btn_;
    K::TextLabel *           status_label_;
    ErrorEntryWidget::Status status_;
};

}

#endif
