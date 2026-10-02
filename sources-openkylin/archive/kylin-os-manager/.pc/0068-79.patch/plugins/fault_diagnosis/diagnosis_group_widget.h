#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_GROUP_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSIS_GROUP_WIDGET_H

#include <QWidget>
#include <QString>
#include <ktoolbutton.h>
#include "icon_label.h"
#include "text_label.h"

namespace fault_diagnosis {

class DiagnosisGroupWidget: public QWidget {
    Q_OBJECT
public:
    enum class Status {
        kNoState,
        kNormal,
        kException,
        kFix
    };

    DiagnosisGroupWidget(QWidget *parent = nullptr);
    ~DiagnosisGroupWidget();
    void SetIcon(const QString &name);
    void SetCategory(const QString &category_name);
    void SetTotal(int total);
    void SetStatus(Status status);
    QString Category() const;
    void SetExpanded(bool is_expand);
    void Reset();

Q_SIGNALS:
    void sig_Expanded(bool is_expand);

private Q_SLOTS:
    void on_UpdateExpandOrCloseIcon();

private:
    K::IconLabel *icon_label_;
    K::TextLabel *category_label_;
    K::TextLabel *total_label_;
    K::TextLabel *status_label_;
    kdk::KToolButton *expand_or_close_btn_;
    bool is_expand_;    // 记录检测项是否展开
};

}

#endif
