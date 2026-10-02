#ifndef CHOOSEDEVICWIDGET_H
#define CHOOSEDEVICWIDGET_H

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
#include "kwidget.h"
using namespace kdk;
using namespace K;

class ChooseDeviceWidget : public QWidget
{
    Q_OBJECT
public:
    static ChooseDeviceWidget *getInstance(void);
    ChooseDeviceWidget(QWidget *parent = nullptr);
    ~ChooseDeviceWidget();

private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();

    TextLabel *m_labelHeadline = nullptr;
    RadioWidget *m_newRadioBtn = nullptr;
    RadioWidget *m_oldRadioBtn = nullptr;
    QPushButton *m_btnNext = nullptr;

    QHBoxLayout *m_layoutHBtn = nullptr;     // 按钮横向布局
    QHBoxLayout *m_layoutHRadioBtn = nullptr;  // 选择按钮横向布局
    QHBoxLayout *m_layoutHHeadline = nullptr;  // 系统迁移横布局

    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局

    QGSettings* m_pTransparency;
    double tranSparency;
    bool m_isClickNew = false;
    bool m_isClickOld = false;

    SysType m_chooseType;

signals:
    void sigNextPage(SysType type);

public slots:
    void slotCilckNewRadio();
    void slotCilckOldRadio();
    void slotCannelNewRadio();
    void slotCannelOldRadio();
    void slotNextBtnClick();

};
#endif
