#ifndef HARDWAREINFOMAINWIDGET_H
#define HARDWAREINFOMAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QScrollArea>

#include "hardwareinfowidget.h"
#include "hardwareinfogetfailwidget.h"

class HardwareInfoMainWidget : public QWidget {
Q_OBJECT
public:
    HardwareInfoMainWidget(QWidget *parent = nullptr);

private:
    QHBoxLayout *m_mainLayout = nullptr;
    QStackedWidget *m_mainWidget = nullptr;
    HardWareInfoWidget *m_hardwareinfoWidget = nullptr;
    HardwareInfoGetFailWidget *m_hardwareinfoGetFailWidget = nullptr;
    QScrollArea *m_hardwareInfoSrollArea = nullptr;

    void initGui();

    void initConnect();

};

#endif //HARDWAREINFOMAINWIDGET_H
