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
#ifndef KBASEFRAME_H
#define KBASEFRAME_H

#include <QFrame>
#include <QHBoxLayout>

#include "klabel.h"

#define RADIUS_VALUE 6 /**< 圆角数值 */
#define PC_HEIGHT 60 /**< pc模式下高度 */
#define TABLET_HEIGHT 64 /**< 平板模式下高度 */
#define SETTING_LABEL_WIDTH 214 /**< label 宽度*/

namespace kdk
{

/**
 * @defgroup ContainerModule
 * @brief 支持圆角四种类型的容器
 * @since 3.0.1.0
 * {
 */

class KBaseFramePrivate;

class KBaseFrame : public QFrame
{
    Q_OBJECT
public:
    enum BorderRadiusStyle {
        Top, /**< 顶部有圆角 */
        None, /**< 没有圆角 */
        Bottom, /**< 底部有圆角 */
        Around /**< 四周都有圆角 */
    };

    /**
     * @brief 构造函数
     *
     * @param parent 父对象
     * @param style 圆角风格枚举值
     * @param heightAdaptive 是否自动调整高度，默认PC下60，平板模式64
     */
    explicit KBaseFrame(QWidget *parent = nullptr, BorderRadiusStyle style = Around, bool heightAdaptive = false);
    ~KBaseFrame();

    /**
     * @brief 设置圆角风格
     *
     * @param style 圆角风格枚举值
     */
    void setRadiusType(BorderRadiusStyle style);

    /**
     * @brief 获取平板模式
     *
     * @return bool 是否是平板模式
     */
    bool getTabletMode();

    /**
     * @brief 获取是否是一个容器frame，此时settinggroup需要设置的是其子frame的圆角
     *
     * @return bool 是/不是
     */
    bool isContainer();

    /**
     * @brief 设置该frame是否是一个容器frame,如果是同时setFrameShape(NoFrame)；
     *
     * @param b 是/不是
     */
    void setContainer(bool b = false);

Q_SIGNALS:
    void tabletModeChanged(bool b);

protected:
    void paintEvent(QPaintEvent *e);

private:
    Q_DECLARE_PRIVATE(KBaseFrame)
    KBaseFramePrivate *const d_ptr;
};

}

#endif // KBASEFRAME_H
