#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_FINISH_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_FINISH_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <kborderlessbutton.h>
#include "icon_label.h"
#include "text_label.h"

namespace junk_clean {

class CleanUpFinishWidget: public QWidget {
    Q_OBJECT
public:
    CleanUpFinishWidget(QWidget *parent = nullptr);
    void SetCleanedSize(quint64 byte);
    void SetCleanedFileCount(quint64 count);
    void SetCleanTime(quint64 second);
    void checkDiskAgain();

Q_SIGNALS:
    void sig_Complete();

private:
    void needDeepClean(QString size);
    void startDeepClean();
    K::IconLabel *          clean_success_icon_label_;
    K::TextLabel *          clean_success_describe_label_;
    K::IconLabel *          clean_file_icon_label_;
    K::TextLabel *          clean_file_describe_label_;
    K::IconLabel *          clean_time_icon_label_;
    K::TextLabel *          clean_time_describe_label_;
    K::TextLabel *          clean_need_deep_label_;
    QPushButton  *          complete_btn_;
    kdk::KBorderlessButton *deep_clean_btn_;
};

}

#endif
