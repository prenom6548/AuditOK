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
#ifndef KCOMBOBOXFRAME_H
#define KCOMBOBOXFRAME_H

#include "klabel.h"
#include <QComboBox>
#include <QObject>

#include "kbaseframe.h"

namespace kdk
{

/**
 * @defgroup ContainerModules
 * @brief 存在combox的frame
 * @since 3.0.1.0
 * {
 */
class KComboBoxFramePrivate;

class KComboBoxFrame : public KBaseFrame
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个带 QComboBox 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit KComboBoxFrame(const QString title, QWidget *parent = nullptr, BorderRadiusStyle style = BorderRadiusStyle::None);
    ~KComboBoxFrame();

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

    /**
     * @brief 获取下拉框控件
     *
     * @return QComboBox
     */
    QComboBox *comboBox();

private:
    Q_DECLARE_PRIVATE(KComboBoxFrame)
    KComboBoxFramePrivate *const d_ptr;
};

}

#endif // KCOMBOBOXFRAME_H
