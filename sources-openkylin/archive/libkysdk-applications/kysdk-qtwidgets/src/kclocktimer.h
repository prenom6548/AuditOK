/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KCLOCKTIMER_H
#define KCLOCKTIMER_H

#include "gui_g.h"
#include <QWidget>

namespace kdk
{

class KClockTimerPrivate;

/**
 * @defgroup timerModule
 * @since 2.5.2.0
 */
class GUI_EXPORT KClockTimer : public QWidget
{
    Q_OBJECT
public:
    KClockTimer(QWidget *parent = nullptr);

    /**
     * @brief 获取当前时间
     * @return
     */
    QString time();

public slots:
    /**
     * @brief 开始计时信号
     */
    void startStopwatch();

    /**
     * @brief 停止计时信号
     */
    void stopStopwatch();

    /**
     * @brief 复位信号
     */
    void resetStopwatch();

private:
    KClockTimerPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(KClockTimer)
};

}
#endif // KCLOCKTIMER_H
