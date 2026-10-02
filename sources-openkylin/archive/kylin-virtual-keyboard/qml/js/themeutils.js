/*
* Copyright 2025 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

.pragma library

/**
 * 计算默认主题的混合色
 * 高亮、关闭对应的颜色
 * @param {color} baseColor - 基础颜色，例如 #F53F3F 或 #3790FA
 * @param {real} overlayAlpha - 叠加的黑色透明度，例如 0.05 (5%) 或 0.2 (20%)
 * @returns {color} 计算后的混合颜色
 */
function calculateMixedColor(baseColor, overlayAlpha) {
    var bgR = baseColor.r
    var bgG = baseColor.g
    var bgB = baseColor.b

    var finalR = bgR * (1.0 - overlayAlpha)
    var finalG = bgG * (1.0 - overlayAlpha)
    var finalB = bgB * (1.0 - overlayAlpha)

    return Qt.rgba(finalR, finalG, finalB, 1.0)
}

/**
 * 从 GradientColor 对象获取颜色值并应用主题透明度
 * 用于处理 ukuiquick 的 DtTheme 中的渐变颜色
 * 参考 DtThemeBackground 的实现方式
 * 
 * @param {GradientColor} gradientColor - ukuiquick 的 GradientColor 对象
 * @param {real} transparency - 主题透明度，默认 1.0
 * @returns {color} 计算后的颜色
 */
function getColorFromGradientByTransparency(gradientColor, transparency) {
    if (!gradientColor) {
        return Qt.rgba(0, 0, 0, 0)
    }

    if (gradientColor.colorType === 0) {
        return gradientColor.setAlphaF(gradientColor.pureColor, transparency);
    } else {
        var gradientStart = gradientColor.gradientStart;
        var gradientBackground = gradientColor.gradientBackground;
        return gradientColor.mixBackGroundColor(gradientStart, gradientBackground, transparency);
    }
}

function getColorFromGradient(gradientColor) {
    if (!gradientColor) {
        return Qt.rgba(0, 0, 0, 0)
    }

    if (gradientColor.colorType === 0) {
        return gradientColor.pureColor
    }

    if (gradientColor.colorType === 1) {
        var gradientStart = gradientColor.gradientStart
        var gradientBackground = gradientColor.gradientBackground
        return gradientColor.mixBackGroundColor(gradientBackground, gradientStart, 1)
    }
}
