/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kcustomdateedit.h"
#include "themeController.h"
#include <QApplication>
#include <QBrush>
#include <QPainter>
#include <QRandomGenerator>
#include <QRect>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QSvgRenderer>

namespace kdk
{
class KCustomDateEditPrivate : public QObject
{
    Q_DECLARE_PUBLIC(KCustomDateEdit)
    Q_OBJECT
public:
    KCustomDateEditPrivate(KCustomDateEdit *parent);

private slots:
    void onDateSelected(const QDate &date);
    QPixmap loadSvg(const QString &path, int size);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);

private:
    KCustomDateEdit *q_ptr;
    KMiniCalendarWidget *m_pCalendar;
    bool hoverFlag = false;
    bool focusFlag = false;
};

KCustomDateEditPrivate::KCustomDateEditPrivate(KCustomDateEdit *parent)
    : q_ptr(parent)
{
    Q_Q(KCustomDateEdit);
    setParent(parent);
    m_pCalendar = new KMiniCalendarWidget(q);
    m_pCalendar->setFixedSize(316, 344);
    m_pCalendar->setWeekNameFormat(KMiniCalendarWidget::WeekNameFormat_Short);
    connect(m_pCalendar, &KMiniCalendarWidget::clicked, this, &KCustomDateEditPrivate::onDateSelected);
}

void KCustomDateEditPrivate::onDateSelected(const QDate &date)
{
    Q_Q(KCustomDateEdit);
    q->setDate(date);
}

QPixmap KCustomDateEditPrivate::loadSvg(const QString &path, int size)
{
    Q_Q(KCustomDateEdit);
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();

    if (2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }

    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap KCustomDateEditPrivate::drawSymbolicColoredPixmap(const QPixmap &source)
{
    Q_Q(KCustomDateEdit);
    QImage img = source.toImage();

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);

            if (color.alpha() > 0) {
                QColor colorSet = ThemeController::getPaletteColorFromDT("buttontext-active");
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }

    return QPixmap::fromImage(img);
}

KCustomDateEdit::KCustomDateEdit(QWidget *parent)
    : QDateTimeEdit(parent)
    , d_ptr(new KCustomDateEditPrivate(this))
{
    Q_D(KCustomDateEdit);
    this->installEventFilter(this);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void KCustomDateEdit::showCalendar()
{
    Q_D(KCustomDateEdit);
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
    int randomX = QRandomGenerator::global()->bounded(arrowRect.x(), arrowRect.x() + arrowRect.width());
    int randomY = QRandomGenerator::global()->bounded(arrowRect.y(), arrowRect.y() + arrowRect.height());

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(randomX, randomY), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::postEvent(this, event);
}

void KCustomDateEdit::setShowLunar(bool showLunar)
{
    Q_D(KCustomDateEdit);
    d->m_pCalendar->setShowLunar(showLunar);
}

void KCustomDateEdit::updateStyle(bool isDark)
{
    Q_D(KCustomDateEdit);

    if (d->m_pCalendar == nullptr)
        return;

    if (isDark) {
        d->m_pCalendar->setWeekBgColor(QColor(29, 29, 29));
        d->m_pCalendar->setWeekTextColor(QColor(255, 255, 255));
        d->m_pCalendar->setBorderColor(QColor(180, 180, 180));
        d->m_pCalendar->setWeekColor(QColor(255, 255, 255));
        d->m_pCalendar->setLunarColor(QColor(55, 156, 238));
        d->m_pCalendar->setCurrentTextColor(QColor(255, 255, 255));
        d->m_pCalendar->setOtherTextColor(QColor(200, 200, 200));
        d->m_pCalendar->setSelectTextColor(QColor(255, 255, 255));
        d->m_pCalendar->setHoverTextColor(QColor(250, 250, 250));
        d->m_pCalendar->setCurrentLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setOtherLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setSelectLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setHoverLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setCurrentBgColor(QColor(29, 29, 29));
        d->m_pCalendar->setOtherBgColor(QColor(29, 29, 29));
        d->m_pCalendar->setSelectBgColor(ThemeController::getCustomColorFromDT("kbrand-normal"));
        d->m_pCalendar->setHoverBgColor(QColor(180, 180, 180));
    } else {
        d->m_pCalendar->setWeekBgColor(QColor(255, 255, 255));
        d->m_pCalendar->setWeekTextColor(QColor(0, 0, 0));
        d->m_pCalendar->setBorderColor(QColor(180, 180, 180));
        d->m_pCalendar->setWeekColor(QColor(0, 0, 0));
        d->m_pCalendar->setLunarColor(QColor(55, 156, 238));
        d->m_pCalendar->setCurrentTextColor(QColor(0, 0, 0));
        d->m_pCalendar->setOtherTextColor(QColor(200, 200, 200));
        d->m_pCalendar->setSelectTextColor(QColor(255, 255, 255));
        d->m_pCalendar->setHoverTextColor(QColor(250, 250, 250));
        d->m_pCalendar->setCurrentLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setOtherLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setSelectLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setHoverLunarColor(QColor(179, 179, 179));
        d->m_pCalendar->setCurrentBgColor(QColor(255, 255, 255));
        d->m_pCalendar->setOtherBgColor(QColor(255, 255, 255));
        d->m_pCalendar->setSelectBgColor(ThemeController::getCustomColorFromDT("kbrand-normal"));
        d->m_pCalendar->setHoverBgColor(QColor(180, 180, 180));
    }
}

void KCustomDateEdit::mousePressEvent(QMouseEvent *event)
{
    Q_D(KCustomDateEdit);
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
    QPoint point = event->pos();

    if (arrowRect.contains(event->pos())) {
        d->m_pCalendar->setDate(this->date());
        d->m_pCalendar->move(mapToGlobal(QPoint(0, height())));
        d->m_pCalendar->show();
    } else {
        QDateTimeEdit::mousePressEvent(event);
    }
}

void KCustomDateEdit::paintEvent(QPaintEvent *e)
{
    Q_D(KCustomDateEdit);
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); // 反锯齿;
    QBrush brush = QBrush(ThemeController::getCustomColorFromDT("kcomponent-normal"));
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    painter.drawRoundedRect(this->rect(), radius, radius);
    QPixmap pix = d->loadSvg("/usr/share/kylin-user-guide/data/ukui-down-symbolic.svg", 64);
//    QRect rect = QRect(113, 10, 15, 15);
    QRect rect = QRect(this->rect().right()-24, 10, 16, 16);
    painter.drawPixmap(rect, pix);

    QRect rectBoxt = this->rect();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); // 反锯齿;

    if (d->m_pCalendar && d->m_pCalendar->isVisible()) {
        d->focusFlag = true;
    }

    if (d->focusFlag == true) {
        //        QPen pen(palette().brush(QPalette::Active, QPalette::Highlight), 2);
        QPen pen(ThemeController::getPaletteColorFromDT("highlight-active"), 2);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.translate(1, 1);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -2, -2), 6, 6);
    } else if (d->hoverFlag == true) {
        painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
        painter.setBrush(Qt::NoBrush);
        painter.translate(0.5, 0.5);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -1, -1), 6, 6);
    }
}

bool KCustomDateEdit::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(KCustomDateEdit);

    if (QEvent::HoverEnter == event->type()) {
        d->hoverFlag = true;
        repaint();
    } else if (QEvent::HoverLeave == event->type()) {
        d->hoverFlag = false;
        repaint();
    } else if (QEvent::FocusIn == event->type()) {
        d->focusFlag = true;
        repaint();
    } else if (QEvent::FocusOut == event->type()) {
        d->focusFlag = false;
        d->hoverFlag = false;
        repaint();

        if (this->date() != QDateTime::currentDateTime().date() && !d->m_pCalendar->isVisible()) {
            Q_EMIT changeDate();
        }
    }

    return QObject::eventFilter(obj, event);
}
}

#include "kcustomdateedit.moc"
#include "moc_kcustomdateedit.cpp"
