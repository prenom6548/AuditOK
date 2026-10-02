#ifndef VIDEOTITLE_H
#define VIDEOTITLE_H

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QList>
#include <QAction>
#include <QMouseEvent>
#include <tablemodelistener.h>
#include <QGSettings>

class VideoTitle : public QWidget
{
    Q_OBJECT
public:
    VideoTitle(QWidget *parent = nullptr);
    ~VideoTitle();

    void changeMode(bool isTable);
    void setMaxBtnMode(bool isMax);
    void fullScreen();

private:
    void initUI();
    // 绑定槽函数
    void connectInit(void);

    // 设置菜单
    void initMenu(void);

Q_SIGNALS:
    void sigChangeNavigationStatus();
    void sigScroll();

private Q_SLOTS:
    void onMinClick();
    void onMaxClick();
    void onCloseClick();
    void onTripleTrigger();
    void onStayOnTopTrigger();
    void onScrollTrigger();
    void onQuitTrigger();
    void onFullScreenTrigger();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QPushButton *m_titleIconLab = nullptr;
    QLabel *m_titleNameLab = nullptr;
    QToolButton *m_menuBtn = nullptr;   // 菜单按钮
    QMenu *m_menu = nullptr;            // 菜单
    QList<QAction *> m_actions;         // action组
    QPushButton *m_minBtn = nullptr;    // 最小化按钮
    QPushButton *m_maxBtn = nullptr;    // 最大化按钮
    QPushButton *m_closeBtn = nullptr;  // 关闭按钮
    QAction *m_triple = nullptr;        // 三联按钮
    QAction *m_stayOnTop = nullptr;     // 置顶
    QAction *m_fullScreen = nullptr;    // 置顶
    QAction *m_scrollWidget = nullptr;  // 鼠标灵敏度
    QAction *m_actionQuit = nullptr;    // 退出
    bool isTripleShow = true;
    bool isStayOnTop = false;
    bool m_isTable = false;
    Qt::WindowStates m_oldState = Qt::WindowState::WindowNoState;
    QGSettings *m_fontData = nullptr;  // 字体变化
};

#endif  // VIDEOTITLE_H
