/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

layout(std140, binding = 0) uniform buf {
    //64字节 - 4x4矩阵
    highp mat4 matrix; // offset 0
    
    //16字节 - vec4
    lowp vec4 radius;    // offset 64
    lowp vec4 color;     // offset 80
    lowp vec4 shadowColor; // offset 96
    lowp vec4 startColor;  // offset 112
    lowp vec4 endColor;    // offset 128
    lowp vec4 borderColor; // offset 144
    
    //8字节 - vec2
    lowp vec2 aspect;   // offset 160
    lowp vec2 offset;   // offset 168
    
    //4字节 - float和bool
    lowp float opacity;   // offset 176
    lowp float size;      // offset 180
    lowp float angle;     // offset 184
    lowp float borderWidth; // offset 188
    bool pureColor;       // offset 192
} ubuf; // size 196