#ifndef HOMEPAGEWIDGET_H
#define HOMEPAGEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGSettings>
#include "../component/icon_label.h"
#include "../component/text_label.h"
#include "kborderlessbutton.h"
#include "kwidget.h"
using namespace kdk;
using namespace K;

class HomePageWidget : public QWidget
{
    Q_OBJECT
public:
    static HomePageWidget *getInstance(void);
    HomePageWidget(QWidget *parent = nullptr);
    ~HomePageWidget();

private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();
    void setBackgroundColor(QWidget *w);
    void adjustBackground();

    TextLabel *m_labelWelcome = nullptr;
    TextLabel *m_labelInfo = nullptr;

    IconLabel *m_labelAppIcon = nullptr;
    TextLabel *m_labelAppType = nullptr;

    IconLabel *m_labelHomeIcon = nullptr;
    TextLabel *m_labelHomeType = nullptr;

    IconLabel *m_labelSysIcon = nullptr;
    TextLabel *m_labelSysType = nullptr;

    QLabel *m_labelIconBig = nullptr;
    QPushButton *m_btnStart = nullptr;
    KBorderlessButton *m_btnHowUse = nullptr;

    QHBoxLayout *m_layoutHMain = nullptr;         // 文字与图片的横向布局
    QHBoxLayout *m_layoutHBtnStart = nullptr;     // 开始按钮横向布局
    QHBoxLayout *m_layoutHBtnHowToUse = nullptr;  // 提示按钮横向布局
    QFormLayout *m_layoutF = nullptr;
    QHBoxLayout *m_layoutHApp = nullptr;  // 应用迁移横向布局
    QHBoxLayout *m_layoutHHome = nullptr;  // 用户数据迁移横布局
    QHBoxLayout *m_layoutHSys = nullptr;  // 系统迁移横布局

    QVBoxLayout *m_layoutVMain = nullptr; // 主界面纵向布局
    QVBoxLayout *m_layoutVInfo = nullptr; // 文字信息纵向布局

    QGSettings* m_pTransparency;
    double tranSparency;

signals:
    void sinStatClick();
public slots:
};
#endif
