#ifndef TITLEBAR_H
#define TITLEBAR_H

#define KYLIN_USER_GUIDE_PATH "/"
#define KYLIN_USER_GUIDE_SERVICE "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE "com.guide.hotel"

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QList>
#include <QAction>
#include <QPalette>
#include <unistd.h>
#include "publicattributes.hpp"

class Titlebar : public QWidget
{
    Q_OBJECT
public:
    Titlebar(QWidget *parent = nullptr);
    ~Titlebar();
    void changeMode(PublicAttributes::Model model);
    void setMaxBtnMode(bool isMax);
    void initHelp(); // 设置帮助Action
    void setTheme(PublicAttributes::Theme theme);
    void setBackground(bool isDefault = true);
    void changeFontSize(double fontSize);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

Q_SIGNALS:
    void sigAboutWinShow();
public Q_SLOTS:
    void onClicked(void);
    void triggerMenu(QAction *);

private:
    void initUI();
    void connectInit(void); // 绑定槽函数
    void initMenu(void);    // 设置菜单
    void initAbout();       // 设置关于Action

private:
    QPushButton *m_titleIcon = nullptr;
    QLabel *m_titleNameLab = nullptr;
    QToolButton *m_menuBtn = nullptr;                                 // 菜单按钮
    QMenu *m_menu = nullptr;                                          // 菜单
    QList<QAction *> m_actions;                                       // action组
    QPushButton *m_minBtn = nullptr;                                  // 最小化按钮
    QPushButton *m_maxBtn;                                            // 最大化按钮
    QPushButton *m_closeBtn = nullptr;                                // 关闭按钮
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light; // 主题
    bool m_isDefault = true;
    bool m_isMaximize = false;
};

#endif // TITLEBAR_H
