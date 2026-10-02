/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include "fixlabel.h"
#include "lightlabel.h"
#include "libukcc_global.h"
#include "ukccframe.h"

#include <kysdk/applications/kswitchbutton.h>
using namespace kdk;

/**
 * @brief 创建一个带 KSwitchButton 的 Frame
 *
 */
class LIBUKCC_EXPORT SwitchWidget : public UkccFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 KSwitchButton 的 Frame
     *
     * @param title: text of m_settingName
     * @param style: 设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit SwitchWidget(const QString title, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None, const QString hintText = QString());

    /**
     * @brief
     *
     */
    ~SwitchWidget();

    /**
     * @brief 设置提示文案是否可见
     * @param visible
     */
    void setHintTextVisible(bool visible);

    /**
     * @brief 设置是否可点击
     * @param checked
     */
    void setEnabled(const bool checked = true);

    /**
     * @brief
     *
     * @return bool
     */
    bool isEnabled() const;

    /**
     * @brief 设置开启状态
     * @param checked
     */
    void setChecked(const bool checked = true);

    /**
     * @brief 返回是否选中
     * @return
     */
    bool isChecked() const;

    /**
     * @brief 设置 m_settingName text
     * @param title
     */
    void setTitle(const QString &title);

    /**
     * @brief 获取 m_settingName text
     * @return
     */
    QString title() const;

    /**
     * @brief blockSignals of m_switchButton
     * @param b
     * @return bool
     */
    bool blockSignals(bool b);

    /**
     * @brief 在 mainLayout 中插入序号为 index 的 widget
     *
     * @param index:    插入序号
     * @param widget:   插入控件
     * @param stretch:  stretch factor
     * @param alignment:alignment
     */
    void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());


    /**
     * @brief 获取 m_slider
     * @return KSlider: m_slider
     */
    KSwitchButton *switchButton() const;


    /**
     * @brief 获取 m_settingName
     * @return FixLabel: m_settingName
     */
    FixLabel* nameLabel();


private:
    void init();

    QHBoxLayout *mainLayout;
    QVBoxLayout *m_vLayout;
    FixLabel *m_settingName;
    LightLabel *m_hintText;
    KSwitchButton *m_switchButton;

private Q_SLOTS:

Q_SIGNALS:
    /**
     * @brief m_switchButton 状态改变信号
     *
     * @param checked
     */
    void stateChanged(const bool checked);
};

#endif // SWITCHWIDGET_H
