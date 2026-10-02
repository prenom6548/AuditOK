#include "videotitle.h"

#include <QHBoxLayout>
#include <KF5/KWindowSystem/KWindowSystem>
#include <QMouseEvent>
#include <gsettingmonitor.h>

#define UKUI_STYLE_GSETTING_PATH "org.ukui.style"

VideoTitle::VideoTitle(QWidget *parent) : QWidget(parent)
{
    initUI();
    // 设置菜单
    initMenu();
    // 信号连接槽
    connectInit();

    double fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    QFont font;
    font.setPointSizeF(fontSize);
    m_titleNameLab->setFont(font);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        double fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
        QFont font;
        font.setPointSizeF(fontSize);
        m_titleNameLab->setFont(font);
    });
    if (QGSettings::isSchemaInstalled(UKUI_STYLE_GSETTING_PATH)) {
        m_fontData = new QGSettings(UKUI_STYLE_GSETTING_PATH);
        connect(m_fontData, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "systemFont") {
                double fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
                QFont font;
                font.setPointSizeF(fontSize);
                m_titleNameLab->setFont(font);
            }
        });
    }
}

VideoTitle::~VideoTitle()
{
    if (m_menu != nullptr) {
        delete m_menu;
        m_menu = nullptr;
    }
}

void VideoTitle::initUI()
{
    m_titleIconLab = new QPushButton(this);
    m_titleNameLab = new QLabel(this);
    m_titleIconLab->setIconSize(QSize(24, 24));
    m_titleIconLab->setIcon(QIcon::fromTheme("kylin-connectivity"));
    QString m_titleIconStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_titleIconLab->setStyleSheet(m_titleIconStyle);
    m_titleIconLab->setContentsMargins(8, 8, 8, 8);
    m_menuBtn = new QToolButton(this);
    m_menu = new QMenu();
    m_minBtn = new QPushButton(this);
    m_maxBtn = new QPushButton(this);
    m_closeBtn = new QPushButton(this);

    m_titleNameLab->setText(tr("kylin-connectivity"));
    // 固定高度
    setFixedHeight(40);
    setAutoFillBackground(true);
    // 菜单按钮
    m_menuBtn->setFixedSize(30, 30);
    m_menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuBtn->setIconSize(QSize(16, 16));
    m_menuBtn->setProperty("isWindowButton", 0x1);
    m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuBtn->setPopupMode(QToolButton::InstantPopup);
    m_menuBtn->setAutoRaise(true);
    m_menuBtn->setContentsMargins(0, 0, 0, 0);
    m_menuBtn->setFocusPolicy(Qt::NoFocus);
    // 最小化按钮
    m_minBtn->setFixedSize(30, 30);
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_minBtn->setIconSize(QSize(16, 16));
    m_minBtn->setProperty("isWindowButton", 0x1);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setFlat(true);
    m_minBtn->setContentsMargins(0, 0, 0, 0);
    m_minBtn->setFocusPolicy(Qt::NoFocus);
    // 最大化按钮
    m_maxBtn->setFixedSize(30, 30);
    m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_maxBtn->setIconSize(QSize(16, 16));
    m_maxBtn->setProperty("isWindowButton", 0x1);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setFlat(true);
    m_maxBtn->setContentsMargins(0, 0, 0, 0);
    m_maxBtn->setFocusPolicy(Qt::NoFocus);
    // 关闭按钮
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setIconSize(QSize(16, 16));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    m_closeBtn->setContentsMargins(0, 0, 0, 0);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);

    // 整体布局
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_titleIconLab);
    layout->addWidget(m_titleNameLab);
    layout->addStretch();
    layout->addWidget(m_menuBtn);
    layout->addWidget(m_minBtn);
    layout->addWidget(m_maxBtn);
    layout->addWidget(m_closeBtn);

    setLayout(layout);
}

// 连接槽
void VideoTitle::connectInit()
{
    // 连接最大、小化、关闭按钮信号槽
    connect(m_minBtn, &QPushButton::clicked, this, &VideoTitle::onMinClick);
    connect(m_maxBtn, &QPushButton::clicked, this, &VideoTitle::onMaxClick);
    connect(m_closeBtn, &QPushButton::clicked, this, &VideoTitle::onCloseClick);
    // 连接菜单信号槽
    connect(m_triple, &QAction::triggered, this, &VideoTitle::onTripleTrigger);
    connect(m_stayOnTop, &QAction::triggered, this, &VideoTitle::onStayOnTopTrigger);
    connect(m_fullScreen, &QAction::triggered, this, &VideoTitle::onFullScreenTrigger);
    connect(m_scrollWidget, &QAction::triggered, this, &VideoTitle::onScrollTrigger);
    connect(m_actionQuit, &QAction::triggered, this, &VideoTitle::onQuitTrigger);
}

// 设置菜单按钮
void VideoTitle::initMenu()
{
    // 创建Action
    m_triple = new QAction(m_menu);        // 三联按钮
    m_stayOnTop = new QAction(m_menu);     // 置顶
    m_fullScreen = new QAction(m_menu);    // 全屏
    m_scrollWidget = new QAction(m_menu);  // 全屏
    m_actionQuit = new QAction(m_menu);    // 退出
    m_triple->setText(tr("Hide Navigation Button"));
    m_stayOnTop->setText(tr("Stay on top"));
    m_fullScreen->setText(tr("FullScreen"));
    m_scrollWidget->setText(tr("Mouse sensitivity"));
    m_actionQuit->setText(tr("Quit"));

    m_actions << m_triple << m_stayOnTop << m_fullScreen << m_scrollWidget << m_actionQuit;
    m_menu->addActions(m_actions);
    // 为菜单按钮设置菜单
    m_menuBtn->setMenu(m_menu);
}

void VideoTitle::onMinClick()
{
    window()->showMinimized();
}

void VideoTitle::onMaxClick()
{
    // 当前窗口是否最大化，最大化：点击后还原，更换图标。窗口化：点击后最大化，更换图标
    if (window()->isMaximized()) {
        window()->showNormal();
    } else {
        window()->showMaximized();
    }
}

void VideoTitle::onCloseClick()
{
    window()->close();
}

void VideoTitle::onTripleTrigger()
{
    if (isTripleShow) {
        m_triple->setText(tr("Show Navigation Button"));
        isTripleShow = false;
    } else {
        m_triple->setText(tr("Hide Navigation Button"));
        isTripleShow = true;
    }
    Q_EMIT sigChangeNavigationStatus();
}

void VideoTitle::onStayOnTopTrigger()
{
    if (isStayOnTop) {
        KWindowSystem::clearState(window()->winId(), KWindowSystem::KeepAbove);
        m_stayOnTop->setText(tr("Stay On Top"));
        isStayOnTop = false;
    } else {
        KWindowSystem::setState(window()->winId(), KWindowSystem::KeepAbove);
        m_stayOnTop->setText(tr("Cancel Stay On Top"));
        isStayOnTop = true;
    }
}

void VideoTitle::onFullScreenTrigger()
{
    if (window()->isFullScreen()) {
        window()->showNormal();
        if (m_oldState == Qt::WindowState::WindowMaximized) {
            window()->showMaximized();
        }
        m_fullScreen->setText(tr("FullScreen"));
    } else {
        m_oldState = window()->windowState();
        window()->showFullScreen();
        m_fullScreen->setText(tr("Cancel FullScreen"));
    }
}

void VideoTitle::onScrollTrigger()
{
    Q_EMIT sigScroll();
}

void VideoTitle::onQuitTrigger()
{
    window()->close();
}

void VideoTitle::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (window()->isMaximized()) {
        window()->showNormal();
    } else {
        window()->showMaximized();
    }
}

void VideoTitle::setMaxBtnMode(bool isMax)
{
    if (!isMax) {
        m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    } else {
        m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    }
}

void VideoTitle::fullScreen()
{
    onFullScreenTrigger();
}

void VideoTitle::changeMode(bool isTable)
{
    m_isTable = isTable;
    if (m_isTable) {
        m_maxBtn->hide();
        // 将Action添加到菜单中
        m_menu->removeAction(m_stayOnTop);
        m_menu->removeAction(m_fullScreen);
    } else {
        m_maxBtn->show();
        // 将Action添加到菜单中
        m_actions.clear();
        m_actions << m_triple << m_stayOnTop << m_fullScreen << m_scrollWidget << m_actionQuit;
        m_menu->addActions(m_actions);
    }
    // 为菜单按钮设置菜单
    m_menuBtn->setMenu(m_menu);
}
