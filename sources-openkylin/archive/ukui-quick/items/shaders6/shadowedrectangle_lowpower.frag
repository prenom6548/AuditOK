/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

// See sdf.glsl for the SDF related functions.
#extension GL_GOOGLE_include_directive: enable
#include "sdf_lowpower.glsl"

// This is a version of shadowedrectangle.frag meant for very low power hardware
// (PinePhone). It does not render a shadow and does not do alpha blending.

#include "uniforms.glsl"

layout(location = 0) in lowp vec2 uv;
layout(location = 0) out lowp vec4 out_color;

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the main rectangle distance field.
    lowp float rect = sdf_rounded_rectangle(uv, ubuf.aspect, ubuf.radius);

    // Render it with gradient support
    if (ubuf.pureColor) {
        col = sdf_render(rect, col, ubuf.color);
    } else {
        // 计算渐变方向
        vec2 dir = normalize(vec2(cos(ubuf.angle), sin(ubuf.angle)) / ubuf.aspect);
        // 转换到矩形本地坐标系
        vec2 rectUV = uv / ubuf.aspect;
        // 计算渐变插值因子
        float t = clamp((dot(rectUV, dir) + 1.0) / 2.0, 0.0, 1.0);
        // 混合颜色
        vec4 gradientColor = mix(ubuf.startColor, ubuf.endColor, t);

        col = sdf_render(rect, col, gradientColor);
    }

    out_color = col * ubuf.opacity;
}
