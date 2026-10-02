/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "titlewidget.h"
#include <QLabel>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QGSettings>
#include <QDebug>

#include "global/global.h"
#include "kmenu.h"

using namespace Global;

TitleWidget::TitleWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnect();
    if (g_config->videoOutputType() == GlobalConfig::VO_OPENGL)
        setAttribute(Qt::WA_TranslucentBackground, true);
    m_windowHelper = new UkuiWindowHelper(parent);
    m_windowHelper->removeTitleBar();
    m_maxBtnHoverDelayTimer = new QTimer(this);
    m_maxBtnHoverDelayTimer->setInterval(500);
    m_maxBtnHoverDelayTimer->setSingleShot(true);
    connect(m_maxBtnHoverDelayTimer, &QTimer::timeout, this, [this]() {
        m_windowHelper->showTileFlyout(m_btnMaxSize->geometry());
    });
    inactiveColor =  Qt::white;
}

TitleWidget::~TitleWidget()
{

}

int TitleWidget::getMenuBtnX()
{
    return m_btnMenu->x();
}

void TitleWidget::setTitle(QString title)
{
    m_fullTitle = title;
    QFontMetrics fontWidth(labTitle->font());//得到每个字符的宽度
    QString show_name = fontWidth.elidedText(m_fullTitle, Qt::ElideRight, labTitle->width() < 720 ? 720 : labTitle->width());//最大宽度150像素
    labTitle->setText(show_name);

    if (show_name != m_fullTitle)
        labTitle->setToolTip(m_fullTitle);
    else
        labTitle->setToolTip("");
}

/** ************************************************************************
 * 更新最大化按钮图标
 ***************************************************************************/
void TitleWidget::updateMaxButtonStatus(bool _isMaxed)
{
    if (_isMaxed)
    {
        m_btnMaxSize->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    }
    else
    {
        m_btnMaxSize->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_btnMaxSize->setToolTip(tr(""));
    }
    m_btnMaxSize->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
}

void TitleWidget::setButtonState(bool _isAvailable)
{
    m_btnMinSize->setVisible(_isAvailable);
    m_btnMaxSize->setVisible(Global::isTablet ? false : _isAvailable);
    if(!m_isHomePage && !Global::isTablet) {
        if (g_config->videoOutputType() == GlobalConfig::VO_WID)
            m_btnMiniMode->setVisible(false);
        else
            m_btnMiniMode->setVisible(_isAvailable);
    }
}

void TitleWidget::setHide()
{
    hide();
}

void TitleWidget::setShow()
{
    if(!m_isMiniMode)
        show();
}

void TitleWidget::setHomePage(bool _isHomePage)
{
    if (_isHomePage == m_isHomePage)
        return;
    m_isHomePage = _isHomePage;

    if (Global::isTablet) {
        m_btnMiniMode->setVisible(false);
        return;
    }

    if (g_config->videoOutputType() == GlobalConfig::VO_WID)
        m_btnMiniMode->setVisible(false);
    else
        m_btnMiniMode->setVisible(!_isHomePage);
}

void TitleWidget::setUIMode(bool isTabletMode)
{
    m_isTabletMode = isTabletMode;
    if (m_isTabletMode) {
        m_btnMaxSize->setVisible(false);
        m_btnMiniMode->setVisible(false);
    }
    else {
        m_btnMaxSize->setVisible(true);
        if (g_config->videoOutputType() == GlobalConfig::VO_OPENGL && !m_isHomePage) {
            m_btnMiniMode->setVisible(true);
        }
    }
}

void TitleWidget::initLayout()
{
    // 标题栏没有主题适配，黑白主题都是黑底白字
    setFixedHeight(TITLE_BAR_FIXED_HEIGHT);

    menu = new KMenu;
    menu->setAccessibleName("kylin-video_KMenu_menu");

    widget = new QWidget(this);
    widget->setObjectName("kylin-video_QWidget_widget");
    // 渐变背景
    widget->setStyleSheet("#kylin-video_QWidget_widget{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(19, 19, 20, 77), stop:1 rgba(255, 255, 255, 0));}");

    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(0,0,0,0);
    h->addWidget(widget);
    QHBoxLayout *hb = new QHBoxLayout(widget);
    hb->setSpacing(8);

    btnIcon = new QPushButton;
    btnIcon->setAccessibleName("kylin-video_QPushButton_btnIcon");
    btnIcon->setFlat(true);
    btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    btnIcon->setFixedSize(QSize(24, 24));
    btnIcon->setIconSize(QSize(24, 24));
    btnIcon->setIcon(QIcon::fromTheme("kylin-video"));
    hb->addWidget(btnIcon);

    labTitle = new QLabel;
    labTitle->setAccessibleName("kylin-video_QLabel_labTitle");
    labTitle->setObjectName("title");
    labTitle->setStyleSheet("#title{color:#ffffff;}");

    hb->addWidget(labTitle);
    hb->setStretchFactor(labTitle, 1);
    labTitle->setText(tr("Video Player"));
    if (isHaWeiKe){
        labTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hb->setContentsMargins(4, 0, 8, 0);
    }else{
        hb->setContentsMargins(8, 0, 4, 0);
    }

    m_btnMenu = new QPushButton;
    m_btnMenu->setAccessibleName("kylin-video_QPushButton_m_btnMenu");
    hb->addWidget(m_btnMenu);
    m_btnMenu->setFixedSize(30, 30);
    m_btnMenu->setProperty("isOptionButton", true);
    m_btnMenu->setProperty("isWindowButton", 0x1);
    m_btnMenu->setProperty("useIconHighlightEffect", 0x2);
    m_btnMenu->setFlat(true);
    m_btnMenu->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_btnMenu->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_btnMenu->setToolTip(tr("Options"));

    m_btnMiniMode = new QPushButton;
    m_btnMiniMode->setAccessibleName("kylin-video_QPushButton_m_btnMiniMode");
    hb->addWidget(m_btnMiniMode);
    m_btnMiniMode->setFixedSize(30, 30);
    m_btnMiniMode->setProperty("isOptionButton", true);
    m_btnMiniMode->setProperty("isWindowButton", 0x1);
    m_btnMiniMode->setProperty("useIconHighlightEffect", 0x2);
    m_btnMiniMode->setFlat(true);
    m_btnMiniMode->setIcon(QIcon::fromTheme("ukui-mini-symbolic"));
    m_btnMiniMode->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_btnMiniMode->setVisible(false);
    m_btnMiniMode->setToolTip(tr("Mini mode Shift+Enter"));

    m_btnMinSize = new QPushButton;
    m_btnMinSize->setAccessibleName("kylin-video_QPushButton_m_btnMinSize");
    hb->addWidget(m_btnMinSize);
    m_btnMinSize->setFixedSize(30, 30);
    m_btnMinSize->setProperty("isWindowButton", 0x1);
    m_btnMinSize->setProperty("useIconHighlightEffect", 0x2);
    m_btnMinSize->setFlat(true);
    m_btnMinSize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_btnMinSize->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_btnMinSize->setToolTip(tr("Minimize"));

    m_btnMaxSize = new QPushButton;
    m_btnMaxSize->setAccessibleName("kylin-video_QPushButton_m_btnMaxSize");
    hb->addWidget(m_btnMaxSize);
    m_btnMaxSize->setFixedSize(30, 30);
    m_btnMaxSize->setProperty("isWindowButton", 0x1);
    m_btnMaxSize->setProperty("useIconHighlightEffect", 0x2);
    m_btnMaxSize->setFlat(true);
    m_btnMaxSize->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_btnMaxSize->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_btnMaxSize->setToolTip(tr(""));
    if (Global::isTablet)
        m_btnMaxSize->setVisible(false);
    m_btnMaxSize->installEventFilter(this);

    m_btnClose = new QPushButton;
    m_btnClose->setAccessibleName("kylin-video_QPushButton_m_btnClose");
    hb->addWidget(m_btnClose);
    m_btnClose->setFixedSize(30, 30);
    m_btnClose->setProperty("isWindowButton", 0x2);
    m_btnClose->setProperty("useIconHighlightEffect", 0x8);
    m_btnClose->setFlat(true);
    m_btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_btnClose->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_btnClose->setToolTip(tr("Close"));

    resetFont();
}

void TitleWidget::initConnect()
{
    connect(m_btnClose,       &QPushButton::clicked,  [this](){emit sigClose();});
    connect(m_btnMaxSize,     &QPushButton::clicked,  [this](){emit sigMaxSize();});
    connect(m_btnMinSize,     &QPushButton::clicked,  [this](){emit sigMiniSize();});
    if (g_config->videoOutputType() == GlobalConfig::VO_OPENGL)
        connect(m_btnMiniMode,    &QPushButton::clicked,  [this](){emit sigMiniMode();});
    connect(m_btnMenu,        &QPushButton::clicked,  [this](){emit sigShowMenu();});
    connect(g_gsettings,    &QGSettings::changed,   [this](QString key){
        if (key == "systemFontSize" || key == "systemFont")
            resetFont();
    });
}

void TitleWidget::resetFont()
{
    QString fontFamily = g_gsettings->get("systemFont").toString();
    double fontSize = g_gsettings->get("systemFontSize").toDouble();

    QFont f(fontFamily);
    f.setPointSizeF(fontSize);
    labTitle->setFont(f);

    QFontMetrics fontWidth(labTitle->font());//得到每个字符的宽度
    QString show_name = fontWidth.elidedText(m_fullTitle, Qt::ElideRight, labTitle->width() < 720 ? 720 : labTitle->width());//最大宽度150像素
    labTitle->setText(show_name);

    if (show_name != m_fullTitle)
        labTitle->setToolTip(m_fullTitle);
    else
        labTitle->setToolTip("");
}

bool TitleWidget::event(QEvent *e)
{
    return QWidget::event(e);
}

#ifndef WHITE_TEST
void TitleWidget::enterEvent(QEvent *e)
{
    if(m_leaveState) {
        m_leaveState = false;
        emit sigCanHide(false);
    }
    return QWidget::enterEvent(e);
}

void TitleWidget::leaveEvent(QEvent *e)
{
    m_leaveState = true;
    emit sigCanHide(true);
    return QWidget::leaveEvent(e);
}

void TitleWidget::resizeEvent(QResizeEvent *e)
{
    QFontMetrics fontWidth(labTitle->font());//得到每个字符的宽度
    QString show_name = fontWidth.elidedText(m_fullTitle, Qt::ElideRight, labTitle->width() < 720 ? 720 : labTitle->width());
    labTitle->setText(show_name);

    if (show_name != m_fullTitle)
        labTitle->setToolTip(m_fullTitle);
    else
        labTitle->setToolTip("");
    return QWidget::resizeEvent(e);
}

void TitleWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit sigMaxSize();
}
#endif

bool TitleWidget::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == m_btnMaxSize) {
        if (event->type() == QEvent::HoverEnter) {
            m_maxBtnHoverDelayTimer->start();
        } else if (event->type() == QEvent::HoverLeave) {
            m_maxBtnHoverDelayTimer->stop();
        }
    }
    // if (QEvent::WindowDeactivate == event->type()) {

    //     // 设置透明度为35%（0.0-1.0范围，0.35对应35%不透明度）
    //     inactiveColor.setAlphaF(0.35);
    //     // 应用到样式表，注意使用HexArgb格式包含透明度信息
        
    //     labTitle->setStyleSheet(QString("#title{color:%1;}").arg(inactiveColor.name(QColor::HexArgb)));
    // }
    // else if(QEvent::WindowActivate == event->type())
    // {
    //     labTitle->setStyleSheet("#title{color:#ffffff;}");
    // }

    return QWidget::eventFilter(obj, event);
}

void TitleWidget::onWindowActivated()
{
    labTitle->setStyleSheet("#title{color:#ffffff;}");
}

void TitleWidget::onWindowDeactivated()
{
    QColor inactiveColor = Qt::white;
    inactiveColor.setAlphaF(0.35);
    labTitle->setStyleSheet(QString("#title{color:%1;}").arg(inactiveColor.name(QColor::HexArgb)));
}
