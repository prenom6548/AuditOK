#include "titlebar.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>
#include <usermanual.h>

const int DEFAULT_HEIGHT = 48;
const int TABLET_HEIGHT = 64;
const int ICON_SIZE = 16;
const int BTN_SIZE = 36;
const int TABLET_BTN_SIZE = 48;

Titlebar::Titlebar(QWidget *parent) : QWidget(parent)
{
    initMenu();
    initUI();
    connectInit();
}

Titlebar::~Titlebar()
{
}

void Titlebar::initUI()
{
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("titlebar");
    setFixedHeight(DEFAULT_HEIGHT);
    m_titleNameLab = new QLabel(this);
    m_titleIcon = new QPushButton(this);
    m_titleIcon->setIconSize(QSize(24, 24));
    QString m_titleIconStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_titleIcon->setStyleSheet(m_titleIconStyle);
    m_titleIcon->setIcon(QIcon::fromTheme("kylin-connectivity"));
    m_titleIcon->setContentsMargins(8, 8, 8, 8);

    m_minBtn = new QPushButton(this);
    m_maxBtn = new QPushButton(this);
    m_closeBtn = new QPushButton(this);

    m_titleNameLab->setText(tr("kylin-connectivity"));
    // 固定高度
    setAutoFillBackground(true);
    // 最小化按钮
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_minBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_minBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_minBtn->setProperty("isWindowButton", 0x1);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setFlat(true);
    m_minBtn->setContentsMargins(4, 0, 4, 0);
    m_minBtn->setFocusPolicy(Qt::NoFocus);
    m_minBtn->setToolTip(tr("Minimize"));
    // 最大化按钮
    m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_maxBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_maxBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_maxBtn->setProperty("isWindowButton", 0x1);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setFlat(true);
    m_maxBtn->setContentsMargins(4, 0, 4, 0);
    m_maxBtn->setFocusPolicy(Qt::NoFocus);
    m_maxBtn->setToolTip(tr("Maximize"));
    // 关闭按钮
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_closeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    m_closeBtn->setContentsMargins(4, 0, 4, 0);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setToolTip(tr("Close"));

    // 整体布局
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addSpacing(8);
    layout->addWidget(m_titleIcon);
    layout->addWidget(m_titleNameLab);
    layout->addStretch();
    layout->addWidget(m_menuBtn);
    layout->addWidget(m_minBtn);
    layout->addWidget(m_maxBtn);
    layout->addWidget(m_closeBtn);
    layout->addSpacing(4);
    layout->setContentsMargins(0, 4, 0, 0);

    setLayout(layout);
}

void Titlebar::changeMode(PublicAttributes::Model model)
{
    if (window()->isMaximized()) {
        m_isMaximize = true;
    } else {
        m_isMaximize = false;
    }
    if (model == PublicAttributes::Model::Table) {
        setFixedHeight(TABLET_HEIGHT);
        m_titleIcon->setIconSize(QSize(32, 32));
        m_isMaximize = true;
        m_maxBtn->hide();
        m_menuBtn->setFixedSize(TABLET_BTN_SIZE, TABLET_BTN_SIZE);
        m_minBtn->setFixedSize(TABLET_BTN_SIZE, TABLET_BTN_SIZE);
        m_closeBtn->setFixedSize(TABLET_BTN_SIZE, TABLET_BTN_SIZE);
    } else {
        setFixedHeight(DEFAULT_HEIGHT);
        m_titleIcon->setIconSize(QSize(24, 24));
        m_maxBtn->show();
        m_menuBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
        m_minBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
        m_closeBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    }
}

void Titlebar::setMaxBtnMode(bool isMax)
{
    if (!isMax) {
        m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maxBtn->setToolTip(tr("Maximize"));
        m_isMaximize = false;
    } else {
        m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maxBtn->setToolTip(tr("Reduction"));
        m_isMaximize = true;
    }
}

void Titlebar::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    if (m_isDefault) {
        switch (m_theme) {
            case PublicAttributes::Theme::Light: {
                setStyleSheet("QWidget#titlebar{background-color: #FFFFFF;}");
            } break;
            case PublicAttributes::Theme::Dark: {
                setStyleSheet("QWidget#titlebar{background-color: #1D1D1D;}");
            } break;
        }
    }
}

void Titlebar::setBackground(bool isDefault)
{
    m_isDefault = isDefault;
    if (isDefault) {
        switch (m_theme) {
            case PublicAttributes::Theme::Light: {
                setStyleSheet("QWidget#titlebar{background-color: #FFFFFF;}");
            } break;
            case PublicAttributes::Theme::Dark: {
                setStyleSheet("QWidget#titlebar{background-color: #1D1D1D;}");
            } break;
        }
    } else {
        setAttribute(Qt::WA_StyledBackground);
        setStyleSheet("QWidget#titlebar{background-color: rgba(98, 142, 255, 0.1);}");
    }
}

void Titlebar::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize);

    m_titleNameLab->setFont(font);
}

void Titlebar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton) {  // 判断鼠标右键是否被按下
        return;
    }

    if (window()->isMaximized()) {
        window()->showNormal();
    } else {
        window()->showMaximized();
    }
}

// 连接槽
void Titlebar::connectInit()
{
    // 连接最大、小化、关闭按钮信号槽
    connect(m_minBtn, &QPushButton::clicked, this, &Titlebar::onClicked);
    connect(m_maxBtn, &QPushButton::clicked, this, &Titlebar::onClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &Titlebar::onClicked);
    // 连接菜单信号槽
    connect(m_menu, &QMenu::triggered, this, &Titlebar::triggerMenu);
}

// 设置菜单按钮
void Titlebar::initMenu()
{
    m_menuBtn = new QToolButton(this);
    m_menuBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_menuBtn->setToolTip(tr("Menu"));
    // 菜单按钮
    m_menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_menuBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_menuBtn->setProperty("isWindowButton", 0x1);
    m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuBtn->setPopupMode(QToolButton::InstantPopup);
    m_menuBtn->setFocusPolicy(Qt::NoFocus);
    m_menuBtn->setAutoRaise(true);
    m_menuBtn->setContentsMargins(4, 0, 4, 0);

    m_menu = new QMenu(this);
    // 创建Action
    QAction *actionHelp = new QAction(m_menu);
    QAction *actionAbout = new QAction(m_menu);
    QAction *actionQuit = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    actionAbout->setText(tr("About"));
    actionQuit->setText(tr("Quit"));

    // 将Action添加到菜单中
    m_actions << actionHelp << actionAbout << actionQuit;
    m_menu->addActions(m_actions);

    // 为菜单按钮设置菜单
    m_menuBtn->setMenu(m_menu);
}

// 处理点击信号
void Titlebar::onClicked()
{
    QPushButton *clickBtn = qobject_cast<QPushButton *>(sender());
    if (clickBtn == m_minBtn) {
        // 最小化
        if (m_isMaximize) {
            window()->showMaximized();
        } else {
            window()->showNormal();
        }
        window()->showMinimized();
    } else if (clickBtn == m_closeBtn) {
        window()->close();
    } else if (clickBtn == m_maxBtn) {
        // 当前窗口是否最大化，最大化：点击后还原，更换图标。窗口化：点击后最大化，更换图标
        if (window()->isMaximized()) {
            window()->showNormal();
        } else {
            window()->showMaximized();
        }
    }
}

// Action处理
void Titlebar::triggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("Help") == str) {
        initHelp();
    } else if (tr("About") == str) {
        Q_EMIT sigAboutWinShow();
    } else if (tr("Quit") == str) {
        window()->close();
    }
}

// 帮助入口，调用用户手册
void Titlebar::initHelp()
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusMessage msg = QDBusMessage::createMethodCall(QString(service_name), KYLIN_USER_GUIDE_PATH,
                                                      KYLIN_USER_GUIDE_INTERFACE, "showGuide");
    msg << "kylin-connectivity";
    if (QDBusConnection::sessionBus().isConnected())
        QDBusConnection::sessionBus().call(msg);
}