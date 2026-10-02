#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_HOME_PAGE_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_HOME_PAGE_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <kborderlessbutton.h>
#include <kpushbutton.h>
#include "text_label.h"
#include "icon_label.h"

namespace fault_diagnosis {

class CategoryDiagnosisBtn: public kdk::KPushButton {
    Q_OBJECT
public:
    CategoryDiagnosisBtn(const QString &icon, const QString &name, const QString desc, QWidget *parent = nullptr);
    ~CategoryDiagnosisBtn();

private:
    QPushButton *icon_btn_;
    K::TextLabel *name_label_;
    K::TextLabel *desc_label_;
};

class HomePageWidget: public QWidget {
    Q_OBJECT
public:
    enum class State {
        kInit,         // 初始状态
        kPreparing,    // 准备中
    };

    HomePageWidget();
    ~HomePageWidget();
    void SetState(enum State state);

Q_SIGNALS:
    void sig_OneClickDetection();
    void sig_NetworkDetection();
    void sig_PerformanceAndComponentsDetection();
    void sig_ApplicationDetection();
    void sig_SystemUpdateDetection();
    void sig_SoftwareStoreDetection();
    void sig_DiskDetection();
    void sig_WiredNetworkDetection();
    void sig_WirelessNetworkDetection();

private Q_SLOTS:
    void on_InternalNetworkDetection();

private:
    K::TextLabel *          describe_label_;
    K::TextLabel *          details_label_;
    K::IconLabel *          icon_label_;
    QPushButton *           one_click_detection_btn_;
    kdk::KBorderlessButton *internal_network_detection_btn_;
};

}

#endif
