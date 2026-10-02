#ifndef CHOOSEWIDGET_H
#define CHOOSEWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "choosedevicewidget.h"
#include "choosesyswidget.h"
#include "senddatawidget.h"
#include "acceptdatawidget.h"

class ChooseWidget : public QWidget
{
    Q_OBJECT
public:
    static ChooseWidget *getInstance(void);
    ChooseWidget(QWidget *parent = nullptr);
    ~ChooseWidget();

private:
    void initUI();

    //堆栈窗体
    QStackedWidget *m_stackedWid = nullptr;
    ChooseDeviceWidget *m_chooseDevice = nullptr;
    ChooseSysWidget *m_chooseSys = nullptr;
    SendDataWidget *m_sendWidget = nullptr;
    AcceptDataWidget *m_acceptWidget = nullptr;
    
    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局

signals:

public slots:
    void slotNextPage(SysType type);
    void slotBackPage();

};
#endif
