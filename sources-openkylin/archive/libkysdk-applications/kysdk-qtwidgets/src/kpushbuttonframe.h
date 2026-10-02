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
#ifndef PUSHBUTTONWIDGET_H
#define PUSHBUTTONWIDGET_H

#include "kbaseframe.h"
#include "klabel.h"
#include "kpushbutton.h"

/**
 * @brief 创建一个带 QPushButton 的 Frame
 *
 */

namespace kdk
{

/**
 * @defgroup ContainerModules
 * @brief 带QpushButton的frame
 * @since 3.0.1.0
 * {
 */

class KPushButtonFramePrivate;

class KPushButtonFrame : public KBaseFrame
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个带 QPushButton 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit KPushButtonFrame(const QString title, QWidget *parent = nullptr, BorderRadiusStyle style = BorderRadiusStyle::None);
    /**
     * @brief
     *
     */
    ~KPushButtonFrame();

    /**
     * @brief 获取 m_pushButton
     *
     * @return KPushButton:  m_pushButton
     */
    KPushButton *pushButton();

    /**
     * @brief 获取label
     *
     * @return KLabel
     */
    KLabel *titleLabel();

    /**
     * @brief 获取layout
     *
     * @return QHBoxLayout
     */
    QHBoxLayout *mainHLayout();

private:
    Q_DECLARE_PRIVATE(KPushButtonFrame)
    KPushButtonFramePrivate *const d_ptr;
};

}

#endif // PUSHBUTTONWIDGET_H
