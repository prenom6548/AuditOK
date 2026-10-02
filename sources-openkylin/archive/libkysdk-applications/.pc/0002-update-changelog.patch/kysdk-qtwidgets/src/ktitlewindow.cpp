/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "ktitlewindow.h"
#include "themeController.h"
#include "ukuistylehelper/ukui-decoration-manager.h"
#include "ukuistylehelper/ukui-shell-manager.h"
#include "xatom-helper.h"
#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
namespace kdk
{

class TitleButton : public QPushButton
{
    Q_OBJECT
public:
    TitleButton(QWidget *widget = nullptr);

private:
    QColor m_color;

protected:
    void paintEvent(QPaintEvent *event);
};

class Q_DECL_HIDDEN KTitleWindowPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTitleWindow)

public:
    KTitleWindowPrivate(KTitleWindow *parent);

protected:
    void changeTheme();

private:
    KTitleWindow *q_ptr;
    QWidget *m_pTitleWidget;
    QWidget *m_pBaseWidget;
    KPushButton *m_pAddBtn;
    TitleButton *m_pColorBtn;
    TitleButton *m_pMenuBtn;
    QPushButton *m_pCloseBtn;
    QHBoxLayout *m_pTitleWidgetLayou;
    QVBoxLayout *m_pVLayout;
    QColor m_titleColor;
};

KTitleWindow::KTitleWindow(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KTitleWindowPrivate(this))
{
    Q_D(KTitleWindow);
    installEventFilter(this);
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    setMinimumSize(390, 364);
}

KPushButton *KTitleWindow::addButton()
{
    Q_D(KTitleWindow);
    return d->m_pAddBtn;
}

QPushButton *KTitleWindow::colorButton()
{
    Q_D(KTitleWindow);
    return static_cast<QPushButton *>(d->m_pColorBtn);
}

QPushButton *KTitleWindow::menuButton()
{
    Q_D(KTitleWindow);
    return static_cast<QPushButton *>(d->m_pMenuBtn);
}

QWidget *KTitleWindow::baseWidget()
{
    Q_D(KTitleWindow);
    return d->m_pBaseWidget;
}

void KTitleWindow::setTitleColor(QColor color)
{
    Q_D(KTitleWindow);
    d->m_titleColor = color;
}

void KTitleWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(KTitleWindow);

    QWidget::resizeEvent(event);
    d->m_pTitleWidget->setFixedWidth(width());
    repaint();
}

void KTitleWindow::paintEvent(QPaintEvent *event)
{
    Q_D(KTitleWindow);

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ThemeController::getPaletteColorFromDT("base"));
    int radius = ThemeController::getRadiusFromDT("kradius-window");
    if (radius == -1)
        radius = 12;
    painter.drawRoundedRect(this->rect(), radius, radius);

    d->m_pAddBtn->setIconColor(d->m_titleColor);
    d->m_pColorBtn->setIcon(ThemeController::drawColoredPixmap(d->m_pColorBtn->icon().pixmap(24, 24), Qt::white));
    d->m_pMenuBtn->setIcon(ThemeController::drawColoredPixmap(d->m_pMenuBtn->icon().pixmap(24, 24), Qt::white));
    d->m_pCloseBtn->setIcon(ThemeController::drawColoredPixmap(d->m_pCloseBtn->icon().pixmap(24, 24), Qt::white));

    QPainterPath path1;
    QRect rect = this->rect().adjusted(0, 0, 1, 1);
    if (isActiveWindow()) {
        if (!d->m_pTitleWidget->isVisible()) {
            d->m_pTitleWidget->show();
            d->m_pVLayout->setContentsMargins(8, 40, 8, 0);
        }
        path1.lineTo(rect.topLeft() + QPoint(12, 0));
        path1.lineTo(rect.topRight() - QPoint(12, 0));
        path1.quadTo(rect.topRight(), rect.topRight() + QPoint(0, 12));
        path1.lineTo(rect.topRight() + QPoint(0, 40));
        path1.lineTo(rect.topLeft() + QPoint(0, 40));
        path1.lineTo(rect.topLeft() + QPoint(0, 12));
        path1.quadTo(rect.topLeft(), rect.topLeft() + QPoint(12, 0));
    } else {
        if (d->m_pTitleWidget->isVisible()) {
            d->m_pTitleWidget->hide();
            d->m_pVLayout->setContentsMargins(8, 12, 8, 0);
        }
        path1.lineTo(rect.topLeft() + QPoint(12, 0));
        path1.lineTo(rect.topRight() - QPoint(12, 0));
        path1.quadTo(rect.topRight(), rect.topRight() + QPoint(0, 12));
        path1.lineTo(rect.topRight() + QPoint(0, 12));
        path1.lineTo(rect.topLeft() + QPoint(0, 12));
        path1.lineTo(rect.topLeft() + QPoint(0, 12));
        path1.quadTo(rect.topLeft(), rect.topLeft() + QPoint(12, 0));
    }
    painter.setBrush(d->m_titleColor);
    painter.drawPath(path1);
}

bool KTitleWindow::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KTitleWindow);
    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        if ((event->type() == QEvent::PlatformSurface || event->type() == QEvent::Show || event->type() == QEvent::UpdateRequest)) {
#ifdef USE_UKUI_PROTOCOL
            if (UkuiShellManager::getInstance()->ukuishellReady())
                UkuiShellManager::getInstance()->removeHeaderBar(this->windowHandle());
            else
                UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
#else
            UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
#endif
        }
    }
    return QWidget::eventFilter(watched, event);
}

KTitleWindowPrivate::KTitleWindowPrivate(KTitleWindow *parent)
    : q_ptr(parent)
{
    Q_Q(KTitleWindow);
    m_titleColor = ThemeController::getPaletteColorFromDT("highlight");
    m_pTitleWidget = new QWidget(q);
    m_pTitleWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_pTitleWidget->setContentsMargins(0, 0, 0, 0);
    m_pTitleWidget->setFixedHeight(40);
    m_pTitleWidgetLayou = new QHBoxLayout(m_pTitleWidget);

    m_pAddBtn = new KPushButton();
    m_pAddBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pAddBtn->setBackgroundColor(Qt::white);
    m_pAddBtn->setFixedSize(24, 24);
    m_pAddBtn->setBorderRadius(12);
    m_pAddBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));

    m_pColorBtn = new TitleButton();
    //    m_pColorBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pColorBtn->setFlat(true);
    m_pColorBtn->setFixedSize(24, 24);
    m_pColorBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    m_pMenuBtn = new TitleButton();
    //    m_pMenuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pMenuBtn->setFlat(true);
    m_pMenuBtn->setFixedSize(24, 24);
    m_pMenuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    m_pCloseBtn = new QPushButton();
    m_pCloseBtn->setProperty("isWindowButton", 0x02);
    m_pCloseBtn->setProperty("useIconHighlightEffect", 0x08);
    m_pCloseBtn->setFlat(true);
    m_pCloseBtn->setFixedSize(24, 24);
    m_pCloseBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    connect(m_pCloseBtn, &QPushButton::clicked, q, [=] {
        q->close();
    });

    m_pBaseWidget = new QWidget();

    m_pVLayout = new QVBoxLayout(q);

    m_pTitleWidgetLayou->setSpacing(8);
    m_pTitleWidgetLayou->setContentsMargins(8, 8, 8, 8);
    m_pTitleWidgetLayou->addWidget(m_pAddBtn);
    m_pTitleWidgetLayou->addStretch();
    m_pTitleWidgetLayou->addWidget(m_pColorBtn);
    m_pTitleWidgetLayou->addWidget(m_pMenuBtn);
    m_pTitleWidgetLayou->addWidget(m_pCloseBtn);
    m_pVLayout->addLayout(m_pTitleWidgetLayou);
    m_pVLayout->addWidget(m_pBaseWidget);

    m_pBaseWidget->setBackgroundRole(QPalette::Base);
    connect(m_gsetting, &QGSettings::changed, this, [=] {
        changeTheme();
        m_pBaseWidget->setBackgroundRole(QPalette::Base);
    });
}

void KTitleWindowPrivate::changeTheme()
{
    Q_Q(KTitleWindow);
    initThemeStyle();
}

TitleButton::TitleButton(QWidget *widget)
{
}

void TitleButton::paintEvent(QPaintEvent *event)
{
    QStyleOptionButton option;
    initStyleOption(&option);
    m_color = Qt::transparent;
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (option.state.testFlag(QStyle::State_MouseOver)) {
        m_color = ThemeController::getPaletteColorFromDT("brighttext");
        if (option.state.testFlag(QStyle::State_Sunken)) {
            if (ThemeController::themeMode() == LightTheme)
                m_color.setAlphaF(0.28);
            else
                m_color.setAlphaF(0.15);
        } else {
            if (ThemeController::themeMode() == LightTheme)
                m_color.setAlphaF(0.15);
            else
                m_color.setAlphaF(0.28);
        }
    }
    painter.setBrush(m_color);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(this->rect(), 6, 6);

    QRect rect = option.rect.adjusted(0, 0, 1, 1);
    QRect iconRect(rect.center().x() - option.iconSize.width() / 2,
                   rect.center().y() - option.iconSize.height() / 2,
                   option.iconSize.width(),
                   option.iconSize.height());
    painter.drawPixmap(iconRect, option.icon.pixmap(option.iconSize));
}

}

#include "ktitlewindow.moc"
#include "moc_ktitlewindow.cpp"
