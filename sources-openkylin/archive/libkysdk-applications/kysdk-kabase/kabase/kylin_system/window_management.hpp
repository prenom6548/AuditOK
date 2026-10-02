/*
 * libkysdk-base's Library
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef KYLIN_SYSTEM_WINDOW_MANAGEMENT_H_
#define KYLIN_SYSTEM_WINDOW_MANAGEMENT_H_

/**
 * @brief kabase模块应用通用函数
 * @since 1.0.0
 */

namespace kdk
{
namespace kabase
{

class WindowManagement
{
public:
    WindowManagement();
    ~WindowManagement();

    /**
     * @brief 设置窗口主题提示
     *
     * @param winId 窗口id
     *
     * @return 无
     */
    static bool setWindowMotifHint(int winId);

    /**
     * @brief 设置缩放属性
     *
     * @param 无
     *
     * @retval true 成功
     * @retval false 失败
     */
    static bool setScalingProperties(void);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
