#ifndef UNINSTALLITEM_H
#define UNINSTALLITEM_H

#include <QWidget>
#include <kpressbutton.h>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include "include/text_label.h"
#include "hardwareinfo/hardwareinfo.h"

class UninstallItem : public QWidget {
Q_OBJECT
public:
    UninstallItem(HardWareInfo hardwareinfo, QWidget *parent = nullptr);

    QString getDevName();

    DeviceType getDevType();

private:
    HardWareInfo m_hardwareinfo;

    QCheckBox *m_check = nullptr;
    kdk::KPressButton *m_Icon = nullptr;
    QLabel *m_deviceType = nullptr;
    K::TextLabel *m_deviceName = nullptr;
    QLabel *m_driverVersion = nullptr;
    QPushButton *m_uninstallButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QStackedWidget *m_btnStackWidget = nullptr;
    QHBoxLayout *m_uninstallingLayout = nullptr;
    QWidget *m_uninstallingWidget = nullptr;
    QVBoxLayout *m_typeNameLayout = nullptr;
    QWidget *m_typeNameWidget = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;

    void initGui();

    void initConnect();

    DriverInfo getGpuDriver();

    void resizeEvent(QResizeEvent *event);

private slots:

    void themeChangeSlot();

    void uninstallBtnClicked();

public slots:
    void changeProgress(int percent);

};


#endif //UNINSTALLITEM_H
