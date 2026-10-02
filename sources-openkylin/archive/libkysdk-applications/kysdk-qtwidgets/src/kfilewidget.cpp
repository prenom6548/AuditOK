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

#include "kfilewidget.h"
#include "themeController.h"
#include <QDebug>
#include <QEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include "accessinfohelper.h"

namespace kdk
{

class KFileWidgetPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KFileWidget)
public:
    KFileWidgetPrivate(KFileWidget *parent);

protected:
    void changeTheme();

private:
    KFileWidget *q_ptr;
    QHBoxLayout *m_pHLayout;
    QVBoxLayout *m_pVLayout;
    QLabel *m_pIconLabel;
    QLabel *m_pMainLabel;
    QLabel *m_pSubLabel;
    QColor m_buttonBkgColor;
    QColor m_iconColor;
    QRect m_buttonRect;
    int m_bottomRightRadius;
    int m_topLeftRadius;
    int m_bottomLeftRadius;
    int m_topRightRadius;
};

KFileWidget::KFileWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KFileWidgetPrivate(this))
{
    Q_D(KFileWidget);
    installEventFilter(this);
    setMouseTracking(true);
    d->m_pMainLabel->installEventFilter(this);
    setFixedHeight(56);
}

void KFileWidget::setMainText(QString str)
{
    Q_D(KFileWidget);
    d->m_pMainLabel->setText(str);
}

void KFileWidget::setSubText(QString str)
{
    Q_D(KFileWidget);
    d->m_pSubLabel->setText(str);
}

void KFileWidget::setIcon(QIcon icon)
{
    Q_D(KFileWidget);
    d->m_pIconLabel->setFixedSize(32, 32);
    d->m_pIconLabel->setPixmap(icon.pixmap(32, 32));
}

void KFileWidget::setBorderRadius(int radius)
{
    Q_D(KFileWidget);
    d->m_bottomRightRadius = radius;
    d->m_topLeftRadius = radius;
    d->m_bottomLeftRadius = radius;
    d->m_topRightRadius = radius;
}

void KFileWidget::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KFileWidget);
    d->m_bottomRightRadius = bottomRight;
    d->m_topLeftRadius = topLeft;
    d->m_bottomLeftRadius = bottomLeft;
    d->m_topRightRadius = topRight;
}

void KFileWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KFileWidget);
    if (d->m_pSubLabel->text().isNull())
        d->m_pSubLabel->setVisible(false);
    d->m_buttonRect = QRect(rect().width() - 17, 0, 16, 16);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setBrush(ThemeController::getPaletteColorFromDT("window-active"));
    if (ThemeController::themeMode() == LightTheme) {
        QColor color(38, 38, 38);
        color.setAlphaF(0.15);
        painter.setPen(color);
    } else {
        QColor color(255, 255, 255);
        color.setAlphaF(0.15);
        painter.setPen(color);
    }

    QPainterPath path;
    QRect rectPath = rect().adjusted(0, 2, -2, 0);

    path.moveTo(rectPath.topRight() - QPoint(d->m_topRightRadius, 0)); // 右上
    path.lineTo(rectPath.topLeft() + QPointF(d->m_topLeftRadius, 0)); // 上方线
    path.quadTo(rectPath.topLeft(), rectPath.topLeft() + QPointF(0, d->m_topLeftRadius)); // 圆角
    path.lineTo(rectPath.bottomLeft() + QPointF(0, -d->m_bottomLeftRadius)); // 左方线
    path.quadTo(rectPath.bottomLeft(), rectPath.bottomLeft() + QPointF(d->m_bottomLeftRadius, 0)); // 圆角
    path.lineTo(rectPath.bottomRight() - QPointF(d->m_bottomRightRadius, 0)); // 下方线
    path.quadTo(rectPath.bottomRight(), rectPath.bottomRight() + QPointF(0, -d->m_bottomRightRadius)); // 圆角
    path.lineTo(rectPath.topRight() + QPointF(0, d->m_topRightRadius)); // 右方线
    path.quadTo(rectPath.topRight(), rectPath.topRight() - QPointF(d->m_topRightRadius, 0)); // 圆角

    if (!isEnabled()) {
        d->m_buttonBkgColor = ThemeController::getPaletteColorFromDT("button-disable");
        d->m_iconColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
    }
    painter.drawPath(path);
    painter.setBrush(d->m_buttonBkgColor);
    painter.drawEllipse(d->m_buttonRect);

    d->m_pHLayout->setContentsMargins(8, 8, d->m_buttonRect.width(), 8);
    QIcon icon(QIcon::fromTheme("application-exit-symbolic"));
    QPixmap pixmap = icon.pixmap(d->m_buttonRect.size());
    painter.drawPixmap(d->m_buttonRect, ThemeController::drawColoredPixmap(pixmap, d->m_iconColor));
}

bool KFileWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KFileWidget);

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::MouseMove: {
        if (d->m_buttonRect.contains(mouseEvent->pos())) {
            d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
            d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kred1");
        } else {
            if (ThemeController::themeMode() == LightTheme) {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kgray-17");
                d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kwhite");
            } else {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
                d->m_buttonBkgColor = ThemeController::getPaletteColorFromDT("window-active");
            }
        }
        update();
        break;
    }
    case QEvent::MouseButtonPress: {
        if (d->m_buttonRect.contains(mouseEvent->pos())) {
            d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
            d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("error-click");
        } else {
            if (ThemeController::themeMode() == LightTheme) {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kgray-17");
                d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kwhite");
            } else {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
                d->m_buttonBkgColor = ThemeController::getPaletteColorFromDT("window-active");
            }
        }

        update();
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (d->m_buttonRect.contains(mouseEvent->pos())) {
            d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
            d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kred1");
            if (isEnabled())
                emit clicked();
        } else {
            if (ThemeController::themeMode() == LightTheme) {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kgray-17");
                d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kwhite");
            } else {
                d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
                d->m_buttonBkgColor = ThemeController::getPaletteColorFromDT("window-active");
            }
        }
        update();
        break;
    }
    case QEvent::Leave: {
        if (ThemeController::themeMode() == LightTheme) {
            d->m_iconColor = ThemeController::getCustomColorFromDT("kgray-17");
            d->m_buttonBkgColor = ThemeController::getCustomColorFromDT("kwhite");
        } else {
            d->m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
            d->m_buttonBkgColor = ThemeController::getPaletteColorFromDT("window-active");
        }
        update();
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

KFileWidgetPrivate::KFileWidgetPrivate(KFileWidget *parent)
    : q_ptr(parent)
{
    Q_Q(KFileWidget);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1) {
        m_bottomRightRadius = 6;
        m_topLeftRadius = 6;
        m_bottomLeftRadius = 6;
        m_topRightRadius = 6;
    } else {
        m_bottomRightRadius = radius;
        m_topLeftRadius = radius;
        m_bottomLeftRadius = radius;
        m_topRightRadius = radius;
    }
    m_pMainLabel = new QLabel();
    m_pSubLabel = new QLabel();

    m_pVLayout = new QVBoxLayout();
    m_pVLayout->setSpacing(0);
    m_pVLayout->addStretch();
    m_pVLayout->addWidget(m_pMainLabel);
    m_pVLayout->addWidget(m_pSubLabel);
    m_pVLayout->addStretch();
    m_pVLayout->setContentsMargins(0, 0, 0, 0);

    m_pIconLabel = new QLabel();
    m_pHLayout = new QHBoxLayout(q);

    m_pHLayout->setContentsMargins(0, 0, 0, 0);
    m_pHLayout->setSpacing(0);
    m_pHLayout->addWidget(m_pIconLabel);
    m_pHLayout->addSpacing(8);
    m_pHLayout->addLayout(m_pVLayout);
    q->setContentsMargins(0, 0, 0, 0);

    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            changeTheme();
        }
    });
    setParent(parent);

    KDK_ALL_INFO_FORMAT(m_pIconLabel,"");
    KDK_ALL_INFO_FORMAT(m_pMainLabel,"");
    KDK_ALL_INFO_FORMAT(m_pSubLabel,"");
}

void KFileWidgetPrivate::changeTheme()
{
    Q_Q(KFileWidget);
    initThemeStyle();

    QPalette subPalette = q->palette();
    QColor color = ThemeController::getPaletteColorFromDT("windowtext-disable");
    subPalette.setColor(QPalette::Text, color);
    m_pSubLabel->setPalette(subPalette);

    if (ThemeController::themeMode() == LightTheme) {
        m_iconColor = ThemeController::getCustomColorFromDT("kgray-17");
        m_buttonBkgColor = ThemeController::getCustomColorFromDT("kwhite");
    } else {
        m_iconColor = ThemeController::getCustomColorFromDT("kwhite");
        m_buttonBkgColor = ThemeController::getPaletteColorFromDT("window-active");
    }
}

}

#include "kfilewidget.moc"
#include "moc_kfilewidget.cpp"
