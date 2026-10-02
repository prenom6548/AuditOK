/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_H
#define COMMON_H


#define DEFAULT_MAX_WIDTH   4096
#define DEFAULT_MAX_HEIGHT  4096
#define DEFAULT_SLIDER_WIDTH_OR_HEIGHT 0//46
#define DEFAULT_TITLEBAR_HEIGHT 40
#define TOOLBAR_ALIGN_OFFSET 5
#define DEFAULT_FLOAT_EXPAND_WIDTH 70
#define DEFAULT_FLOAT_SHRINK_WIDTH 18
#define ALIGN_SETTTING_OFFSET 10

#define BUTTON_SIZE 30
#define BUTTON_SPACE 4
#define MOUSE_MARGINS 5


typedef enum WalkDirectionType {
    WALK_DIRECTION_CENTER = 0,
    WALK_DIRECTION_LEFT = 1,
    WALK_DIRECTION_DOWN = 2,
    WALK_DIRECTION_UP = 3,
    WALK_DIRECTION_RIGHT = 4,
    WALK_DIRECTION_LEFT_UP = 5,
    WALK_DIRECTION_LEFT_DOWN = 6,
    WALK_DIRECTION_RIGHT_UP = 7,
    WALK_DIRECTION_RIGHT_DOWN = 8,
} WalkDirectionType;




#endif // COMMON_H
