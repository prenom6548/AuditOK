#ifndef SENDDATAWIDGET_H
#define SENDDATAWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "choosedevicewidget.h"
#include "choosesyswidget.h"
#include "../component/text_label.h"
#include "sendoldsysinfo.h"

class SendDataWidget : public QWidget
{
    Q_OBJECT
public:
    SendDataWidget(QWidget *parent = nullptr);
    ~SendDataWidget();

private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();
    
    K::TextLabel *m_labelInfo = nullptr;
    QPushButton *m_btnBack = nullptr;
    OldSysInfo *m_widget = nullptr;
    
    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    QFormLayout *m_layoutF = nullptr;

signals:
    void sigBackPage();

public slots:
    void slotBackPage();

};
#endif
