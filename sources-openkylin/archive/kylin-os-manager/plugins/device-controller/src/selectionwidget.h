#ifndef SELECTIONWIDGET_H
#define SELECTIONWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <ktabbar.h>

#include "driverinstallmainwidget.h"
#include "hardwareinfomainwidget.h"
#include "include/custompushbutton.h"

class SelectionWidget : public QWidget {
Q_OBJECT
public:
    explicit SelectionWidget(QWidget *parent = nullptr);

private:
    kdk::KTabBar *m_tabBtnBar = nullptr;
    QHBoxLayout *m_tabBtnLayout = nullptr;
    QStackedWidget *m_stackWidget = nullptr;
    DriverInstallMainWidget *m_driverInstallWidget = nullptr;
    HardwareInfoMainWidget *m_hardwareInfoMainWidget = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;

    void initGui();

    void initConnect();
};


#endif //SELECTIONWIDGET_H
