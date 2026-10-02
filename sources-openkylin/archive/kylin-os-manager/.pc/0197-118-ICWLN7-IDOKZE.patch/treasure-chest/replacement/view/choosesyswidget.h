#ifndef CHOOSESYSWIDGET_H
#define CHOOSESYSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QGSettings>
#include "../component/icon_label.h"
#include "../component/text_label.h"
#include "../component/radiobutton.h"
#include "../component/radio_widget.h"
#include "kborderlessbutton.h"
using namespace K;

class ChooseSysWidget : public QWidget
{
    Q_OBJECT
public:
    static ChooseSysWidget *getInstance(void);
    ChooseSysWidget(QWidget *parent = nullptr);
    ~ChooseSysWidget();

private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();

    TextLabel *m_labelHeadline = nullptr;
    RadioWidget *m_winRadioBtn = nullptr;
    RadioWidget *m_linuxRadioBtn = nullptr;
    QPushButton *m_btnNext = nullptr;
    QPushButton *m_btnBack = nullptr;

    QHBoxLayout *m_layoutHBtn = nullptr;     // 按钮横向布局
    QHBoxLayout *m_layoutHHeadline = nullptr;  // 系统迁移横布局

    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局

    bool m_isClickLinux = false;
    bool m_isClickWin = false;

    SysType m_chooseType;

signals:
    void sigBackPage();
    void sigNextPage(SysType type);

public slots:
    void slotCilckWinRadio();
    void slotCilckLinuxRadio();
    void slotCannelWinRadio();
    void slotCannelLinuxRadio();
    void slotBackPage();
    void slotNextPadge();

};
#endif
