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

#ifndef KMINICALENDARWIDGET_H
#define KMINICALENDARWIDGET_H

#include "gui_g.h"
#include "klunarcalendarinfo.h"
#include "kminicalendaritem.h"
#include <QComboBox>
#include <QDate>
#include <QLabel>
#include <QToolButton>
#include <QWheelEvent>
#include <QWidget>

namespace kdk
{
class KMiniCalendarWidgetPrivate;

/**
 *  @defgroup KMiniCalendarWidget
 *  @since 2.5.2.0
 */

class GUI_EXPORT KMiniCalendarWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(CalendarStyle)
    Q_ENUMS(WeekNameFormat)
    Q_ENUMS(SelectType)

    Q_PROPERTY(CalendarStyle calendarStyle READ getCalendarStyle WRITE setCalendarStyle)
    Q_PROPERTY(WeekNameFormat weekNameFormat READ getWeekNameFormat WRITE setWeekNameFormat)
    Q_PROPERTY(QDate date READ getDate WRITE setDate)

    Q_PROPERTY(QColor weekTextColor READ getWeekTextColor WRITE setWeekTextColor)
    Q_PROPERTY(QColor weekBgColor READ getWeekBgColor WRITE setWeekBgColor)

    Q_PROPERTY(bool showLunar READ getShowLunar WRITE setShowLunar)
    Q_PROPERTY(bool enableBorder READ getEnableBorder WRITE setEnableBorder)
    Q_PROPERTY(SelectType selectType READ getSelectType WRITE setSelectType)

    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor weekColor READ getWeekColor WRITE setWeekColor)
    Q_PROPERTY(QColor lunarColor READ getLunarColor WRITE setLunarColor)

    Q_PROPERTY(QColor currentTextColor READ getCurrentTextColor WRITE setCurrentTextColor)
    Q_PROPERTY(QColor otherTextColor READ getOtherTextColor WRITE setOtherTextColor)
    Q_PROPERTY(QColor selectTextColor READ getSelectTextColor WRITE setSelectTextColor)
    Q_PROPERTY(QColor hoverTextColor READ getHoverTextColor WRITE setHoverTextColor)

    Q_PROPERTY(QColor currentLunarColor READ getCurrentLunarColor WRITE setCurrentLunarColor)
    Q_PROPERTY(QColor otherLunarColor READ getOtherLunarColor WRITE setOtherLunarColor)
    Q_PROPERTY(QColor selectLunarColor READ getSelectLunarColor WRITE setSelectLunarColor)
    Q_PROPERTY(QColor hoverLunarColor READ getHoverLunarColor WRITE setHoverLunarColor)

    Q_PROPERTY(QColor currentBgColor READ getCurrentBgColor WRITE setCurrentBgColor)
    Q_PROPERTY(QColor otherBgColor READ getOtherBgColor WRITE setOtherBgColor)
    Q_PROPERTY(QColor selectBgColor READ getSelectBgColor WRITE setSelectBgColor)
    Q_PROPERTY(QColor hoverBgColor READ getHoverBgColor WRITE setHoverBgColor)

public:
    enum CalendarStyle {
        CalendarStyle_Red = 0
    };

    enum WeekNameFormat {
        WeekNameFormat_Short = 0, // 短名称模式
        WeekNameFormat_Normal = 1, // 普通名称模式
        WeekNameFormat_Long = 2, // 长名称模式
        WeekNameFormat_En = 3 // 英文模式
    };

    enum SelectType {
        SelectType_Rect = 0, // 矩形背景
        SelectType_Circle = 1, // 圆形背景
        SelectType_Triangle = 2, // 带三角标
        SelectType_Image = 3, // 图片背景
        SelectType_CR = 4           //圆角矩形
    };

    explicit KMiniCalendarWidget(QWidget *parent = 0);
    ~KMiniCalendarWidget();

public:
    /**
     * @brief 获取整体样式
     * @return
     */
    CalendarStyle getCalendarStyle() const;

    /**
     * @brief 获取星期名称格式
     * @return
     */
    WeekNameFormat getWeekNameFormat() const;

    /**
     * @brief 获取当前日期
     * @return
     */
    QDate getDate() const;

    /**
     * @brief 获取星期文本颜色
     * @return
     */
    QColor getWeekTextColor() const;

    /**
     * @brief 获取周末名称背景色
     * @return
     */
    QColor getWeekBgColor() const;

    /**
     * @brief 获取农历显示
     * @return
     */
    bool getShowLunar() const;

    /**
     * @brief 获取启用边框
     * @return
     */
    bool getEnableBorder() const;

    /**
     * @brief 获取选择类型
     * @return
     */
    SelectType getSelectType() const;

    /**
     * @brief 获取边框颜色
     * @return
     */
    QColor getBorderColor() const;

    /**
     * @brief 获取周末颜色
     * @return
     */
    QColor getWeekColor() const;

    /**
     * @brief 获取农历节日颜色
     * @return
     */
    QColor getLunarColor() const;

    /**
     * @brief 获取当前月文本颜色
     * @return
     */
    QColor getCurrentTextColor() const;

    /**
     * @brief 获取其他月文本颜色
     * @return
     */
    QColor getOtherTextColor() const;

    /**
     * @brief 获取选中日期文本颜色
     * @return
     */
    QColor getSelectTextColor() const;

    /**
     * @brief 获取悬浮日期文本颜色
     * @return
     */
    QColor getHoverTextColor() const;

    /**
     * @brief 获取当前月农历文字颜色
     * @return
     */
    QColor getCurrentLunarColor() const;

    /**
     * @brief 获取其他月农历文本颜色
     * @return
     */
    QColor getOtherLunarColor() const;

    /**
     * @brief 获取选中日期农历颜文本色
     * @return
     */
    QColor getSelectLunarColor() const;

    /**
     * @brief 获取悬浮日期农历文本颜色
     * @return
     */
    QColor getHoverLunarColor() const;

    /**
     * @brief 获取当前月背景色
     * @return
     */
    QColor getCurrentBgColor() const;

    /**
     * @brief 获取其他月背景色
     * @return
     */
    QColor getOtherBgColor() const;

    /**
     * @brief 获取选择日期背景色
     * @return
     */
    QColor getSelectBgColor() const;

    /**
     * @brief 获取悬浮日期背景色
     * @return
     */
    QColor getHoverBgColor() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public Q_SLOTS:
    // 上一月,下一月
    void showPreviousMonth();
    void showNextMonth();
    // 上一年,下一年
    void showPreviousYear();
    void showNextYear();

    // 转到今天
    void showToday();

    // 设置整体样式
    void setCalendarStyle(const CalendarStyle &calendarStyle);
    // 设置星期名称格式
    void setWeekNameFormat(const WeekNameFormat &weekNameFormat);

    // 设置日期
    void setDate(const QDate &date);
    // 设置顶部星期名称文字颜色+背景色
    void setWeekTextColor(const QColor &weekTextColor);
    void setWeekBgColor(const QColor &weekBgColor);

    // 设置是否显示农历信息
    void setShowLunar(bool showLunar);
    // 是否显示边框
    void setEnableBorder(bool bVisible);
    // 设置选中背景样式
    void setSelectType(const SelectType &selectType);

    // 设置边框颜色
    void setBorderColor(const QColor &borderColor);
    // 设置周末颜色
    void setWeekColor(const QColor &weekColor);
    // 设置农历节日颜色
    void setLunarColor(const QColor &lunarColor);

    // 设置当前月文字颜色
    void setCurrentTextColor(const QColor &currentTextColor);
    // 设置其他月文字颜色
    void setOtherTextColor(const QColor &otherTextColor);
    // 设置选中日期文字颜色
    void setSelectTextColor(const QColor &selectTextColor);
    // 设置悬停日期文字颜色
    void setHoverTextColor(const QColor &hoverTextColor);

    // 设置当前月农历文字颜色
    void setCurrentLunarColor(const QColor &currentLunarColor);
    // 设置其他月农历文字颜色
    void setOtherLunarColor(const QColor &otherLunarColor);
    // 设置选中日期农历文字颜色
    void setSelectLunarColor(const QColor &selectLunarColor);
    // 设置悬停日期农历文字颜色
    void setHoverLunarColor(const QColor &hoverLunarColor);

    // 设置当前月背景颜色
    void setCurrentBgColor(const QColor &currentBgColor);
    // 设置其他月背景颜色
    void setOtherBgColor(const QColor &otherBgColor);
    // 设置选中日期背景颜色
    void setSelectBgColor(const QColor &selectBgColor);
    // 设置悬停日期背景颜色
    void setHoverBgColor(const QColor &hoverBgColor);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void clicked(const QDate &date);
    void selectionChanged();

private slots:
    void initWidget();
    void initStyle();
    void initDate();
    void slotClicked(const QDate &date, const KMiniCalendarItem::DayType &dayType);
    void dayChanged(const QDate &date);
    void dateChanged(int year, int month, int day);

private:
    Q_DECLARE_PRIVATE(KMiniCalendarWidget)
    KMiniCalendarWidgetPrivate *const d_ptr;
};
}

#endif // KMINICALENDARWIDGET_H
