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

#include "kintrowidget.h"
#include "klabel.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace kdk
{

class Q_DECL_HIDDEN KIntroWidgetPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KIntroWidget)
public:
    KIntroWidgetPrivate(KIntroWidget *parent);

protected:
    void changeTheme();

private:
    KIntroWidget *const q_ptr;
    ViewType m_viewType;
    int m_bottomRightRadius;
    int m_topLeftRadius;
    int m_bottomLeftRadius;
    int m_topRightRadius;
    KLabel *m_pMainLabel;
    KLabel *m_pSubLabel;
    QColor m_bkgColor;
    QColor m_mixColor;
    QColor m_borderColor;
    QColor m_titleColor;
};

KIntroWidgetPrivate::KIntroWidgetPrivate(KIntroWidget *parent)
    : q_ptr(parent)
    , m_viewType(ListView)
{
    Q_Q(KIntroWidget);
    m_pMainLabel = new KLabel();
    QFont font = q->font();
    font.setBold(true);
    m_pMainLabel->setFont(font);
    m_pSubLabel = new KLabel();
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(140, 140, 140));
    m_pSubLabel->setPalette(palette);
    m_titleColor = ThemeController::getPaletteColorFromDT("highlight");

    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1) {
        m_bottomLeftRadius = 6;
        m_bottomRightRadius = 6;
        m_topLeftRadius = 6;
        m_topRightRadius = 6;
    } else {
        m_bottomLeftRadius = radius;
        m_bottomRightRadius = radius;
        m_topLeftRadius = radius;
        m_topRightRadius = radius;
    }
}

void KIntroWidgetPrivate::changeTheme()
{
    Q_Q(KIntroWidget);
    initThemeStyle();
    m_bkgColor = ThemeController::getPaletteColorFromDT("tooltipbase");
    m_borderColor = ThemeController::getPaletteColorFromDT("tooltipbase");
    m_borderColor.setAlphaF(0.08);
    if (ThemeController::themeMode() == LightTheme)
        m_mixColor = Qt::black;
    else
        m_mixColor = Qt::white;
    q->update();
}

KIntroWidget::KIntroWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KIntroWidgetPrivate(this))
{
    Q_D(KIntroWidget);
    if (d->m_viewType == ListView)
        setContentsMargins(22, 4, 16, 4);
    else
        setContentsMargins(22, d->m_topLeftRadius + 4, 16, 4);
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setSpacing(0);
    vlayout->addWidget(d->m_pMainLabel);
    //    vlayout->addSpacing(4);
    vlayout->addWidget(d->m_pSubLabel);
    installEventFilter(this);
    //    setFixedSize(rect().width(),74);
    d->m_pMainLabel->setAlignment(Qt::AlignLeft);
    d->m_pSubLabel->setAlignment(Qt::AlignLeft);
    d->changeTheme();
    connect(d->m_gsetting, &QGSettings::changed, this, [=] {
        d->changeTheme();
    });
}

void KIntroWidget::setViewType(ViewType type)
{
    Q_D(KIntroWidget);
    d->m_viewType = type;
    if (d->m_viewType == ListView)
        setContentsMargins(22, 4, 16, 4);
    else
        setContentsMargins(22, d->m_topLeftRadius + 4, 16, 4);
}

void KIntroWidget::setMainLabel(QString str)
{
    Q_D(KIntroWidget);
    d->m_pMainLabel->setText(str);
}

void KIntroWidget::setSubLabel(QString str)
{
    Q_D(KIntroWidget);
    d->m_pSubLabel->setText(str);
}

void KIntroWidget::setBorderRadius(int radius)
{
    Q_D(KIntroWidget);
    d->m_bottomRightRadius = radius;
    d->m_topLeftRadius = radius;
    d->m_bottomLeftRadius = radius;
    d->m_topRightRadius = radius;
    if (d->m_viewType == ListView)
        setContentsMargins(22, 4, 16, 4);
    else
        setContentsMargins(22, d->m_topLeftRadius + 4, 16, 4);
}

void KIntroWidget::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KIntroWidget);
    d->m_bottomRightRadius = bottomRight;
    d->m_topLeftRadius = topLeft;
    d->m_bottomLeftRadius = bottomLeft;
    d->m_topRightRadius = topRight;
    if (d->m_viewType == ListView)
        setContentsMargins(22, 4, 16, 4);
    else
        setContentsMargins(22, d->m_topLeftRadius + 4, 16, 4);
}

void KIntroWidget::setTitleColor(QColor color)
{
    Q_D(KIntroWidget);
    d->m_titleColor = color;
}

void KIntroWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KIntroWidget);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);
    QPainterPath path;
    QPainterPath path1;
    QPainterPath path2;
    QRect drawRect = this->rect();
    path.moveTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));
    path.lineTo(drawRect.topRight() - QPoint(d->m_topRightRadius, 0));
    path.quadTo(drawRect.topRight(), drawRect.topRight() + QPoint(0, d->m_topRightRadius));
    path.lineTo(drawRect.bottomRight() - QPoint(0, d->m_bottomRightRadius));
    path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(d->m_bottomRightRadius, 0));
    path.lineTo(drawRect.bottomLeft() + QPoint(d->m_bottomLeftRadius, 0));
    path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() - QPoint(0, d->m_bottomLeftRadius));
    path.lineTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
    path.quadTo(drawRect.topLeft(), drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));

    switch (d->m_viewType) {
    case ViewType::IconView: {
        //            d->m_pMainLabel->setAlignment(Qt::AlignCenter);
        //            d->m_pSubLabel->setAlignment(Qt::AlignCenter);
        d->m_pMainLabel->setWordWrap(true);
        d->m_pSubLabel->setWordWrap(true);
        path1.moveTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));
        path1.lineTo(drawRect.topRight() - QPoint(d->m_topRightRadius, 0));
        path1.quadTo(drawRect.topRight(), drawRect.topRight() + QPoint(0, d->m_topRightRadius));
        path1.lineTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
        path1.quadTo(drawRect.topLeft(), drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));

        path2.moveTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
        path2.lineTo(drawRect.bottomLeft() - QPoint(0, d->m_bottomLeftRadius));
        path2.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() + QPoint(d->m_bottomLeftRadius, 0));
        path2.lineTo(drawRect.bottomRight() - QPoint(d->m_bottomRightRadius, 0));
        path2.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(0, d->m_bottomRightRadius));
        path2.lineTo(drawRect.topRight() + QPoint(0, d->m_topRightRadius));
        path2.lineTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
    } break;
    case ViewType::ListView: {

        d->m_pMainLabel->setWordWrap(false);
        d->m_pSubLabel->setWordWrap(false);
        path1.moveTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));
        path1.lineTo(drawRect.bottomLeft() + QPoint(d->m_bottomLeftRadius, 0));
        path1.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() - QPoint(0, d->m_bottomLeftRadius));
        path1.lineTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
        path1.quadTo(drawRect.topLeft(), drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));

        path2.moveTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));
        path2.lineTo(drawRect.topRight() - QPoint(d->m_topLeftRadius, 0));
        path2.quadTo(drawRect.topRight(), drawRect.topRight() + QPoint(0, d->m_topRightRadius));
        path2.lineTo(drawRect.bottomRight() - QPoint(0, d->m_bottomRightRadius));
        path2.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(d->m_bottomRightRadius, 0));
        path2.lineTo(drawRect.bottomLeft() + QPoint(d->m_bottomLeftRadius, 0));
        path2.lineTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));

    } break;
    default:
        break;
    }
    painter.setPen(d->m_borderColor);
    painter.drawPath(path);

    painter.setPen(Qt::NoPen);
    painter.setBrush(d->m_titleColor);
    painter.drawPath(path1);
    painter.setBrush(d->m_bkgColor);
    painter.drawPath(path2);
}

bool KIntroWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KIntroWidget);
    if (watched == this) {

        switch (event->type()) {
        case QEvent::Enter:
            d->m_bkgColor = ThemeController::mixColor(ThemeController::getPaletteColorFromDT("tooltipbase"), d->m_mixColor, 0.05);
            update();
            break;
        case QEvent::Leave:
            d->m_bkgColor = ThemeController::getPaletteColorFromDT("tooltipbase");
            update();
            break;
        case QEvent::MouseButtonPress:
            d->m_bkgColor = ThemeController::mixColor(ThemeController::getPaletteColorFromDT("tooltipbase"), d->m_mixColor, 0.1);
            update();
            break;
        case QEvent::MouseButtonRelease:
            d->m_bkgColor = ThemeController::mixColor(ThemeController::getPaletteColorFromDT("tooltipbase"), d->m_mixColor, 0.05);
            update();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

QSize KIntroWidget::sizeHint() const
{
    Q_D(const KIntroWidget);
    return QWidget::sizeHint();
}

void KIntroWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(KIntroWidget);
    if (event->button() == Qt::MouseButton::RightButton) {
        emit rightClick(event->globalPos());
    }
    return QWidget::mousePressEvent(event);
}

void KIntroWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KIntroWidget);
    return QWidget::mouseReleaseEvent(event);
}

}

#include "kintrowidget.moc"
#include "moc_kintrowidget.cpp"
