/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "widget.h"
#include <kwindowsystem.h>
#include <KWindowEffects>
#include <QPainter>
#include <QPainterPath>
#include <QMetaEnum>
#include <QApplication>
#include <QDBusConnection>
#include <QScreen>
#include <QAction>
#include "style_helper.h"
#include "screen_switch.h"
#include "usd_base_class.h"
#include <QApplication>
#include "clib-syslog.h"
#include <QDebug>
#define SHOW_2_MODE_COUNT 2

const static QVector<QPair<UsdBaseClass::eScreenMode, QString>> s_modeImages = {
    {UsdBaseClass::firstScreenMode, ":/img/main.png"},
    {UsdBaseClass::secondScreenMode, ":/img/vice.png"},
    {UsdBaseClass::cloneScreenMode, ":/img/clone.png"},
    {UsdBaseClass::extendScreenMode, ":/img/extend.png"}
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    initWindowHelper();
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setProperty("useStyleWindowManager", false);
    //初始化UI
    this->initUi();
    //键盘快捷操作
    this->addShortcutActions();
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this,[&](WId activeWindowId) {
        //失焦退出
        if (activeWindowId != 0 && activeWindowId != this->winId()) {
            this->close();
        }
    });
    connect(ScreenSwitch::instance(), &ScreenSwitch::sigScreenParamChanged, this, [=]{
        //屏幕参数改变退出
        USD_LOG(LOG_DEBUG, "close : due to screens param changed");
        this->close();
    });
}

Button* Widget::createButton(const QString& text, const QString& img)
{
    Button* button = new Button(text, img, this);
    button->setCheckable(true);
    button->setFixedSize(338, 64);
    return button;
}

void Widget::initTitleUi()
{
    m_titleWidget = new QWidget(this);
    m_titleWidget->setFixedHeight(84);
    m_title = new QLabel(this);

    QFont font;
    font.setPointSizeF(StyleHelper::instance()->getFontSize() + 4);
    m_title->setFont(font);
    m_title->setText(tr("System Screen Projection"));
    m_title->adjustSize();
    m_titleLayout = new QHBoxLayout(m_titleWidget);
    m_titleLayout->addWidget(m_title);
    m_titleLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->addWidget(m_titleWidget);
}

void Widget::initButtons()
{
    m_btnsWidget = new QWidget(this);
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    m_btnsLayout = new QVBoxLayout(m_btnsWidget);
    m_btnsLayout->setContentsMargins(24, 0, 24, 24);
    m_btnsLayout->setSpacing(16);

    //获取屏幕列表
    QStringList screens = ScreenSwitch::instance()->getScreenList();
    int currentMode = ScreenSwitch::instance()->getScreenMode();

    m_screenCount = screens.count();
    if (screens.count() <= 1) {
        screens << tr("None");
    } else {
        screens = screens.mid(0, 2);
    }

    screens << tr("Clone Screen");
    screens << tr("Extend Screen");

    int modes =  m_screenCount <= SHOW_2_MODE_COUNT ? 0 : 2;

    for (int i = modes; i < screens.count(); ++i) {
        const QString& image = s_modeImages[i].second;
        Button* btn = createButton(screens[i], image);
        btn->setMode(s_modeImages[i].first);
        m_btnsLayout->addWidget(btn);
        m_buttonGroup->addButton(btn, i - modes);
        if (currentMode == btn->getMode()) {
            //选中状态
            btn->setStatus(true);
            btn->setChecked(true);
        }
    }
    m_btnsWidget->adjustSize();
    m_mainLayout->addWidget(m_btnsWidget);

    connect(m_buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
          [=](QAbstractButton *button){
        Button * btn = dynamic_cast<Button *>(button);
        QMetaEnum modeEnum = QMetaEnum::fromType<UsdBaseClass::eScreenMode>();
        ScreenSwitch::instance()->setScreenMode(modeEnum.key(btn->getMode()));
        this->close();
    });
}

void Widget::initLine()
{
    //添加分割线
    QFrame* hLine = new QFrame(this);
    hLine->setFixedHeight(1);
    hLine->setFrameShape(QFrame::HLine);
    QPalette palette = hLine->palette();
    QColor color = palette.color(QPalette::WindowText);
    color.setAlphaF(0.08);
    palette.setColor(QPalette::NoRole, color);
    hLine->setPalette(palette);
    m_mainLayout->addWidget(hLine);
}

void Widget::initBottom()
{
    if (initNetworkDisplayButton()) {
        m_bottomWidget = new QWidget(this);
        m_bottomWidget->setFixedHeight(60);
        m_bottomLayout = new QHBoxLayout(m_bottomWidget);
        m_bottomLayout->setContentsMargins(20, 0, 0, 0);
        m_bottomLayout->setAlignment(Qt::AlignLeft);
        //网络显示器
        m_networkDisplay = new QPushButton(this);
        m_networkDisplay->setText(tr("Network display"));
        m_networkDisplay->adjustSize();
        m_bottomLayout->addWidget(m_networkDisplay);
        m_bottomLayout->addStretch();
        m_mainLayout->addWidget(m_bottomWidget);
        connect(m_networkDisplay, &QPushButton::clicked, [=]() {
            m_networkDisplayInterface->asyncCall("discovery");
            close();
        });
    }
}

void Widget::initUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    //标题
    initTitleUi();
    //按钮
    initButtons();
    //分割线
    initLine();
    //底部区域
    initBottom();
    //调整尺寸位置
    this->adjustSize();
    this->setFixedSize(this->width(), this->height());
}

bool Widget::initNetworkDisplayButton()
{
    m_networkDisplayInterface = new QDBusInterface(QStringLiteral("com.kylin.networkdisplays"),
                                                   QStringLiteral("/networkdisplays"),
                                                   QStringLiteral("com.kylin.networkdisplays"),
                                                   QDBusConnection::sessionBus(), this);
    if (m_networkDisplayInterface && m_networkDisplayInterface->isValid()) {
        return true;
    }
    return false;
}

void Widget::showWidget()
{
    this->show();
    this->setWindowCenter();

    /*kdk接口要在show之后调用 */
    if (!UsdBaseClass::isWayland()) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }
}

void Widget::selectedLast()
{
    int current = m_buttonGroup->checkedId();
    int last = current - 1 < 0 ? m_buttonGroup->buttons().count() - 1 : current - 1;
//    qDebug() << last << m_buttonGroup->buttons().count();
    USD_LOG_SHOW_PARAM1(last);
    USD_LOG_SHOW_PARAM1(current);
    USD_LOG_SHOW_PARAM1(m_buttonGroup->buttons().count());
    Button *btn = dynamic_cast<Button *>(m_buttonGroup->button(last));
    if (btn && btn->isEnabled()) {
        btn->setChecked(true);
    }
}

void Widget::selectedNext()
{
    int current = m_buttonGroup->checkedId();
    int next = (current + 1) % m_buttonGroup->buttons().count();
    Button *btn = dynamic_cast<Button *>(m_buttonGroup->button(next));
    if (btn && btn->isEnabled()) {
        btn->setChecked(true);
    }
}

void Widget::confirmCurrent()
{
    int current = m_buttonGroup->checkedId();
    if (current < 0) {
        return;
    }
    Button * btn = dynamic_cast<Button *>(m_buttonGroup->button(current));
    if (btn && btn->isEnabled()) {
        btn->animateClick();
    }
}

void Widget::addShortcutAction(const QKeySequence& keys, Action action)
{
    QAction *act = new QAction(this);
    act->setShortcut(keys);
    this->addAction(act);
    connect(act, &QAction::triggered, this, [=] {
        action();
    });
}

void Widget::addShortcutActions()
{
    addShortcutAction(Qt::Key_Up, std::bind(&Widget::selectedLast, this));
    addShortcutAction(Qt::Key_Down, std::bind(&Widget::selectedNext, this));
    addShortcutAction(Qt::Key_Return, std::bind(&Widget::confirmCurrent, this));
    addShortcutAction(Qt::Key_Enter, std::bind(&Widget::confirmCurrent, this));
}

void Widget::msgReceiveAnotherOne()
{
    selectedNext();
}

void Widget::setWindowCenter()
{
    QRect rect = QApplication::primaryScreen()->geometry();
    int x = rect.x() + (rect.width() - this->width()) / 2;
    int y = rect.y() + (rect.height() - this->height()) / 2;
    setGeometry(QRect(x, y, this->width(), this->height()));
}

void Widget::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() || event->key() == Qt::Key_Display) {
        return;
    }
    this->close();
}

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
QT_END_NAMESPACE

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPainterPath path;
    path.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 12, 12);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);

    pixmapPainter.setPen(Qt::transparent);
    QColor shadowColor = StyleHelper::instance()->isLightStyle() ? QColor("#000000") : QColor("#FFFFFF");
    shadowColor.setAlphaF(0.3);
    pixmapPainter.setBrush(shadowColor);
    pixmapPainter.setCompositionMode(QPainter::CompositionMode_Difference);
    pixmapPainter.drawPath(path);
    pixmapPainter.end();
    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(path);
    // 绘制阴影
    painter.drawPixmap(this->rect(), pixmap, pixmap.rect());

    //毛玻璃区域
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);  //反锯齿;
    painter.setPen(Qt::transparent);
    painter.setBrush(QBrush(StyleHelper::instance()->getColor(QPalette::Window)));

    QColor lineColor = StyleHelper::instance()->getColor(QPalette::WindowText);
    lineColor.setAlphaF(0.15);
    QPen pen(lineColor, 1);
    painter.setPen(pen);
    painter.setOpacity(StyleHelper::instance()->getOpacity());
    painter.drawPath(path);
    m_windowHelper->setBlurEffect(QRegion(path.toFillPolygon().toPolygon()));
    QWidget::paintEvent(event);
}

bool Widget::event(QEvent *event)
{
    if (UsdBaseClass::isWayland()) {
        if (event->type() == QEvent::ActivationChange) {
            if (QApplication::activeWindow() != this) {
                close();
                return true;
            }
        }
    }
    return QWidget::event(event);
}

Widget::~Widget()
{
}

void Widget::initWindowHelper()
{
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
    m_windowHelper->grabKeyboard();
}
