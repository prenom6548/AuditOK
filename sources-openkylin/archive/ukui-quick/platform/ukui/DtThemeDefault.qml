/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQml 2.15
import QtQuick 2.15
import org.ukui.quick.platform 1.0

DtThemeDefinition {
    id: dtTheme
    //QPalette 颜色
    property color windowTextActive : kFontPrimary
    property color windowTextInactive : kFontSecondaryDisable
    property color windowTextDisable : kFontPrimaryDisable
    property color buttonActive : kGray3
    property color buttonInactive : kGray3
    property color buttonDisable : Qt.rgba(0, 0, 0, 0.45)
    property color lightActive : kGray0
    property color lightInactive : kGray0
    property color lightDisable : Qt.rgba(255/255, 255/255, 255/255, 0.45)
    property color midLightActive : kGray7
    property color midLightInactive : kGray7
    property color midLightDisable : Qt.rgba(204/255, 204/255, 204/255, 0.42)
    property color darkActive : kBlack
    property color darkInactive : kBlack
    property color darkDisable : kBlack
    property color midActive : kGray13
    property color midInactive : kGray13
    property color midDisable : Qt.rgba(102/255, 102/255, 102/255, 0.4)
    property color textActive : kFontPrimary
    property color textInactive : kFontPrimary
    property color textDisable : kFontPrimaryDisable
    property color brightTextActive : kFontStrong
    property color brightTextInactive : kFontStrong
    property color brightTextDisable : Qt.rgba(0, 0, 0, 0.45)
    property color buttonTextActive : kFontPrimary
    property color buttonTextInactive : kFontPrimary
    property color buttonTextDisable : kFontPrimaryDisable
    property color baseActive : kGray0
    property color baseInactive : kGray1
    property color baseDisable : kGray1
    property color windowActive : kGray2
    property color windowInactive : kGray3
    property color windowDisable : kGray3
    property color shadowActive : Qt.rgba(0, 0, 0, 0.25)
    property color shadowInactive : Qt.rgba(0, 0, 0, 0.18)
    property color shadowDisable : Qt.rgba(0, 0, 0, 0.11)
    property color highlightActive : kBrandNormal
    property color highlightInactive : kBrandNormal
    property color highlightDisable : kBrandDisable
    property color highlightedTextActive : kFontWhite
    property color highlightedTextInactive : kFontWhite
    property color highlightedTextDisable : kFontWhiteDisable
    property color linkActive : Qt.rgba(46/255, 95/255, 209/255, 1)
    property color linkInactive : Qt.rgba(46/255, 95/255, 209/255, 1)
    property color linkDisable : Qt.rgba(46/255, 95/255, 209/255, 0.4)
    property color linkVisitedActive : Qt.rgba(120/255, 48/255, 191/255, 1)
    property color linkVisitedInactive : Qt.rgba(120/255, 48/255, 191/255, 1)
    property color linkVisitedDisable : Qt.rgba(120/255, 48/255, 191/255, 0.4)
    property color alternateBaseActive : Qt.rgba(0, 0, 0, 0.03)
    property color alternateBaseInactive : Qt.rgba(0, 0, 0, 0.01)
    property color alternateBaseDisable : Qt.rgba(0, 0, 0, 0.01)
    property color noRoleActive : kGray1
    property color noRoleInactive : kGray1
    property color noRoleDisable : Qt.rgba(250/255, 250/255, 250/255, 0.42)
    property color toolTipBaseActive : kGray0
    property color toolTipBaseInactive : kGray0
    property color toolTipBaseDisable : Qt.rgba(255/255, 255/255, 255/255, 0.4)
    property color toolTipTextActive : kFontPrimary
    property color toolTipTextInactive : kFontPrimary
    property color toolTipTextDisable : kFontPrimaryDisable
    property color placeholderTextActive : kFontPlaceholderText
    property color placeholderTextInactive : kFontPlaceholderText
    property color placeholderTextDisable : kFontPlaceholderTextDisable

    //中性色
    property color kWhite : Qt.rgba(255/255, 255/255, 255/255, 1)
    property color kBlack : Qt.rgba(0/255, 0/255, 0/255, 1)
    property color kGray0 : Qt.rgba(255/255, 255/255, 255/255, 1)
    property color kGray1 : Qt.rgba(250/255, 250/255, 250/255, 1)
    property color kGray2 : Qt.rgba(245/255, 245/255, 245/255, 1)
    property color kGray3 : Qt.rgba(240/255, 240/255, 240/255, 1)
    property color kGray4 : Qt.rgba(235/255, 235/255, 235/255, 1)
    property color kGray5 : Qt.rgba(229/255, 229/255, 229/255, 1)
    property color kGray6 : Qt.rgba(224/255, 224/255, 224/255, 1)
    property color kGray7 : Qt.rgba(214/255, 214/255, 214/255, 1)
    property color kGray8 : Qt.rgba(199/255, 199/255, 199/255, 1)
    property color kGray9 : Qt.rgba(184/255, 184/255, 184/255, 1)
    property color kGray10 : Qt.rgba(163/255, 163/255, 163/255, 1)
    property color kGray11 : Qt.rgba(143/255, 143/255, 143/255, 1)
    property color kGray12 : Qt.rgba(122/255, 122/255, 122/255, 1)
    property color kGray13 : Qt.rgba(102/255, 102/255, 102/255, 1)
    property color kGray14 : Qt.rgba(82/255, 82/255, 82/255, 1)
    property color kGray15 : Qt.rgba(61/255, 61/255, 61/255, 1)
    property color kGray16 : Qt.rgba(41/255, 41/255, 41/255, 1)
    property color kGray17 : Qt.rgba(26/255, 26/255, 26/255, 1)
    property color kGrayAlpha0 : Qt.rgba(0, 0, 0, 0)
    property color kGrayAlpha1 : Qt.rgba(0, 0, 0, 0.04)
    property color kGrayAlpha2 : Qt.rgba(0, 0, 0, 0.06)
    property color kGrayAlpha3 : Qt.rgba(0, 0, 0, 0.8)
    property color kGrayAlpha4 : Qt.rgba(0, 0, 0, 0.12)
    property color kGrayAlpha5 : Qt.rgba(0, 0, 0, 0.16)
    property color kGrayAlpha6 : Qt.rgba(0, 0, 0, 0.24)
    property color kGrayAlpha7 : Qt.rgba(0, 0, 0, 0.3)
    property color kGrayAlpha8 : Qt.rgba(0, 0, 0, 0.38)
    property color kGrayAlpha9 : Qt.rgba(0, 0, 0, 0.42)
    property color kGrayAlpha10 : Qt.rgba(0, 0, 0, 0.55)
    property color kGrayAlpha11 : Qt.rgba(0, 0, 0, 0.9)
    property color kGrayAlpha12 : Qt.rgba(0, 0, 0, 0.02)
    property color kGrayAlpha13 : Qt.rgba(0, 0, 0, 0.75)

    //文本颜色
    property color kFontStrong : Qt.rgba(0, 0, 0, 1)
    property color kFontPrimary : Qt.rgba(0, 0, 0, 0.88)
    property color kFontPrimaryDisable : Qt.rgba(0, 0, 0, 0.28)
    property color kFontSecondary : Qt.rgba(0, 0, 0, 0.4)
    property color kFontSecondaryDisable : Qt.rgba(0, 0, 0, 0.15)
    property color kFontWhite : Qt.rgba(255/255, 255/255, 255/255, 0.98)
    property color kFontWhiteDisable : Qt.rgba(255/255, 255/255, 255/255, 0.68)
    property color kFontWhiteSecondary : Qt.rgba(255/255, 255/255, 255/255, 0.65)
    property color kFontWhiteSecondaryDisable : Qt.rgba(255/255, 255/255, 255/255, 0.24)

    //功能色
    property color kBrandNormal : Qt.rgba(54/255, 118/255, 245/255, 1)
    property var kBrandHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(54/255, 118/255, 245/255, 1)
    }
    property var kBrandClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(54/255, 118/255, 245/255, 1)
    }
    property color kBrandFocus : kGrayAlpha13
    property color kSuccessNormal : kGreen1
    property var kSuccessHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(24/255, 168/255, 13/255, 1)
    }
    property var kSuccessClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(24/255, 168/255, 13/255, 1)
    }
    property color kWarningNormal : kOrange1
    property var kWarningHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(255/255, 128/255, 31/255, 1)
    }
    property var kWarningClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(255/255, 128/255, 31/255, 1)
    }
    property color kErrorNormal : kRed1
    property var kErrorHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(245/255, 73/255, 73/255, 1)
    }
    property var kErrorClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(245/255, 73/255, 73/255, 1)
    }
    property color kBrand1 : Qt.rgba(55/255, 144/255, 250/255, 1)
    property color kBrand2 : Qt.rgba(137/255, 38/255, 235/255, 1)
    property color kBrand3 : Qt.rgba(228/255, 74/255, 232/255, 1)
    property color kBrand4 : Qt.rgba(242/255, 181/255, 39/255, 1)
    property color kBrand5 : Qt.rgba(20, 166/255, 33/255, 1)
    property color kBrand6 : Qt.rgba(240/255, 73/255, 67/255, 1)
    property color kBrand7 : Qt.rgba(250/255, 125/255, 15/255, 1)

    //其他颜色
    property color kContainHover : kGray2

    property color kContainClick : kGray4
    property color kContainGeneralNormal : kGray0
    property color kContainSecondaryNormal : kGray1
    property color kContainSecondaryAlphaNormal : kGrayAlpha12
    property color kComponentNormal : kGray3
    property color kComponentHover : kGray4
    property color kComponentClick : kGray6
    property color kComponentDisable : Qt.rgba(245/255, 245/255, 245/255, 0.45)
    property color kComponentAlphaNormal : kGrayAlpha1
    property color kComponentAlphaHover : kGrayAlpha3
    property color kComponentAlphaClick : kGrayAlpha4
    property color kComponentAlphaDisable : Qt.rgba(0, 0, 0, 0.02)
    property color kLineWindowActive : Qt.rgba(255/255, 255/255, 255/255, 0.7)
    property color kLineComponentNormal : kGrayAlpha2
    property color kLineComponentHover : kGrayAlpha3
    property color kLineComponentClick : kGrayAlpha4
    property color kLineComponentDisable : Qt.rgba(0, 0, 0, 0.03)
    property color kLineBrandNormal : Qt.rgba(54/255, 118/255, 245/255, 0.55)
    property var kLineBrandHover : {
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(54/255, 118/255, 245/255, 0.55)
    }
    property var kLineBrandClick : {
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(54/255, 118/255, 245/255, 0.55)
    }
    property color kLineBrandDisable : Qt.rgba(54/255, 118/255, 245/255, 0.23)
    property color kModalMask : KGrayAlpha6

    //2025/01/02根据主题补充token
    property color kSuccessDisable: Qt.rgba(24/255, 168/255, 13/255, 0.45)
    property color kErrorDisable: Qt.rgba(245/255, 73/255, 73/255, 0.45)
    property color kWarningDisable: Qt.rgba(255/255, 128/255, 31/255, 0.45)
    property color kLineNormal: Qt.rgba(0, 0, 0, 0.06)
    property color kContainGeneralAlphaNormal: kGrayAlpha0
    property color kLineWindowInactive: Qt.rgba(255/255, 255/255, 255/255, 0.4)
    property color kContainAlphaClick: kGrayAlpha3
    property color kContainAlphaHover: kGrayAlpha1
    property color kBrandDisable: Qt.rgba(54/255, 118/255, 245/255, 0.4)
    property color kLineDisable: Qt.rgba(0, 0, 0, 0.03)
    property color kDivider: kGrayAlpha3

    //2025/03/03根据5.0主题新增token,默认值为V11主题颜色
    property color kBrand8: Qt.rgba(54/255, 118/255, 245/255, 1)
    property color kBrand9: Qt.rgba(6/255, 192/255, 199/255, 1)
    property color kLineSelectboxNormal: Qt.rgba(0/255, 0/255, 0/255, 0.2)
    property color kLineSelectboxHover: Qt.rgba(0/255, 0/255, 0/255, 0.24)
    property color kLineSelectboxClick: Qt.rgba(0/255, 0/255, 0/255, 0.28)
    property color kLineSelectboxSelected: Qt.rgba(0/255, 0/255, 0/255, 0)
    property color kLineSelectboxDisable: Qt.rgba(0/255, 0/255, 0/255, 0.08)
    property color kFontWhitePlaceholderTextDisable: Qt.rgba(255/255, 255/255, 255/255, 0.13)
    property color kContainGeneralInactive: kGray1
    property color kFontPlaceholderTextDisable: Qt.rgba(0/255, 0/255, 0/255, 0.12)
    property color kFontWhitePlaceholderText: Qt.rgba(255/255, 255/255, 255/255, 0.35)
    property color kLineInputDisable: Qt.rgba(0/255, 0/255, 0/255, 0.04)
    property color kLineDock: Qt.rgba(255/255, 255/255, 255/255, 0.55)
    property color kLineMenu: Qt.rgba(255/255, 255/255, 255/255, 0.7)
    property color kLineTable: Qt.rgba(0/255, 0/255, 0/255, 0.12)
    property color kLineInputNormal: Qt.rgba(0/255, 0/255, 0/255, 0.08)
    property color kLineInputHover: Qt.rgba(0/255, 0/255, 0/255, 0.12)
    property color kLineInputClick: kBrand8
    property color kOrange1: Qt.rgba(255/255, 128/255, 31/255, 1)
    property color kGreen1: Qt.rgba(24/255, 168/255, 13/255, 1)
    property color kRed1: Qt.rgba(235/255, 70/255, 70/255, 1)
    property color kDividerWhite: Qt.rgba(255/255, 255/255, 255/255, 0.12)

    property color kLineSuccessNormal: Qt.rgba(24/255, 168/255, 13/255, 0.55)
    property var kLineSuccessHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(24/255, 168/255, 13/255, 0.55)
    }
    property var kLineSuccessClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(24/255, 168/255, 13/255, 0.55)
    }
    property color kLineSuccessDisable: Qt.rgba(24/255, 168/255, 13/255, 0.23)
    property color kLineErrorNormal: Qt.rgba(245/255, 73/255, 73/255, 0.55)
    property var kLineErrorHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(245/255, 73/255, 73/255, 0.55)
    }
    property var kLineErrorClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(245/255, 73/255, 73/255, 0.55)
    }
    property color kLineWarningNormal: Qt.rgba(255/255, 128/255, 31/255, 0.55)
    property var kLineWarningHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(255/255, 128/255, 31/255, 0.55)
    }
    property var kLineWarningClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(255/255, 128/255, 31/255, 0.55)
    }
    property color kLineWarningDisable: Qt.rgba(255/255, 128/255, 31/255, 0.23)

    property var kShadowPrimaryActive : ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 4
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.2)
        isMultiShadow: true
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 18
            blurRadius: 38
            spread: 0
            shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.3)
            isMultiShadow: false
        }
    }

    property var kShadowPrimaryInactive : ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 2
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.2)
        isMultiShadow: true
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 8
            blurRadius: 16
            spread: 0
            shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.18)
            isMultiShadow: false
        }
    }

    property var kShadowSecondaryActive : ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 2
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.1)
        isMultiShadow: true
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 12
            blurRadius: 26
            spread: 0
            shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.25)
            isMultiShadow: false
        }
    }

    property var kShadowSecondaryInactive : ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 2
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.1)
        isMultiShadow: true
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 6
            blurRadius: 13
            spread: 0
            shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.18)
            isMultiShadow: false
        }
    }

    property var kShadowMenu : ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 1
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.06)
        isMultiShadow: true
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 6
            blurRadius: 14
            spread: 0
            shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.2)
            isMultiShadow: false
        }
    }

    property var kShadowComponent : ShadowData {
        xOffset: 0
        yOffset: 1
        blurRadius: 2
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.1)
        isMultiShadow: false
    }

    property var kShadowMin : ShadowData {
        xOffset: 0
        yOffset: 2
        blurRadius: 6
        spread: 0
        shadowColor: Qt.rgba(0/255, 0/255, 0/255, 0.15)
        isMultiShadow: false
    }

    //2025.05.14添加
    property color kComponentSelectedNormal : kGray3
    property color kComponentSelectedHover: kGray4
    property color kComponentSelectedClick: kGray6
    property color kComponentSelectedDisable : Qt.rgba(224/255, 224/255, 224/255, 0.45)
    property color kComponentSelectedAlphaNormal: kGrayAlpha3
    property color kComponentSelectedAlphaHover: kGrayAlpha4
    property color kComponentSelectedAlphaClick: kGrayAlpha5
    property color kComponentSelectedAlphaDisable: Qt.rgba(0, 0, 0, 0.04)
    property color kComponentInput: kWhite
    property color kComponentInputAlpha: Qt.rgba(255/255, 255/255, 255/255, 0.5)

    //2025.06.25添加
    property color kSelectAlphaWhiteDisable : Qt.rgba(255/255, 255/255, 255/255, 0.45)
    property var kSelectAlphaWhiteHover : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.1)
        gradientBackground : Qt.rgba(255/255, 255/255, 255/255, 1)
    }
    property var kSelectAlphaWhiteClick : GradientColor {
        colorType: DtColorType.Gradient
        gradientDeg : 0
        gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0.2)
        gradientBackground : Qt.rgba(255/255, 255/255, 255/255, 1)
    }
    property color kMenu : kGray1
    property color kLineErrorDisable : Qt.rgba(245/255, 73/255, 73/255, 0.23)
    property color kContainSecondaryInactive : kGray2
    property color kSelectAlphaWhite : kGray0
    property color kFontPlaceholderText : Qt.rgba(0, 0, 0, 0.28)

    //2026.05.08添加
    property color kPanel: Qt.rgba(255/255, 255/255, 255/255, 0.65)
    property color kBrand10: Qt.rgba(25/255, 131/255, 255/255, 1)

    //描边
    property int normalLine : 1
    property int focusLine : 2

    //圆角
    property int kRadiusMin : 4
    property int kRadiusNormal : 6
    property int kRadiusMax : 8
    property int kRadiusMenu : 8
    property int kRadiusWindow : 12

    //间距
    property int kMarginMin : 4
    property int kMarginNormal : 8
    property int kMarginBig : 16
    property int kMarginWindow : 24
    property int kMarginComponent : 40

    //边距
    property int kPaddingMinLeft : 8
    property int kPaddingMinTop : 2
    property int kPaddingMinRight : 8
    property int kPaddingMinBottom : 2
    property int kPaddingNormalLeft : 16
    property int kPaddingNormalTop : 2
    property int kPaddingNormalRight : 16
    property int kPaddingNormalBottom : 2
    property int kPadding8Left : 8
    property int kPadding8Top : 8
    property int kPadding8Right : 8
    property int kPadding8Bottom : 8
    property int kPaddingWindowLeft : 24
    property int kPaddingWindowTop : 16
    property int kPaddingWindowRight : 24
    property int kPaddingWindowBottom : 24
}
