/*
 *
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
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include "kbaseframe.h"

#include "kswitchbutton.h"

/**
 * @defgroup ContainerModules
 * @brief 创建一个带 KSwitchButton 的 Frame
 * @since 3.0.1.0
 * {
 */
namespace kdk
{

class KSwitchButtonFramePrivate;

class KSwitchButtonFrame : public KBaseFrame
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
    explicit KSwitchButtonFrame(const QString title, QWidget *parent = nullptr, BorderRadiusStyle style = BorderRadiusStyle::None);
    ~KSwitchButtonFrame();

    /**
     * @brief 设置提示文案
     * @param hintText
     */
    void setHintText(QString &hintText);

    /**
     * @brief 获取 switchbutton
     * @return KSlider: m_slider
     */
    KSwitchButton *switchButton();

    /**
     * @brief 获取label
     *
     * @return QLabel
     */
    KLabel *titleLabel();

    /**
     * @brief 获取layout
     *
     * @return QHBoxLayout
     */
    QHBoxLayout *mainHLayout();

private:
    Q_DECLARE_PRIVATE(KSwitchButtonFrame)
    KSwitchButtonFramePrivate *const d_ptr;
};
}

#endif // SWITCHWIDGET_H
