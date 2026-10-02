/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

// See sdf.glsl for the SDF related functions.

// This is a version of shadowedrectangle.frag meant for very low power hardware
// (PinePhone). It does not render a shadow and does not do alpha blending.

uniform lowp float opacity;
uniform lowp float size;
uniform lowp vec4 radius;
uniform lowp vec4 color;
uniform lowp vec4 shadowColor;
uniform lowp vec2 offset;
uniform lowp vec2 aspect;
// 添加渐变相关uniform变量
uniform bool pureColor;          //是否纯色
uniform lowp vec4 startColor;    // 渐变起始颜色
uniform lowp vec4 endColor;      // 渐变结束颜色
uniform lowp float angle;        // 渐变角度（弧度）

#ifdef CORE_PROFILE
in lowp vec2 uv;
out lowp vec4 out_color;
#else
varying lowp vec2 uv;
#define out_color gl_FragColor
#endif

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the main rectangle distance field.
    lowp float rect = sdf_rounded_rectangle(uv, aspect, radius);

    // Render it.
    if (pureColor) {
        col = sdf_render(rect, col, color);
    } else {
        // 添加渐变渲染逻辑
        // 计算渐变方向
        vec2 dir = normalize(vec2(cos(angle), sin(angle)) / aspect);
        // 转换到矩形本地坐标系
        vec2 rectUV = uv / aspect;
        // 计算渐变插值因子
        float t = clamp((dot(rectUV, dir) + 1.0) / 2.0, 0.0, 1.0);
        // 混合颜色
        vec4 gradientColor = mix(startColor, endColor, t);

        col = sdf_render(rect, col, gradientColor);
    }

    out_color = col * opacity;
}
