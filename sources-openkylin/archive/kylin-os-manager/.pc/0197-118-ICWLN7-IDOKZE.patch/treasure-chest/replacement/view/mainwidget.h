#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>

#include "../globaldata.h"
#include "homepagewidget.h"
// #include "choosedevicewidget.h"
#include "choosewidget.h"
#include "kwidget.h"

using namespace kdk;

class MainWidget : public kdk::KWidget
{
    Q_OBJECT
public:
    static MainWidget *getInstance(void);

private:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    void adjustBackground();
    
    //堆栈窗体
    QStackedWidget *m_stackedWid = nullptr;
    HomePageWidget *m_homePage = nullptr;
    // ChooseDeviceWidget *m_chooseDevice = nullptr;
    ChooseWidget *m_chooseWidget = nullptr;

    void initWidgetUI();
    void initLayout();
    void initSetStyle();
    void initConn();
    void slotTriggerMenu(QAction *act);
    void initAbout();
    void initHelp();
    void initQuit();

    QGSettings* m_pTransparency;
    double tranSparency;
signals:

public slots:
};
#endif
