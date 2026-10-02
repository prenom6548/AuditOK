#ifndef SUSPENDTABBAR_H
#define SUSPENDTABBAR_H

#include <QWidget>
#include <QPushButton>
#include <QPropertyAnimation>
#include <ktoolbutton.h>
#include "publicattributes.hpp"

class OpenBtn;

class SuspendTabBar : public QWidget
{
    Q_OBJECT
public:
    enum Display {
        Whole = 0, // 全部按钮显示
        Part,      // 部分按钮显示
    };
    Q_ENUM(Display)

    SuspendTabBar(SuspendTabBar::Display mode = SuspendTabBar::Display::Whole, bool isTool = false,
                  QWidget *parent = nullptr);
    ~SuspendTabBar();

    enum BtnType {
        ControlBtn = 0,   // 反控按钮
        CloseControlBtn,  // 关闭反控
        ReturnDesktopBtn, // 返回桌面
        ScreenShareBtn,   // 投屏
        ExitBtn,          // 退出
    };
    Q_ENUM(BtnType)

    void setTheme(PublicAttributes::Theme theme);
    void hideTabBar();
    void showTabBar();
    void setDeskBtnState(bool isDesktop);

protected:
    void paintEvent(QPaintEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    kdk::KToolButton *m_controlBtn = nullptr;
    kdk::KToolButton *m_switchDeskBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
    OpenBtn *m_openBtn = nullptr;
    QPropertyAnimation *m_setHeightAnimation = nullptr;

    bool m_ctrlBtnState = true;
    bool m_deskBtnState = false;
    bool m_isOpen = true;
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light; // 主题

private:
    void initWholeUI();
    void initPartUI();
    void initAnimation();

Q_SIGNALS:
    void sigBtnClicked(SuspendTabBar::BtnType);

public Q_SLOTS:
    void onClicked();
    void onOpenClicked();
    void onExitClicked();
};

// 展开按钮
class OpenBtn : public QWidget
{
    Q_OBJECT
public:
    OpenBtn(bool isTool = false, QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    kdk::KToolButton *m_openBtn = nullptr;

    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light; // 主题

Q_SIGNALS:
    void sigClicked();
};

#endif // SUSPENDTABBAR_H
