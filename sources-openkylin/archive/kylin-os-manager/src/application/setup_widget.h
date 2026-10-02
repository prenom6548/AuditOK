#ifndef KYLIN_OS_MANAGER_SRC_APPLICATION_SETUP_WIDGET_H
#define KYLIN_OS_MANAGER_SRC_APPLICATION_SETUP_WIDGET_H

#include <kdialog.h>
#include <QCheckBox>

class SetupWidget: public kdk::KDialog {
    Q_OBJECT
public:
    SetupWidget(QWidget *parent = nullptr);
    ~SetupWidget();

private Q_SLOTS:
    void on_Ok();

private:
    QCheckBox *allow_popup_check_box_;
};

#endif

