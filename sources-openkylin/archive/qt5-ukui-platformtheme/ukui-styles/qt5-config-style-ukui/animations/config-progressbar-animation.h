/*
 * Qt5-UKUI's Library
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef PROGRESSBARANIMATOR_H
#define PROGRESSBARANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QProgressBar>


namespace UKUIConfigStyleSpace {

enum class ProgressBarStatus {
    NORMAL = 0,//进度条正常状态
    INDETERMINATE = 1 //进度条不确定状态
};

class ConfigProgressBarAnimation : public QVariantAnimation
{
    Q_OBJECT
public:
    // ConfigProgressBarAnimation(QObject *parent);
    ConfigProgressBarAnimation(QObject *parent, ProgressBarStatus curStatus);
    ~ConfigProgressBarAnimation();

    QWidget *target();
private:
    // void init();
    void init(ProgressBarStatus curStatus);
};
}
#endif // PROGRESSBARANIMATOR_H
