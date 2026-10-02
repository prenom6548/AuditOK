/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <KWindowEffects>
#include <QPainterPath>

#include "titlebar.h"
#include "view/kyview.h"
#include <gsettingmonitor.h>
#include "global/utils/global_data.h"

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    // 初始化数据
    setInitData();

    // 读取配置文件
    initGsetting();
    m_isBlur = true;
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    m_windowHelper = new UkuiWindowHelper(this);

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

TitleBar::TitleBar(bool isMenu, bool isClose, bool isBlur, QWidget *parent) : QWidget(parent), m_backColor(QColor(255,255,255,125))
{
    // 初始化数据
    setInitData(isMenu, isClose, isBlur);
    if (isBlur) {
        this->setAttribute(Qt::WA_TranslucentBackground, true);
        m_windowHelper = new UkuiWindowHelper(this);

        QPalette p = palette();
        m_backColor = p.color(QPalette::Window);
    }
    // 读取配置文件
    initGsetting();

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

TitleBar::~TitleBar() {}

// 初始化数据
void TitleBar::setInitData(bool isMenu, bool isClose, bool isBlur)
{
    m_isMenu = isMenu;
    m_isClose = isClose;
    m_isBlur = isBlur;
}

// 读取配置文件
void TitleBar::initGsetting()
{
    if (m_isBlur) {
        // 透明度
        this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this,
                      [=]() {
                          GlobalData::getInstance()->getSystemTransparency();
                          this->update();
                      });
    }
}

// 初始化组件
void TitleBar::setWidgetUi()
{
    this->setFixedHeight(GlobalSizeData::TITLEBAR_HEIGHT);

    // 按钮初始化
    m_pIconBtn = new QPushButton(this);
    m_pFuncLabel = new QLabel(this);    
    m_pMinimizeButton = new QPushButton(this->parentWidget());
    m_pCloseButton = new QPushButton(this->parentWidget());
    //设置objectname
    m_pIconBtn->setAccessibleName("kylin-ipmsg_titlebar_QPushButton_mpIconBtn");
    m_pFuncLabel->setAccessibleName("kylin-ipmsg_titlebar_QLabel_mpFuncLabel");
    m_pMinimizeButton->setAccessibleName("kylin-ipmsg_titlebar_QPushButton_mpMinimizeButton");
    m_pCloseButton->setAccessibleName("kylin-ipmsg_titlebar_QPushButton_mpCloseButton");

    // 菜单按钮
    if (m_isMenu) {
        m_menuButton = new QPushButton(this);
        m_menuBar = new menuModule(this);
        m_menuButton->setAccessibleName("kylin-ipmsg_titlebar_QPushButton_mmenuButton");
        m_menuBar->setAccessibleName("kylin-ipmsg_titlebar_menuModule_mmenuBar");
    }

    // 设置按钮布局
    m_pLayout = new QHBoxLayout(this);
    m_pLayout->setObjectName("kylin-ipmsg_titlebar_QHBoxLayout_mpLayout");
    m_pLayout->setContentsMargins(4, 4, 4, 4);
    m_pLayout->setSpacing(0);
    m_pLayout->addSpacing(2);
    m_pLayout->addWidget(m_pIconBtn);
    m_pLayout->addSpacing(8);
    m_pLayout->addWidget(m_pFuncLabel);
    m_pLayout->addStretch(); /*表示加了弹簧*/

    // 菜单按钮
    if (m_isMenu) {
        m_pLayout->addWidget(m_menuButton);
        m_pLayout->addSpacing(4);
    }

    m_pLayout->addWidget(m_pMinimizeButton);
    m_pLayout->addSpacing(4);
    m_pLayout->addWidget(m_pCloseButton);

    this->setLayout(m_pLayout);

    // 设置信号和槽函数
    connect(m_pMinimizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));

    if (m_isClose) {
        connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    }

    switch (GlobalData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCMode();        
            break;
        case CurrentMode::HMode:
        slotChangeHScreenMode();
            break;
        case CurrentMode::VMode:
        slotChangeVScreenMode();
            break;
    }
    connect(GlobalData::getInstance(), &GlobalData::signalChangePC, this, &TitleBar::slotChangePCMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatH, this, &TitleBar::slotChangeHScreenMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatV, this, &TitleBar::slotChangeVScreenMode);
    if(m_isMenu){
        connect(m_menuButton, &QPushButton::clicked, this, &TitleBar::onMenuButtonClicked);
    }
}

// 设置组件样式
void TitleBar::setWidgetStyle()
{
    //毛玻璃
    if (m_isBlur) {
        this->setProperty("useSystemStyleBlur", true);
        this->setAttribute(Qt::WA_TranslucentBackground, true);
    }

    // 设置对象名
    m_pFuncLabel->setObjectName("whiteLabel");
    m_pMinimizeButton->setObjectName("minimizeButton");
    m_pCloseButton->setObjectName("closeButton");

    // 设置悬浮提示
    m_pMinimizeButton->setToolTip(tr("Minimize"));
    m_pCloseButton->setToolTip(tr("Close"));

    // 设置标题栏文字
    m_pFuncLabel->setText(tr("Messages")); /*给label设置信息*/
    QFont font = GlobalData::getInstance()->getFontSize14px();
    m_pFuncLabel->setFont(font);

    m_pIconBtn->setIconSize(QSize(24, 24));               /*设置ico类型图片大小*/
    m_pIconBtn->setIcon(QIcon::fromTheme("kylin-ipmsg")); /*使用fromTheme函数调用库中的图片*/

    if(m_isMenu){
        m_menuButton->setToolTip(tr("Options"));
        m_menuButton->setProperty("isOptionButton", true);
        m_menuButton->setProperty("isWindowButton", 0x1);
        m_menuButton->setProperty("useIconHighlightEffect", 0x2);
        m_menuButton->setFlat(true);
        m_menuButton->setFixedSize(30, 30);
        m_menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
        m_menuButton->setFocusPolicy(Qt::NoFocus);
        m_menuButton->raise();
    }

    m_pMinimizeButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_pMinimizeButton->setIconSize(QSize(16, 16));
    m_pMinimizeButton->setFixedSize(QSize(30, 30));
    m_pMinimizeButton->setProperty("isWindowButton", 0x1);
    m_pMinimizeButton->setProperty("useIconHighlightEffect", 0x2);
    m_pMinimizeButton->setFlat(true);
    m_pMinimizeButton->setFocusPolicy(Qt::NoFocus);

    m_pCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_pCloseButton->setIconSize(QSize(16, 16));
    m_pCloseButton->setFixedSize(QSize(30, 30));
    m_pCloseButton->setProperty("isWindowButton", 0x2);
    m_pCloseButton->setProperty("useIconHighlightEffect", 0x8);
    m_pCloseButton->setFlat(true);
    m_pCloseButton->setFocusPolicy(Qt::NoFocus);

    // 设置按钮样式
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_pIconBtn->setStyleSheet(btnStyle);

    this->update();
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    // qDebug() << "TitleBar::paintEvent";
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    if (m_isBlur) {
        /* 反锯齿 */
        QPainterPath rectPath;
        rectPath.addRoundedRect(rect(), 0, 0);
        /* 开启背景模糊效果（毛玻璃） */
        // KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(rectPath.toFillPolygon().toPolygon()));

        QStyleOption opt;
        opt.init(this);
        p.setPen(Qt::NoPen);
        QPalette pal(this->palette());
        pal.setColor(QPalette::Base,QColor(m_backColor));
        this->setPalette(pal);
        QBrush brush = QBrush(m_backColor);
        p.setBrush(brush);
        p.drawRoundedRect(opt.rect, 0, 0);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        // p.fillPath(rectPath, brush);
    }
}

bool TitleBar::event(QEvent *event)
{
    if (m_isBlur) {
        if (event->type() == QEvent::WindowActivate) {
            GlobalData::getInstance()->getSystemTransparency();
            m_backColor.setAlpha(GlobalSizeData::BLUR_TRANSPARENCY);
            m_windowHelper->setBlurEffect(QRegion(), 0, true);
            setFocusFade(true);

        }
        else if (event->type() == QEvent::WindowDeactivate){
            m_backColor.setAlpha(255);
            m_windowHelper->setBlurEffect(QRegion(), 0, false);
            setFocusFade(false);
        }
    }
    return QWidget::event(event);
}


// 设置标题栏文字
void TitleBar::setTitleText(QString titleStr)
{
    this->m_pFuncLabel->setText(titleStr);
}

void TitleBar::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUIDark) {
        m_backColor = QColor("#000000");
    } else {
        m_backColor = QColor("#FFFFFF");
    }
    m_backColor.setAlpha(255);
    this->update();
}

void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel()) {
        if (pButton == m_pMinimizeButton) {
            pWindow->showMinimized();
        } else if (pButton == m_pCloseButton) {
            pWindow->close();
        }
    }
}

void TitleBar::onMenuButtonClicked()
{
    if(m_menuBar && m_menuBar->m_menu && m_isMenu){
        QPoint buttonPos = m_menuButton->mapToGlobal(QPoint(0, m_menuButton->height()));
        m_menuBar->m_menu->exec(buttonPos);
    }
}

void TitleBar::slotChangeHScreenMode()
{
    this->setFixedHeight(GlobalSizeData::TITLEBAR_MAX_HEIGHT);
    m_pIconBtn->setIconSize(QSize(32, 32));
    if (m_isMenu) {
        m_menuButton->setFixedSize(QSize(48, 48));
    } 
    m_pMinimizeButton->setFixedSize(QSize(48, 48));
    m_pCloseButton->setFixedSize(QSize(48, 48));
    return;
}

void TitleBar::slotChangeVScreenMode()
{
    this->setFixedHeight(GlobalSizeData::TITLEBAR_MAX_HEIGHT);
    m_pIconBtn->setIconSize(QSize(32, 32));        
    if (m_isMenu) {
        m_menuButton->setFixedSize(QSize(48, 48));
    }
    m_pMinimizeButton->setFixedSize(QSize(48, 48));
    m_pCloseButton->setFixedSize(QSize(48, 48));
    return;
}

void TitleBar::slotChangePCMode()
{
    this->setFixedHeight(GlobalSizeData::TITLEBAR_HEIGHT);
    m_pIconBtn->setIconSize(QSize(24, 24));  
    if (m_isMenu) {
        m_menuButton->setFixedSize(QSize(30, 30));
    }
    m_pMinimizeButton->setFixedSize(QSize(30, 30));
    m_pCloseButton->setFixedSize(QSize(30, 30));
    return;
}

void TitleBar::setFocusFade(bool hasFocus)
{
    if(m_hasFocus == hasFocus)
        return;

    m_hasFocus = hasFocus;
    // 根据焦点状态设置功能标签的文字颜色
    if (m_pFuncLabel) {
        if (hasFocus) {
            // 有焦点时恢复正常颜色
            m_pFuncLabel->setStyleSheet("");
        } else {
            // 失去焦点时设置为灰色
            m_pFuncLabel->setStyleSheet("color: rgb(168, 168, 168)");
        }
    }

}
