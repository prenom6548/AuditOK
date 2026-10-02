#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_HOMEPAGE_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_HOMEPAGE_H

#include <QWidget>
#include <QPushButton>
#include "text_label.h"
#include "icon_label.h"

namespace junk_clean {

class Homepage: public QWidget {
    Q_OBJECT
public:
    Homepage(QWidget *parent = nullptr);

Q_SIGNALS:
    void sig_StartScan();

private:
    K::TextLabel *title_label_;
    K::TextLabel *describe_label_;
    K::IconLabel *system_junk_icon_label_;
    K::TextLabel *system_junk_name_label_;
    K::TextLabel *system_junk_describe_label_;
    K::IconLabel *internet_junk_icon_label_;
    K::TextLabel *internet_junk_name_label_;
    K::TextLabel *internet_junk_describe_label_;
    K::IconLabel *usage_trace_icon_label_;
    K::TextLabel *usage_trace_name_label_;
    K::TextLabel *usage_trace_describe_label_;
    QPushButton  *start_scan_btn_;
    K::IconLabel *homepage_icon_label_;
};

}

#endif
