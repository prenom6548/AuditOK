#ifndef SELECTIONWIDGET_H
#define SELECTIONWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

#include "driverinstallwidget.h"
#include "hardwareinfowidget.h"
#include "driveruninstallmainwidget.h"
#include "include/custompushbutton.h"

class SelectionWidget : public QWidget {
Q_OBJECT
public:
    explicit SelectionWidget(QWidget *parent = nullptr);

private:
    CustomPushButton *m_driverInstallBtn = nullptr;
    CustomPushButton *m_hardwareInfoBtn = nullptr;
    CustomPushButton *m_driverUninstallBtn = nullptr;

    QButtonGroup *m_btnGroup = nullptr;

    QStackedWidget *m_stackWidget = nullptr;
    DriverInstallWidget *m_driverInstallWidget = nullptr;
    DriverUninstallMainWidget *m_driverUninstallWidget = nullptr;
    HardWareInfoWidget *m_hardwareInfoWidget = nullptr;

    QScrollArea *m_driverInstallSrollArea = nullptr;
    QScrollArea *m_driverUninstallSrollArea = nullptr;
    QScrollArea *m_hardwareInfoSrollArea = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_buttonLayout = nullptr;

    void initGui();

    void initConnect();
};


#endif //SELECTIONWIDGET_H
