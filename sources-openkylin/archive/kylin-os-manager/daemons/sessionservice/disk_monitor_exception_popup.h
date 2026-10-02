#ifndef KYLIN_OS_MANAGER_DAEMONS_SESSIONSERVICE_DISK_MONITOR_EXCEPTION_POPUP_H
#define KYLIN_OS_MANAGER_DAEMONS_SESSIONSERVICE_DISK_MONITOR_EXCEPTION_POPUP_H

#include <kwidget.h>
#include <QCheckBox>
#include "text_label.h"

class DiskMonitorExceptionPopup: public kdk::KWidget {
    Q_OBJECT
public:
    DiskMonitorExceptionPopup(const QString &config_file_name);
    ~DiskMonitorExceptionPopup();
    void SetTitle(const QString &title) const;
    void Show();

private Q_SLOTS:
    void on_NotPromptedWithinThreeDays(int state);

private:
    void GenerateConfigFile(bool enabel_start_display_time) const;

    QString       config_file_name_;
    K::TextLabel *title_label_;
    QCheckBox    *not_prompted_within_3_days_check_box_;
};

#endif
