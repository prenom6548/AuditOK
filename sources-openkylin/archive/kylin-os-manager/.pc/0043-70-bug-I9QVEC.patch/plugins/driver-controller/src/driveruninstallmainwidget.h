#ifndef DRIVERUNINSTALLMAINWIDGET_H
#define DRIVERUNINSTALLMAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>

#include "nodriveruninstallpage.h"
#include "driveruninstallwidget.h"

class DriverUninstallMainWidget : public QWidget {
Q_OBJECT
public:
    DriverUninstallMainWidget(QWidget *parent = nullptr);

private:
    QStackedWidget *m_stackWidget = nullptr;

    NoDriverUninstallPage *m_noDriverUninstallWidget = nullptr;
    DriverUninstallWidget *m_driverUninstallWidget = nullptr;

    QHBoxLayout *m_mainLayout = nullptr;

    void initGui();

    void initConnection();


private slots:

    void switchToUninstallWidget();

    void switchToNoDriverUnistallWidget();

};


#endif //DRIVERUNINSTALLMAINWIDGET_H
