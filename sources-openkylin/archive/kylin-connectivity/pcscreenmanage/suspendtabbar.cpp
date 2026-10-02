#include "suspendtabbar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QApplication>
#include <QScreen>

const int ICON_W = 24;
const int ICON_H = 24;
const int WIN_W = 56;
const int WIN_H = 32;
const int WINTWO_W = 120;
const int WINTWO_H = 56;
const int WINTHREE_W = 168;
const int WINTHREE_H = 56;
const double TRANSPARENCY = 0.75;

SuspendTabBar::SuspendTabBar(SuspendTabBar::Display mode, bool isTool, QWidget *parent) : QWidget(parent)
{
    setProperty("useStyleWindowManager", false);
    setProperty("useSystemStyleBlur", true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    if (isTool) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    }
    m_openBtn = new OpenBtn(isTool, parent);
    connect(m_openBtn, &OpenBtn::sigClicked, this, &SuspendTabBar::onOpenClicked);
    m_openBtn->setFixedHeight(1);
    if (mode == SuspendTabBar::Display::Whole) {
        initWholeUI();
    } else {
        initPartUI();
    }
    initAnimation();
    activateWindow();
    setFocus();

    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    move(availableGeometry.width() / 2 - width() / 2, 0);
}

SuspendTabBar::~SuspendTabBar()
{
    if (m_openBtn != nullptr) {
        delete m_openBtn;
        m_openBtn = nullptr;
    }
}

void SuspendTabBar::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    m_openBtn->setTheme(m_theme);
    switch (theme) {
        case PublicAttributes::Theme::Light: {
            m_exitBtn->setStyleSheet(
                "QPushButton "
                "{border:0px;border-radius:4px;background:transparent;background-image:url(:/tabbaricon/powerlight.svg);}"
                "QPushButton:Hover"
                "{border:0px;border-radius:4px;background:transparent;background-image:url(:/tabbaricon/powerhover.svg);}"
                "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;background-image:url(:/"
                "tabbaricon/powerhover.svg);}");
        } break;
        case PublicAttributes::Theme::Dark: {
            m_exitBtn->setStyleSheet(
                "QPushButton "
                "{border:0px;border-radius:4px;background:transparent;background-image:url(:/tabbaricon/powerdark.svg);}"
                "QPushButton:Hover"
                "{border:0px;border-radius:4px;background:transparent;background-image:url(:/tabbaricon/powerhover.svg);}"
                "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;background-image:url(:/"
                "tabbaricon/powerhover.svg);}");
        } break;
    }
    update();
}

void SuspendTabBar::hideTabBar()
{
    hide();
    m_openBtn->hide();
}

void SuspendTabBar::showTabBar()
{
    show();
    m_openBtn->show();
    activateWindow();
    setFocus();
}

void SuspendTabBar::setDeskBtnState(bool isDesktop)
{
    if (isDesktop) {
        m_deskBtnState = false;
        m_switchDeskBtn->setIcon(QIcon(":/tabbaricon/desktop.svg"));
        m_switchDeskBtn->setToolTip(tr("Back to the desktop"));
    } else {
        m_deskBtnState = true;
        m_switchDeskBtn->setIcon(QIcon(":/tabbaricon/screenshare.svg"));
        m_switchDeskBtn->setToolTip(tr("Resume screen projection"));
    }
}

void SuspendTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    /* 反锯齿 */
    QPainter p(this);
    p.setPen(QPen(Qt::NoPen));
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    if (m_theme == PublicAttributes::Theme::Light) {
        p.drawPixmap(rect(), QPixmap(":/tabbaricon/tabbarlight.svg"), QRect());
    } else {
        p.drawPixmap(rect(), QPixmap(":/tabbaricon/tabbardark.svg"), QRect());
    }
    QPainterPath rectPath;
    rectPath.addRoundRect(rect(), 50, 99);
    QPolygon polygon = rectPath.toFillPolygon().toPolygon();
    QRegion region(polygon);
    setProperty("blurRegion", region);
    QColor mainColor;
    if (m_theme == PublicAttributes::Theme::Light) {
        mainColor = QColor(255, 255, 255, TRANSPARENCY);
    } else {
        mainColor = QColor(29, 29, 29, TRANSPARENCY);
    }
    p.fillPath(rectPath, QBrush(mainColor));

    return;
}

void SuspendTabBar::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (hasFocus()) {
        return;
    }

    if (m_isOpen) {
        m_isOpen = false;
        m_setHeightAnimation->setStartValue(WINTHREE_H);
        m_setHeightAnimation->setEndValue(1);
        m_setHeightAnimation->start();
    }
}

void SuspendTabBar::initWholeUI()
{
    setFixedSize(WINTHREE_W, WINTHREE_H);
    m_controlBtn = new kdk::KToolButton(this);
    m_switchDeskBtn = new kdk::KToolButton(this);
    m_exitBtn = new QPushButton(this);

    connect(m_controlBtn, &kdk::KToolButton::clicked, this, &SuspendTabBar::onClicked);
    connect(m_switchDeskBtn, &kdk::KToolButton::clicked, this, &SuspendTabBar::onClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &SuspendTabBar::onExitClicked);

    m_controlBtn->setType(kdk::KToolButtonType::Flat);
    m_switchDeskBtn->setType(kdk::KToolButtonType::Flat);

    m_controlBtn->setFocusPolicy(Qt::NoFocus);
    m_switchDeskBtn->setFocusPolicy(Qt::NoFocus);
    m_exitBtn->setFocusPolicy(Qt::NoFocus);

    m_ctrlBtnState = true;
    m_controlBtn->setIcon(QIcon(":/tabbaricon/control.svg"));
    m_switchDeskBtn->setIcon(QIcon(":/tabbaricon/desktop.svg"));

    m_controlBtn->setToolTip(tr("Counter control (activate Ctrl+H, cancel Ctrl+G)"));
    m_switchDeskBtn->setToolTip(tr("Back to the desktop"));
    m_exitBtn->setToolTip(tr("End screen projection"));

    m_controlBtn->setIconSize(QSize(ICON_W, ICON_H));
    m_switchDeskBtn->setIconSize(QSize(ICON_W, ICON_H));
    m_exitBtn->setFixedSize(QSize(ICON_W, ICON_H));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addWidget(m_controlBtn);
    hLayout->addSpacing(32);
    hLayout->addWidget(m_switchDeskBtn);
    hLayout->addSpacing(32);
    hLayout->addWidget(m_exitBtn);
    hLayout->setContentsMargins(20, 16, 20, 16);

    setLayout(hLayout);
}

void SuspendTabBar::initPartUI()
{
    setFixedSize(WINTWO_W, WINTWO_H);
    m_controlBtn = new kdk::KToolButton(this);
    m_exitBtn = new QPushButton(this);

    connect(m_controlBtn, &kdk::KToolButton::clicked, this, &SuspendTabBar::onClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &SuspendTabBar::onExitClicked);

    m_controlBtn->setType(kdk::KToolButtonType::Flat);

    m_controlBtn->setFocusPolicy(Qt::NoFocus);
    m_exitBtn->setFocusPolicy(Qt::NoFocus);

    m_ctrlBtnState = false;
    m_controlBtn->setIcon(QIcon(":/tabbaricon/controlclose.svg"));

    m_controlBtn->setToolTip(tr("End Counter Control (Ctrl+G)"));
    m_exitBtn->setToolTip(tr("End screen projection"));

    m_controlBtn->setIconSize(QSize(ICON_W, ICON_H));
    m_exitBtn->setFixedSize(QSize(ICON_W, ICON_H));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addWidget(m_controlBtn);
    hLayout->addSpacing(32);
    hLayout->addWidget(m_exitBtn);
    hLayout->setContentsMargins(20, 16, 20, 16);

    setLayout(hLayout);
}

void SuspendTabBar::initAnimation()
{
    setProperty("setHeight", 0);
    m_setHeightAnimation = new QPropertyAnimation(this, "setHeight");
    // 设置动画时长;
    m_setHeightAnimation->setDuration(200);
    // 设置动画变化类型;
    m_setHeightAnimation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_setHeightAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant &value) {
        setFixedHeight(value.toInt());
        activateWindow();
        setFocus();
        if (!m_isOpen && value.toInt() == 1) {
            m_openBtn->setFixedHeight(WIN_H);
        }
    });
}

void SuspendTabBar::onClicked()
{
    kdk::KToolButton *btn = qobject_cast<kdk::KToolButton *>(sender());

    if (btn == m_controlBtn) {
        if (m_ctrlBtnState) {
            Q_EMIT sigBtnClicked(SuspendTabBar::BtnType::ControlBtn);
        } else {
            Q_EMIT sigBtnClicked(SuspendTabBar::BtnType::CloseControlBtn);
        }
    } else if (btn == m_switchDeskBtn) {
        if (m_deskBtnState) {
            Q_EMIT sigBtnClicked(SuspendTabBar::BtnType::ScreenShareBtn);
        } else {
            Q_EMIT sigBtnClicked(SuspendTabBar::BtnType::ReturnDesktopBtn);
        }
    }
}

void SuspendTabBar::onOpenClicked()
{
    m_isOpen = true;
    m_openBtn->setFixedHeight(1);
    m_setHeightAnimation->setStartValue(1);
    m_setHeightAnimation->setEndValue(WINTHREE_H);
    m_setHeightAnimation->start();
}

void SuspendTabBar::onExitClicked()
{
    Q_EMIT sigBtnClicked(SuspendTabBar::BtnType::ExitBtn);
}

OpenBtn::OpenBtn(bool isTool, QWidget *parent) : QWidget(parent)
{
    setProperty("useStyleWindowManager", false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    if (isTool) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::Tool);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    }

    m_openBtn = new kdk::KToolButton(this);
    connect(m_openBtn, &kdk::KToolButton::clicked, this, &OpenBtn::sigClicked);

    m_openBtn->setType(kdk::KToolButtonType::Flat);
    m_openBtn->setFocusPolicy(Qt::NoFocus);
    m_openBtn->setIcon(QIcon(":/tabbaricon/openlight.svg"));
    m_openBtn->setIconSize(QSize(ICON_W, ICON_H));

    setFixedSize(WIN_W, WIN_H);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addWidget(m_openBtn);
    hLayout->setContentsMargins(16, 0, 16, 4);
    setLayout(hLayout);

    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    move(availableGeometry.width() / 2 - width() / 2, 0);
}

void OpenBtn::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    if (m_theme == PublicAttributes::Theme::Light) {
        m_openBtn->setIcon(QIcon(":/tabbaricon/openlight.svg"));
    } else {
        m_openBtn->setIcon(QIcon(":/tabbaricon/opendark.svg"));
    }
    update();
}

void OpenBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    /* 反锯齿 */
    QPainter p(this);
    p.setPen(QPen(Qt::NoPen));
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_theme == PublicAttributes::Theme::Light) {
        p.drawPixmap(rect(), QPixmap(":/tabbaricon/rectanglelight.svg"), QRect());
    } else {
        p.drawPixmap(rect(), QPixmap(":/tabbaricon/rectangledark.svg"), QRect());
    }

    return;
}
