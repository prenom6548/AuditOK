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

#include "kminicalendarwidget.h"
#include "kborderlessbutton.h"
#include "kpushbutton.h"
#include "qdatetime.h"
#include "qfontdatabase.h"
#include "qlayout.h"
#include "themeController.h"
#include <QPainter>
#include <QPainterPath>

#include <QGraphicsDropShadowEffect>
//#include "kshadowhelper.h" //2503

namespace kdk
{
class KMiniCalendarWidgetPrivate : public QObject
{
    Q_DECLARE_PUBLIC(KMiniCalendarWidget)
    Q_OBJECT
public:
    KMiniCalendarWidgetPrivate(KMiniCalendarWidget *parent);

private:
    KMiniCalendarWidget *q_ptr;
    KMiniCalendarWidget::CalendarStyle calendarStyle; // 整体样式
    KMiniCalendarWidget::WeekNameFormat weekNameFormat; // 星期名称格式
    KMiniCalendarWidget::SelectType selectType; // 选中模式
    bool btnClick; // 按钮单击,避开下拉选择重复触发
    QLabel *labelYearMonth; // 年月
    QList<QLabel *> labWeeks; // 顶部星期名称
    QList<KMiniCalendarItem *> dayItems; // 日期元素

    QDate date; // 当前日期

    QColor weekTextColor; // 星期名称文字颜色
    QColor weekBgColor; // 星期名称背景色

    bool showLunar; // 显示农历
    bool enableBorder; // 显示边框

    QColor borderColor; // 边框颜色
    QColor weekColor; // 周末颜色
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

KMiniCalendarWidgetPrivate::KMiniCalendarWidgetPrivate(KMiniCalendarWidget *parent)
    : q_ptr(parent)
{
    Q_Q(KMiniCalendarWidget);
    btnClick = false;

    calendarStyle = KMiniCalendarWidget::CalendarStyle_Red;
    weekNameFormat = KMiniCalendarWidget::WeekNameFormat_Short;
    date = QDate::currentDate();

    weekBgColor = QColor(255, 255, 255);
    weekBgColor = QColor(255, 255, 255);

    showLunar = false;
    enableBorder = false;
    selectType = KMiniCalendarWidget::SelectType_Circle;

    borderColor = QColor(180, 180, 180);
    weekColor = QColor(0, 0, 0);
    lunarColor = QColor(55, 156, 238);

    currentTextColor = QColor(0, 0, 0);
    otherTextColor = QColor(200, 200, 200);
    selectTextColor = QColor(255, 255, 255);
    hoverTextColor = QColor(250, 250, 250);

    currentLunarColor = QColor(179, 179, 179);
    otherLunarColor = QColor(179, 179, 179);
    selectLunarColor = QColor(179, 179, 179);
    hoverLunarColor = QColor(179, 179, 179);

    currentBgColor = QColor(255, 255, 255);
    otherBgColor = QColor(255, 255, 255);
    selectBgColor = QColor(55, 144, 250);
    hoverBgColor = QColor(180, 180, 180);
}

KMiniCalendarWidget::KMiniCalendarWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KMiniCalendarWidgetPrivate(this))
{
    Q_D(KMiniCalendarWidget);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
//    effects::KShadowHelper::self()->setWidget(this,15);

     QGraphicsDropShadowEffect *m_pShadow_effect = new QGraphicsDropShadowEffect(this);
     m_pShadow_effect->setOffset(0, 0);
     m_pShadow_effect->setColor(ThemeController::getPaletteColorFromDT("text", QPalette::Disabled));
     m_pShadow_effect->setBlurRadius(15);
     setContentsMargins(10,10,10,10);
     this->setGraphicsEffect(m_pShadow_effect);

    initWidget();
    initStyle();
    initDate();
}

KMiniCalendarWidget::~KMiniCalendarWidget()
{
}

void KMiniCalendarWidget::initWidget()
{
    Q_D(KMiniCalendarWidget);
    this->setObjectName("miniCalendarWidget");

    // 顶部widget
    QWidget *widgetTop = new QWidget();
    widgetTop->setObjectName("widgetTop");
    widgetTop->setMinimumHeight(36);

    // 上个月按钮
    kdk::KBorderlessButton *btnPrevMonth = new kdk::KBorderlessButton();
    btnPrevMonth->setObjectName("btnPrevMonth");
    btnPrevMonth->setFixedSize(45, 36);
    btnPrevMonth->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    btnPrevMonth->setIcon(QIcon::fromTheme("ukui-start-symbolic"));

    // 下个月按钮
    kdk::KBorderlessButton *btnNextMonth = new kdk::KBorderlessButton();
    btnNextMonth->setObjectName("btnNextMonth");
    btnNextMonth->setFixedSize(45, 36);
    btnNextMonth->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    btnNextMonth->setIcon(QIcon::fromTheme("ukui-end-symbolic"));

    d->labelYearMonth = new QLabel();
    d->labelYearMonth->setFixedSize(96, 36);
    d->labelYearMonth->setAlignment(Qt::AlignCenter);
    QFont labelFont;
    labelFont.setFamily("Microsoft Yahei");
    labelFont.setPixelSize(14);
    d->labelYearMonth->setFont(labelFont);

    // 顶部横向布局
    QHBoxLayout *layoutTop = new QHBoxLayout(widgetTop);
    layoutTop->setContentsMargins(0, 0, 0, 0);
    layoutTop->addWidget(btnPrevMonth);
    layoutTop->addWidget(d->labelYearMonth);
    layoutTop->addWidget(btnNextMonth);

    // 星期widget
    QWidget *widgetWeek = new QWidget();
    widgetWeek->setObjectName("widgetWeek");
    widgetWeek->setMinimumHeight(30);

    // 星期布局
    QHBoxLayout *layoutWeek = new QHBoxLayout(widgetWeek);
    layoutWeek->setMargin(0);
    layoutWeek->setSpacing(0);

    for (int i = 0; i < 7; i++) {
        QLabel *lab = new QLabel;
        lab->setAlignment(Qt::AlignCenter);
        layoutWeek->addWidget(lab);
        d->labWeeks.append(lab);
    }

    setWeekNameFormat(WeekNameFormat_Long);

    // 日期标签widget
    QWidget *widgetBody = new QWidget();
    widgetBody->setObjectName("widgetBody");

    // 日期标签布局
    QGridLayout *layoutBody = new QGridLayout(widgetBody);
    layoutBody->setMargin(0);
    layoutBody->setHorizontalSpacing(0);
    layoutBody->setVerticalSpacing(0);

    // 逐个添加日标签
    for (int i = 0; i < 42; i++) {
        KMiniCalendarItem *lab = new KMiniCalendarItem();
        lab->setObjectName("KMiniCalendarItem");
        connect(lab, SIGNAL(clicked(QDate, KMiniCalendarItem::DayType)), this, SLOT(slotClicked(QDate, KMiniCalendarItem::DayType)));
        layoutBody->addWidget(lab, i / 7, i % 7);
        d->dayItems.append(lab);
    }

    // 主布局
    QVBoxLayout *verLayoutCalendar = new QVBoxLayout(this);
    verLayoutCalendar->setContentsMargins(15, 10, 15, 15);
    verLayoutCalendar->setSpacing(0);
    verLayoutCalendar->addWidget(widgetTop);
    verLayoutCalendar->addWidget(widgetWeek);
    verLayoutCalendar->addWidget(widgetBody, 1);

    // 绑定按钮和下拉框信号
    connect(btnPrevMonth, SIGNAL(clicked(bool)), this, SLOT(showPreviousMonth()));
    connect(btnNextMonth, SIGNAL(clicked(bool)), this, SLOT(showNextMonth()));
}

void KMiniCalendarWidget::initStyle()
{
    Q_D(KMiniCalendarWidget);
    // 设置样式
    QStringList qss;
    // 星期名称样式
    qss.append(QString("QLabel{background:%1;color:%2;}").arg(d->weekBgColor.name()).arg(d->weekTextColor.name()));
    // 边框
    qss.append("QWidget#widgetBody{border:0px;}");

    // 顶部下拉框
    qss.append(QString("QToolButton{padding:0px;background:none;border:none;border-radius:5px;}"));
    // qss.append(QString("QToolButton:hover{background:rgb(180,180,180);color:#FFFFFF;}"));

    // 自定义日控件颜色
    QString strSelectType;

    if (d->selectType == SelectType_Rect) {
        strSelectType = "SelectType_Rect";
    } else if (d->selectType == SelectType_Circle) {
        strSelectType = "SelectType_Circle";
    } else if (d->selectType == SelectType_Triangle) {
        strSelectType = "SelectType_Triangle";
    }

    qss.append(QString("KMiniCalendarItem{qproperty-showLunar:%1;}").arg(d->showLunar));
    qss.append(QString("KMiniCalendarItem{qproperty-selectType:%1;}").arg(strSelectType));
    qss.append(QString("KMiniCalendarItem{qproperty-borderColor:%1;}").arg(d->borderColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-weekColor:%1;}").arg(d->weekColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-lunarColor:%1;}").arg(d->lunarColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-currentTextColor:%1;}").arg(d->currentTextColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-otherTextColor:%1;}").arg(d->otherTextColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-selectTextColor:%1;}").arg(d->selectTextColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-hoverTextColor:%1;}").arg(d->hoverTextColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-currentLunarColor:%1;}").arg(d->currentLunarColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-otherLunarColor:%1;}").arg(d->otherLunarColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-selectLunarColor:%1;}").arg(d->selectLunarColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-hoverLunarColor:%1;}").arg(d->hoverLunarColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-currentBgColor:%1;}").arg(d->currentBgColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-otherBgColor:%1;}").arg(d->otherBgColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-selectBgColor:%1;}").arg(d->selectBgColor.name()));
    qss.append(QString("KMiniCalendarItem{qproperty-hoverBgColor:%1;}").arg(d->hoverBgColor.name()));

    for (KMiniCalendarItem *dayItem : d->dayItems) {
        dayItem->setShowLunar(d->showLunar);
        dayItem->setSelectType(static_cast<kdk::KMiniCalendarItem::SelectType>(d->selectType));
        dayItem->setBorderColor(d->borderColor.name());
        dayItem->setWeekColor(d->weekColor.name());
        dayItem->setLunarColor(d->lunarColor.name());
        dayItem->setCurrentTextColor(d->currentTextColor.name());
        dayItem->setOtherTextColor(d->otherTextColor.name());
        dayItem->setSelectTextColor(d->selectTextColor.name());
        dayItem->setHoverTextColor(d->hoverTextColor.name());
        dayItem->setCurrentLunarColor(d->currentLunarColor.name());
        dayItem->setOtherLunarColor(d->otherLunarColor.name());
        dayItem->setSelectLunarColor(d->selectLunarColor.name());
        dayItem->setHoverLunarColor(d->hoverLunarColor.name());
        dayItem->setCurrentBgColor(d->currentBgColor.name());
        dayItem->setOtherBgColor(d->otherBgColor.name());
        dayItem->setSelectBgColor(d->selectBgColor.name());
        dayItem->setHoverBgColor(d->hoverBgColor.name());
    }

    this->setStyleSheet(qss.join(""));
}

// 初始化日期面板
void KMiniCalendarWidget::initDate()
{
    Q_D(KMiniCalendarWidget);
    int year = d->date.year();
    int month = d->date.month();

    // 设置为今天,设置变量防止重复触发
    d->btnClick = true;
    d->labelYearMonth->setText(QString("%1年%2月").arg(year).arg(month));
    d->btnClick = false;

    // 首先判断当前月的第一天是星期几
    int week = KLunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, false);
    // 当前月天数
    int countDay = KLunarCalendarInfo::Instance()->getMonthDays(year, month);
    // 上月天数
    int countDayPre = KLunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);

    // 如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;

    if (0 == week) {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    } else {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    // 纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;

    if (tempMonth < 1) {
        tempYear--;
        tempMonth = 12;
    }

    // 显示上月天数
    for (int i = startPre; i < endPre; i++) {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = KLunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        d->dayItems.at(index)->setDate(date, lunar, KMiniCalendarItem::DayType_MonthPre);
    }

    // 纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;

    if (tempMonth > 12) {
        tempYear++;
        tempMonth = 1;
    }

    // 显示下月天数
    for (int i = startNext; i < endNext; i++) {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = KLunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        d->dayItems.at(index)->setDate(date, lunar, KMiniCalendarItem::DayType_MonthNext);
    }

    // 重新置为当前年月
    tempYear = year;
    tempMonth = month;

    // 显示当前月
    for (int i = week; i < (countDay + week); i++) {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = KLunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);

        if (0 == (i % 7) || 6 == (i % 7)) {
            d->dayItems.at(index)->setDate(date, lunar, KMiniCalendarItem::DayType_WeekEnd);
        } else {
            d->dayItems.at(index)->setDate(date, lunar, KMiniCalendarItem::DayType_MonthCurrent);
        }
    }

    dayChanged(d->date);
}

void KMiniCalendarWidget::slotClicked(const QDate &date, const KMiniCalendarItem::DayType &dayType)
{
    Q_D(KMiniCalendarWidget);
    d->date = date;

    if (KMiniCalendarItem::DayType_MonthPre == dayType) {
        d->date = date.addMonths(1);
        showPreviousMonth();
    } else if (KMiniCalendarItem::DayType_MonthNext == dayType) {
        d->date = date.addMonths(-1);
        showNextMonth();
    } else {
        dayChanged(d->date);
    }

    this->close();
}

void KMiniCalendarWidget::dayChanged(const QDate &date)
{
    Q_D(KMiniCalendarWidget);
    // 计算星期几,当前天对应标签索引=日期+星期几-1
    int year = date.year();
    int month = date.month();
    int day = date.day();
    int week = KLunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, false);
    // qDebug() << QString("%1-%2-%3").arg(year).arg(month).arg(day);

    // 选中当前日期,其他日期恢复,这里还有优化空间,比方说类似单选框机制
    for (int i = 0; i < 42; i++) {
        // 当月第一天是星期天要另外计算
        int index = day + week - 1;

        if (week == 0) {
            index = day + 6;
        }

        d->dayItems.at(i)->setSelect(i == index);
    }

    // 发送日期单击信号
    emit clicked(date);
    // 发送日期更新信号
    emit selectionChanged();
}

void KMiniCalendarWidget::dateChanged(int year, int month, int day)
{
    Q_D(KMiniCalendarWidget);
    // 如果原有天大于28则设置为1,防止出错
    d->date.setDate(year, month, day > 28 ? 1 : day);
    initDate();
}

KMiniCalendarWidget::CalendarStyle KMiniCalendarWidget::getCalendarStyle() const
{
    Q_D(const KMiniCalendarWidget);
    return d->calendarStyle;
}

KMiniCalendarWidget::WeekNameFormat KMiniCalendarWidget::getWeekNameFormat() const
{
    Q_D(const KMiniCalendarWidget);
    return d->weekNameFormat;
}

QDate KMiniCalendarWidget::getDate() const
{
    Q_D(const KMiniCalendarWidget);
    return d->date;
}

QColor KMiniCalendarWidget::getWeekTextColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->weekTextColor;
}

QColor KMiniCalendarWidget::getWeekBgColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->weekBgColor;
}

bool KMiniCalendarWidget::getShowLunar() const
{
    Q_D(const KMiniCalendarWidget);
    return d->showLunar;
}

bool KMiniCalendarWidget::getEnableBorder() const
{
    Q_D(const KMiniCalendarWidget);
    return d->enableBorder;
}

KMiniCalendarWidget::SelectType KMiniCalendarWidget::getSelectType() const
{
    Q_D(const KMiniCalendarWidget);
    return d->selectType;
}

QColor KMiniCalendarWidget::getBorderColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->borderColor;
}

QColor KMiniCalendarWidget::getWeekColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->weekColor;
}

QColor KMiniCalendarWidget::getLunarColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->lunarColor;
}

QColor KMiniCalendarWidget::getCurrentTextColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->currentTextColor;
}

QColor KMiniCalendarWidget::getOtherTextColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->otherTextColor;
}

QColor KMiniCalendarWidget::getSelectTextColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->selectTextColor;
}

QColor KMiniCalendarWidget::getHoverTextColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->hoverTextColor;
}

QColor KMiniCalendarWidget::getCurrentLunarColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->currentLunarColor;
}

QColor KMiniCalendarWidget::getOtherLunarColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->otherLunarColor;
}

QColor KMiniCalendarWidget::getSelectLunarColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->selectLunarColor;
}

QColor KMiniCalendarWidget::getHoverLunarColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->hoverLunarColor;
}

QColor KMiniCalendarWidget::getCurrentBgColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->currentBgColor;
}

QColor KMiniCalendarWidget::getOtherBgColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->otherBgColor;
}

QColor KMiniCalendarWidget::getSelectBgColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->selectBgColor;
}

QColor KMiniCalendarWidget::getHoverBgColor() const
{
    Q_D(const KMiniCalendarWidget);
    return d->hoverBgColor;
}

QSize KMiniCalendarWidget::sizeHint() const
{
    Q_D(const KMiniCalendarWidget);
    return QSize(600, 500);
}

QSize KMiniCalendarWidget::minimumSizeHint() const
{
    Q_D(const KMiniCalendarWidget);
    return QSize(200, 150);
}

// 显示上月日期
void KMiniCalendarWidget::showPreviousMonth()
{
    Q_D(KMiniCalendarWidget);
    int year = d->date.year();
    int month = d->date.month();
    int day = d->date.day();

    if (year <= 1901 && month == 1) {
        return;
    }

    month--;

    if (month < 1) {
        month = 12;
        year--;
    }

    dateChanged(year, month, day);
}

// 显示下月日期
void KMiniCalendarWidget::showNextMonth()
{
    Q_D(KMiniCalendarWidget);
    int year = d->date.year();
    int month = d->date.month();
    int day = d->date.day();

    if (year >= 2099 && month == 12) {
        return;
    }

    month++;

    if (month > 12) {
        month = 1;
        year++;
    }

    dateChanged(year, month, day);
}

// 转到今天
void KMiniCalendarWidget::showToday()
{
    Q_D(KMiniCalendarWidget);
    d->date = QDate::currentDate();
    initDate();
    dayChanged(d->date);
}

void KMiniCalendarWidget::setCalendarStyle(const KMiniCalendarWidget::CalendarStyle &calendarStyle)
{
    Q_D(KMiniCalendarWidget);

    if (d->calendarStyle != calendarStyle) {
        d->calendarStyle = calendarStyle;
    }
}

void KMiniCalendarWidget::setWeekNameFormat(const KMiniCalendarWidget::WeekNameFormat &weekNameFormat)
{
    Q_D(KMiniCalendarWidget);

    if (d->weekNameFormat != weekNameFormat) {
        d->weekNameFormat = weekNameFormat;

        QStringList listWeek;

        if (weekNameFormat == WeekNameFormat_Short) {
            listWeek << "一"
                     << "二"
                     << "三"
                     << "四"
                     << "五"
                     << "六"
                     << "日";
        } else if (weekNameFormat == WeekNameFormat_Normal) {
            listWeek << "周一"
                     << "周二"
                     << "周三"
                     << "周四"
                     << "周五"
                     << "周六"
                     << "周日";
        } else if (weekNameFormat == WeekNameFormat_Long) {
            listWeek << "星期一"
                     << "星期二"
                     << "星期三"
                     << "星期四"
                     << "星期五"
                     << "星期六"
                     << "星期天";
        } else if (weekNameFormat == WeekNameFormat_En) {
            listWeek << "Mon"
                     << "Tue"
                     << "Wed"
                     << "Thu"
                     << "Fri"
                     << "Sat"
                     << "Sun";
        }

        // 逐个添加日期文字
        for (int i = 0; i < 7; i++) {
            d->labWeeks.at(i)->setText(listWeek.at(i));
        }
    }
}

void KMiniCalendarWidget::setDate(const QDate &date)
{
    Q_D(KMiniCalendarWidget);

    if (d->date != date) {
        d->date = date;
        initDate();
    }
}

void KMiniCalendarWidget::setWeekTextColor(const QColor &weekTextColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->weekTextColor != weekTextColor) {
        d->weekTextColor = weekTextColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setWeekBgColor(const QColor &weekBgColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->weekBgColor != weekBgColor) {
        d->weekBgColor = weekBgColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setShowLunar(bool showLunar)
{
    Q_D(KMiniCalendarWidget);

    if (d->showLunar != showLunar) {
        d->showLunar = showLunar;
        initStyle();
    }
}

void KMiniCalendarWidget::setEnableBorder(bool bVisible)
{
    Q_D(KMiniCalendarWidget);

    if (d->enableBorder != bVisible) {
        d->enableBorder = bVisible;
        update();
    }
}

void KMiniCalendarWidget::setSelectType(const KMiniCalendarWidget::SelectType &selectType)
{
    Q_D(KMiniCalendarWidget);

    if (d->selectType != selectType) {
        d->selectType = selectType;
        initStyle();
    }
}

void KMiniCalendarWidget::setBorderColor(const QColor &borderColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->borderColor != borderColor) {
        d->borderColor = borderColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setWeekColor(const QColor &weekColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->weekColor != weekColor) {
        d->weekColor = weekColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setLunarColor(const QColor &lunarColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->lunarColor != lunarColor) {
        d->lunarColor = lunarColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setCurrentTextColor(const QColor &currentTextColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->currentTextColor != currentTextColor) {
        d->currentTextColor = currentTextColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setOtherTextColor(const QColor &otherTextColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->otherTextColor != otherTextColor) {
        d->otherTextColor = otherTextColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setSelectTextColor(const QColor &selectTextColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->selectTextColor != selectTextColor) {
        d->selectTextColor = selectTextColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setHoverTextColor(const QColor &hoverTextColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->hoverTextColor != hoverTextColor) {
        d->hoverTextColor = hoverTextColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setCurrentLunarColor(const QColor &currentLunarColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->currentLunarColor != currentLunarColor) {
        d->currentLunarColor = currentLunarColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setOtherLunarColor(const QColor &otherLunarColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->otherLunarColor != otherLunarColor) {
        d->otherLunarColor = otherLunarColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setSelectLunarColor(const QColor &selectLunarColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->selectLunarColor != selectLunarColor) {
        d->selectLunarColor = selectLunarColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setHoverLunarColor(const QColor &hoverLunarColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->hoverLunarColor != hoverLunarColor) {
        d->hoverLunarColor = hoverLunarColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setCurrentBgColor(const QColor &currentBgColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->currentBgColor != currentBgColor) {
        d->currentBgColor = currentBgColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setOtherBgColor(const QColor &otherBgColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->otherBgColor != otherBgColor) {
        d->otherBgColor = otherBgColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setSelectBgColor(const QColor &selectBgColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->selectBgColor != selectBgColor) {
        d->selectBgColor = selectBgColor;
        initStyle();
    }
}

void KMiniCalendarWidget::setHoverBgColor(const QColor &hoverBgColor)
{
    Q_D(KMiniCalendarWidget);

    if (d->hoverBgColor != hoverBgColor) {
        d->hoverBgColor = hoverBgColor;
        initStyle();
    }
}

void KMiniCalendarWidget::wheelEvent(QWheelEvent *event)
{
    Q_D(KMiniCalendarWidget);

    if (event->angleDelta().ry() > 100) {
        showPreviousMonth();
    } else if (event->angleDelta().ry() < -100) {
        showNextMonth();
    }
}

void KMiniCalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KMiniCalendarWidget);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 开启抗锯齿

    // 圆角矩形的尺寸和圆角大小
    QRect rect(0, 0, width(), height());
    rect.adjust(8,8,-8,-8);
    int radius = ThemeController::getRadiusFromDT("kradius-window");
    if (radius == -1)
        radius = 12;

    // 使用QPainterPath来绘制圆角矩形
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);

    // 设置画刷
    painter.setBrush(QBrush(d->weekBgColor)); // 设置背景色
    // 设置画笔颜色为灰色
    QPen pen(QColor(233, 233, 233), 1); // 创建一个灰色、宽度为1的画笔
    painter.setPen(pen); // 将画笔应用于QPainter

    // 绘制圆角矩形（这将包括边框和背景）
    painter.drawPath(path);

    // 调用基类的paintEvent来绘制子控件（如果有的话）
    QWidget::paintEvent(event);
}
}

#include "kminicalendarwidget.moc"
#include "moc_kminicalendarwidget.cpp"
