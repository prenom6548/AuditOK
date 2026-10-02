#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_HOME_PAGE_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_HOME_PAGE_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <kborderlessbutton.h>
#include "text_label.h"
#include "icon_label.h"

namespace fault_diagnosis {

class HomePageWidget: public QWidget {
    Q_OBJECT
public:
    HomePageWidget();
    ~HomePageWidget();

Q_SIGNALS:
    void sig_OneClickDetection();

private Q_SLOTS:
    void on_InternalNetworkDetection();

private:
    K::TextLabel *describe_label_;
    K::TextLabel *details_label_;
    K::IconLabel *hardware_detection_icon_label_;
    K::TextLabel *hardware_detection_label_;
    K::IconLabel *kernel_detection_icon_label_;
    K::TextLabel *kernel_detection_label_;
    K::IconLabel *system_service_detection_icon_label_;
    K::TextLabel *system_service_detection_label_;
    K::IconLabel *system_application_detection_icon_label_;
    K::TextLabel *system_application_detection_label_;
    K::IconLabel *icon_label_;
    QPushButton *one_click_detection_btn_;
    kdk::KBorderlessButton *internal_network_detection_btn_;
};

}

#endif
