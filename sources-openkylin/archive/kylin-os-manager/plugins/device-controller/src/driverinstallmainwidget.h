#ifndef DRIVERINSTALLMAINWIDGET_H
#define DRIVERINSTALLMAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QScrollArea>

#include "driverinstallwidget.h"
#include "drivergetfailwidget.h"

class DriverInstallMainWidget : public QWidget {
    Q_OBJECT
public:
    DriverInstallMainWidget(QWidget *parent = nullptr);

private:
    QHBoxLayout *m_mainLayout = nullptr;
    QStackedWidget *m_mainWidget = nullptr;
    QScrollArea *m_driverInstallSrollArea = nullptr;
    QWidget *m_driverIstallTopWidget = nullptr;
    QHBoxLayout *m_driverInstallLayout = nullptr;
    DriverInstallWidget *m_driverInstallWidget = nullptr;
    DriverGetFailWidget *m_failWidget = nullptr;

    void initGui();

    void initConnect();

};


#endif //DRIVERINSTALLMAINWIDGET_H
