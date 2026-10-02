#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "QGSettings/qgsettings.h"
#include <QHBoxLayout>

#include "selectionwidget.h"
#include "loadingwidget.h"
#include "getinfothread.h"
#include "hotplug/hotplugclient.h"
#include "hotplug/devicemonitorthread.h"

class MainWidget : public QWidget {
Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

    QGSettings *m_themeData = nullptr;

private:
    HotplugClient *m_hotplugClient = nullptr;

    QStackedWidget *m_stackWidget = nullptr;
    SelectionWidget *m_selectionWidget = nullptr;
    LoadingWidget *m_loadingWidget = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    GetInfoThread *m_getinfoThread = nullptr;
    DeviceMonitorThread *m_monitorThread = nullptr;

    void initGsetting();

    void initConnection();

    void initGui();

private slots:

    void initMonitorThread();
};


#endif // MAINWIDGET_H
