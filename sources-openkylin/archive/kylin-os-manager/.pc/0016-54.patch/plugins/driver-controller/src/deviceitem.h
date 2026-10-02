#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QWidget>
#include <QCheckBox>
#include <kpressbutton.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QString>

#include "hardwareinfo/hardwareinfo.h"
#include "include/customlabel.h"

class DeviceItem : public QWidget {
Q_OBJECT
public:
    DeviceItem(HardWareInfo hardwareinfo, QWidget *parent = nullptr);

    bool m_isChecked = false;

    void checkTheBox();

    void unCheckTheBox();

    QString getDevName();

    DeviceType getDevType();

    void clickInstall();

private:
    HardWareInfo m_hardwareinfo;

    QCheckBox *m_check = nullptr;
    kdk::KPressButton *m_Icon = nullptr;
    QLabel *m_deviceType = nullptr;
    CustomLabel *m_deviceName = nullptr;
    QLabel *m_driverVersion = nullptr;
    QPushButton *m_updateButton = nullptr;
    QLabel *m_updateText = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QStackedWidget *m_btnStackWidget = nullptr;
    QHBoxLayout *m_installingLayout = nullptr;
    QWidget *m_installingWidget = nullptr;
    QVBoxLayout *m_typeNameLayout = nullptr;
    QWidget *m_typeNameWidget = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;

    void initGui();

    void initConnect();

    DriverInfo getGpuDriver();

    void resizeEvent(QResizeEvent *event);

private slots:

    void themeChangeSlot();

    void setChecked();

    void installBtnClicked();


public slots:

    void changeProgress(int percent);
};

#endif // DEVICEITEM_H
