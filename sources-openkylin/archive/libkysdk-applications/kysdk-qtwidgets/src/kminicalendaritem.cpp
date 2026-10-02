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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#pragma execution_character_set("utf-8")

#include "kminicalendaritem.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qevent.h"
#include "qpainter.h"

namespace kdk
{
class KMiniCalendarItemPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KMiniCalendarItem)
public:
    KMiniCalendarItemPrivate(KMiniCalendarItem *parent);

private:
    KMiniCalendarItem *q_ptr;
    KMiniCalendarItem::SelectType selectType; // 选中模式
    KMiniCalendarItem::DayType dayType; // 当前日类型
    bool hover; // 鼠标是否悬停
    bool pressed; // 鼠标是否按下

    bool select; // 是否选中
    bool showLunar; // 显示农历
    bool enableBorder; // 显示边框

    QDate date; // 当前日期
    QString lunar; // 农历信息

    QColor borderColor; // 边框颜色
    QColor weekColor; // 周末颜色
    QColor superColor; // 角标颜色
    QColor lunarColor; // 农历节日颜色

    QColor currentTextColor; // 当前月文字颜色
    QColor otherTextColor; // 其他月文字颜色
    QColor selectTextColor; // 选中日期文字颜色
    QColor hoverTextColor; // 悬停日期文字颜色

    QColor currentLunarColor; // 当前月农历文字颜色
    QColor otherLunarColor; // 其他月农历文字颜色
    QColor selectLunarColor; // 选中日期农历文字颜色
    QColor hoverLunarColor; // 悬停日期农历文字颜色

    QColor currentBgColor; // 当前月背景颜色
    QColor otherBgColor; // 其他月背景颜色
    QColor selectBgColor; // 选中日期背景颜色
    QColor hoverBgColor; // 悬停日期背景颜色
};

KMiniCalendarItemPrivate::KMiniCalendarItemPrivate(KMiniCalendarItem *parent)
    : q_ptr(parent)
{
    Q_Q(KMiniCalendarItem);
    setParent(parent);
    hover = false;
    pressed = false;

    select = false;
    showLunar = true;
    enableBorder = false;
    selectType = KMiniCalendarItem::SelectType_Rect;

    date = QDate::currentDate();
    lunar = "初一";
    dayType = KMiniCalendarItem::DayType_MonthCurrent;

    borderColor = QColor(180, 180, 180);
    weekColor = QColor(255, 0, 0);
    superColor = QColor(255, 129, 6);
    lunarColor = QColor(55, 156, 238);

    currentTextColor = QColor(0, 0, 0);
    otherTextColor = QColor(200, 200, 200);
    selectTextColor = QColor(255, 255, 255);
    hoverTextColor = QColor(250, 250, 250);

    currentLunarColor = QColor(150, 150, 150);
    otherLunarColor = QColor(200, 200, 200);
    selectLunarColor = QColor(255, 255, 255);
    hoverLunarColor = QColor(250, 250, 250);

    currentBgColor = QColor(255, 255, 255);
    otherBgColor = QColor(240, 240, 240);
    selectBgColor = QColor(208, 47, 18);
    hoverBgColor = QColor(204, 183, 180);
}

KMiniCalendarItem::KMiniCalendarItem(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KMiniCalendarItemPrivate(this))
{
    Q_D(KMiniCalendarItem);
}

void KMiniCalendarItem::enterEvent(QEvent *)
{
    Q_D(KMiniCalendarItem);
    d->hover = true;
    update();
}

void KMiniCalendarItem::leaveEvent(QEvent *)
{
    Q_D(KMiniCalendarItem);
    d->hover = false;
    update();
}

void KMiniCalendarItem::mousePressEvent(QMouseEvent *)
{
    Q_D(KMiniCalendarItem);
    d->pressed = true;
    update();

    emit clicked(d->date, d->dayType);
}

void KMiniCalendarItem::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(KMiniCalendarItem);
    d->pressed = false;
    update();
}

void KMiniCalendarItem::paintEvent(QPaintEvent *)
{
    Q_D(KMiniCalendarItem);
    // 绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 绘制背景和边框
    drawBg(&painter);

    // 优先绘制选中状态,其次绘制悬停状态
    if (d->select) {
        drawBgCurrent(&painter, d->selectBgColor);
    } else if (d->hover) {
        drawBgCurrent(&painter, d->hoverBgColor);
    }

    // 绘制日期
    drawDay(&painter);

    // 绘制农历信息
    drawLunar(&painter);
}

void KMiniCalendarItem::drawBg(QPainter *painter)
{
    Q_D(KMiniCalendarItem);
    painter->save();

    // 根据当前类型选择对应的颜色
    QColor bgColor = d->currentBgColor;

    if (d->dayType == DayType_MonthPre || d->dayType == DayType_MonthNext) {
        bgColor = d->otherBgColor;
    }

    if (getEnableBorder())
        painter->setPen(d->borderColor);
    else
        painter->setPen(bgColor);

    painter->setBrush(bgColor);
    painter->drawRect(rect());

    painter->restore();
}

void KMiniCalendarItem::drawBgCurrent(QPainter *painter, const QColor &color)
{
    Q_D(KMiniCalendarItem);
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);

    // 根据设定绘制背景样式
    if (d->selectType == SelectType_Rect) {
        painter->drawRect(rect());
    } else if (d->selectType == SelectType_Circle) {
        int radius = side / 2;
        painter->drawEllipse(QPointF(width / 2, height / 2), radius, radius);
    } else if (d->selectType == SelectType_Triangle) {
        int radius = side / 3;
        QPolygon pts;
        pts.setPoints(3, 1, 1, radius, 1, 1, radius);
        painter->drawRect(rect());
        painter->setBrush(d->superColor);
        painter->drawConvexPolygon(pts);
    } else if (d->selectType == SelectType_CR){
        int radius = 6;
        painter->drawRoundedRect(rect(), radius, radius);
    }

    painter->restore();
}

void KMiniCalendarItem::drawDay(QPainter *painter)
{
    Q_D(KMiniCalendarItem);
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    // 根据当前类型选择对应的颜色
    QColor color = d->currentTextColor;

    if (d->dayType == DayType_MonthPre || d->dayType == DayType_MonthNext) {
        color = d->otherTextColor;
    } else if (d->dayType == DayType_WeekEnd) {
        color = d->weekColor;
    }

    if (d->select) {
        color = d->selectTextColor;
    } else if (d->hover) {
        color = d->hoverTextColor;
    }

    painter->setPen(color);

    if (d->showLunar) {
        QFont font;
        font.setPixelSize(side / 2.7);
        painter->setFont(font);

        QRect dayRect = QRect(0, 0, width, height / 1.5);
        painter->drawText(dayRect, Qt::AlignHCenter | Qt::AlignBottom, QString::number(d->date.day()));
    } else {
        QFont font;
        font.setPixelSize(side / 2.2);
        painter->setFont(font);

        QRect dayRect = QRect(0, 0, width, height);
        painter->drawText(dayRect, Qt::AlignCenter, QString::number(d->date.day()));
    }

    painter->restore();
}

void KMiniCalendarItem::drawLunar(QPainter *painter)
{
    Q_D(KMiniCalendarItem);

    if (!d->showLunar) {
        return;
    }

    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    QStringList listDayName;
    listDayName << "*"
                << "初一"
                << "初二"
                << "初三"
                << "初四"
                << "初五"
                << "初六"
                << "初七"
                << "初八"
                << "初九"
                << "初十"
                << "十一"
                << "十二"
                << "十三"
                << "十四"
                << "十五"
                << "十六"
                << "十七"
                << "十八"
                << "十九"
                << "二十"
                << "廿一"
                << "廿二"
                << "廿三"
                << "廿四"
                << "廿五"
                << "廿六"
                << "廿七"
                << "廿八"
                << "廿九"
                << "三十";

    // 根据当前类型选择对应的颜色
    QColor color = d->currentLunarColor;

    if (d->dayType == DayType_MonthPre || d->dayType == DayType_MonthNext) {
        color = d->otherLunarColor;
    }

    if (d->select) {
        color = d->selectTextColor;
    } else if (d->hover) {
        color = d->hoverTextColor;
    }

    painter->setPen(color);

    QFont font;
    font.setPixelSize(side / 4);
    font.setBold(true);
    painter->setFont(font);

    QRect lunarRect(0, height / 2, width, height / 2.2);
    painter->drawText(lunarRect, Qt::AlignCenter, d->lunar);

    painter->restore();
}

bool KMiniCalendarItem::getSelect() const
{
    Q_D(const KMiniCalendarItem);
    return d->select;
}

bool KMiniCalendarItem::getShowLunar() const
{
    Q_D(const KMiniCalendarItem);
    return d->showLunar;
}

bool KMiniCalendarItem::getEnableBorder() const
{
    Q_D(const KMiniCalendarItem);
    return d->enableBorder;
}

KMiniCalendarItem::SelectType KMiniCalendarItem::getSelectType() const
{
    Q_D(const KMiniCalendarItem);
    return d->selectType;
}

QDate KMiniCalendarItem::getDate() const
{
    Q_D(const KMiniCalendarItem);
    return d->date;
}

QString KMiniCalendarItem::getLunar() const
{
    Q_D(const KMiniCalendarItem);
    return d->lunar;
}

KMiniCalendarItem::DayType KMiniCalendarItem::getDayType() const
{
    Q_D(const KMiniCalendarItem);
    return d->dayType;
}

QColor KMiniCalendarItem::getBorderColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->borderColor;
}

QColor KMiniCalendarItem::getWeekColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->weekColor;
}

QColor KMiniCalendarItem::getSuperColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->superColor;
}

QColor KMiniCalendarItem::getLunarColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->lunarColor;
}

QColor KMiniCalendarItem::getCurrentTextColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->currentTextColor;
}

QColor KMiniCalendarItem::getOtherTextColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->otherTextColor;
}

QColor KMiniCalendarItem::getSelectTextColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->selectTextColor;
}

QColor KMiniCalendarItem::getHoverTextColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->hoverTextColor;
}

QColor KMiniCalendarItem::getCurrentLunarColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->currentLunarColor;
}

QColor KMiniCalendarItem::getOtherLunarColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->otherLunarColor;
}

QColor KMiniCalendarItem::getSelectLunarColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->selectLunarColor;
}

QColor KMiniCalendarItem::getHoverLunarColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->hoverLunarColor;
}

QColor KMiniCalendarItem::getCurrentBgColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->currentBgColor;
}

QColor KMiniCalendarItem::getOtherBgColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->otherBgColor;
}

QColor KMiniCalendarItem::getSelectBgColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->selectBgColor;
}

QColor KMiniCalendarItem::getHoverBgColor() const
{
    Q_D(const KMiniCalendarItem);
    return d->hoverBgColor;
}

QSize KMiniCalendarItem::sizeHint() const
{
    Q_D(const KMiniCalendarItem);
    return QSize(100, 100);
}

QSize KMiniCalendarItem::minimumSizeHint() const
{
    Q_D(const KMiniCalendarItem);
    return QSize(20, 20);
}

void KMiniCalendarItem::setSelect(bool select)
{
    Q_D(KMiniCalendarItem);

    if (d->select != select) {
        d->select = select;
        update();
    }
}

void KMiniCalendarItem::setShowLunar(bool showLunar)
{
    Q_D(KMiniCalendarItem);

    if (d->showLunar != showLunar) {
        d->showLunar = showLunar;
        update();
    }
}

void KMiniCalendarItem::setSelectType(const KMiniCalendarItem::SelectType &selectType)
{
    Q_D(KMiniCalendarItem);

    if (d->selectType != selectType) {
        d->selectType = selectType;
        update();
    }
}

void KMiniCalendarItem::setDate(const QDate &date)
{
    Q_D(KMiniCalendarItem);

    if (d->date != date) {
        d->date = date;
        update();
    }
}

void KMiniCalendarItem::setLunar(const QString &lunar)
{
    Q_D(KMiniCalendarItem);

    if (d->lunar != lunar) {
        d->lunar = lunar;
        update();
    }
}

void KMiniCalendarItem::setDayType(const DayType &dayType)
{
    Q_D(KMiniCalendarItem);

    if (d->dayType != dayType) {
        d->dayType = dayType;
        update();
    }
}

void KMiniCalendarItem::setDate(const QDate &date, const QString &lunar, const DayType &dayType)
{
    Q_D(KMiniCalendarItem);
    d->date = date;
    d->lunar = lunar;
    d->dayType = dayType;
    update();
}

void KMiniCalendarItem::setEnableBorder(bool bVisible)
{
    Q_D(KMiniCalendarItem);

    if (d->enableBorder != bVisible) {
        d->enableBorder = bVisible;
        update();
    }
}

void KMiniCalendarItem::setBorderColor(const QColor &borderColor)
{
    Q_D(KMiniCalendarItem);

    if (d->borderColor != borderColor) {
        d->borderColor = borderColor;
        update();
    }
}

void KMiniCalendarItem::setWeekColor(const QColor &weekColor)
{
    Q_D(KMiniCalendarItem);

    if (d->weekColor != weekColor) {
        d->weekColor = weekColor;
        update();
    }
}

void KMiniCalendarItem::setSuperColor(const QColor &superColor)
{
    Q_D(KMiniCalendarItem);

    if (d->superColor != superColor) {
        d->superColor = superColor;
        update();
    }
}

void KMiniCalendarItem::setLunarColor(const QColor &lunarColor)
{
    Q_D(KMiniCalendarItem);

    if (d->lunarColor != lunarColor) {
        d->lunarColor = lunarColor;
        update();
    }
}

void KMiniCalendarItem::setCurrentTextColor(const QColor &currentTextColor)
{
    Q_D(KMiniCalendarItem);

    if (d->currentTextColor != currentTextColor) {
        d->currentTextColor = currentTextColor;
        update();
    }
}

void KMiniCalendarItem::setOtherTextColor(const QColor &otherTextColor)
{
    Q_D(KMiniCalendarItem);

    if (d->otherTextColor != otherTextColor) {
        d->otherTextColor = otherTextColor;
        update();
    }
}

void KMiniCalendarItem::setSelectTextColor(const QColor &selectTextColor)
{
    Q_D(KMiniCalendarItem);

    if (d->selectTextColor != selectTextColor) {
        d->selectTextColor = selectTextColor;
        update();
    }
}

void KMiniCalendarItem::setHoverTextColor(const QColor &hoverTextColor)
{
    Q_D(KMiniCalendarItem);

    if (d->hoverTextColor != hoverTextColor) {
        d->hoverTextColor = hoverTextColor;
        update();
    }
}

void KMiniCalendarItem::setCurrentLunarColor(const QColor &currentLunarColor)
{
    Q_D(KMiniCalendarItem);

    if (d->currentLunarColor != currentLunarColor) {
        d->currentLunarColor = currentLunarColor;
        update();
    }
}

void KMiniCalendarItem::setOtherLunarColor(const QColor &otherLunarColor)
{
    Q_D(KMiniCalendarItem);

    if (d->otherLunarColor != otherLunarColor) {
        d->otherLunarColor = otherLunarColor;
        update();
    }
}

void KMiniCalendarItem::setSelectLunarColor(const QColor &selectLunarColor)
{
    Q_D(KMiniCalendarItem);

    if (d->selectLunarColor != selectLunarColor) {
        d->selectLunarColor = selectLunarColor;
        update();
    }
}

void KMiniCalendarItem::setHoverLunarColor(const QColor &hoverLunarColor)
{
    Q_D(KMiniCalendarItem);

    if (d->hoverLunarColor != hoverLunarColor) {
        d->hoverLunarColor = hoverLunarColor;
        update();
    }
}

void KMiniCalendarItem::setCurrentBgColor(const QColor &currentBgColor)
{
    Q_D(KMiniCalendarItem);

    if (d->currentBgColor != currentBgColor) {
        d->currentBgColor = currentBgColor;
        update();
    }
}

void KMiniCalendarItem::setOtherBgColor(const QColor &otherBgColor)
{
    Q_D(KMiniCalendarItem);

    if (d->otherBgColor != otherBgColor) {
        d->otherBgColor = otherBgColor;
        update();
    }
}

void KMiniCalendarItem::setSelectBgColor(const QColor &selectBgColor)
{
    Q_D(KMiniCalendarItem);

    if (d->selectBgColor != selectBgColor) {
        d->selectBgColor = selectBgColor;
        update();
    }
}

void KMiniCalendarItem::setHoverBgColor(const QColor &hoverBgColor)
{
    Q_D(KMiniCalendarItem);

    if (d->hoverBgColor != hoverBgColor) {
        d->hoverBgColor = hoverBgColor;
        update();
    }
}
}

#include "kminicalendaritem.moc"
#include "moc_kminicalendaritem.cpp"
