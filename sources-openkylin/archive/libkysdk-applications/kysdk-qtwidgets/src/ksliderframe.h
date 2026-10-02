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
#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include "kbaseframe.h"
#include "klabel.h"
#include "kslider.h"

/**
 * @defgroup ContainerModules
 * @brief 创建一个带 KSlider 的 Frame
 * @since 3.0.1.0
 * {
 */

namespace kdk
{

class KSliderFramePrivate;

class KSliderFrame : public KBaseFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 KSlider 的 Frame
     *
     * @param title:    text of m_settingName
     * @param hasNode:  是否带节点
     * @param style:   设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit KSliderFrame(QString title, bool hasNode, QWidget *parent = nullptr, BorderRadiusStyle style = BorderRadiusStyle::None, Qt::Orientation orientation = Qt::Horizontal);
    ~KSliderFrame();

    /**
     * @brief 获取 m_slider
     * @return KSlider
     */
    KSlider *slider();

    /**
     * @brief 获取滑动条左侧label
     * @return QLabel
     */
    QLabel *leftLabel();

    /**
     * @brief 获取滑动条右侧label
     * @return QLabel
     */
    QLabel *rightLabel();

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
    Q_DECLARE_PRIVATE(KSliderFrame)
    KSliderFramePrivate *const d_ptr;
};

}
#endif // SLIDERWIDGET_H
