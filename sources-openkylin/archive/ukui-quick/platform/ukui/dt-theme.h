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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef DT_THEME_H
#define DT_THEME_H

#include <QObject>
#include <QQmlEngine>
#include "dt-theme-definition.h"

namespace UkuiQuick {
class ThemeGlobalConfigPrivate;
class ThemeGlobalConfig: public QObject
{
    Q_OBJECT

public:
    static ThemeGlobalConfig* instance();

    qreal transparency() const;
    QString themeName() const;
    QString styleColor() const;
    qreal fontSize() const;
    QString fontFamily() const;
    QString themeColor() const;
    QString windowRadius() const;
    QString iconThemeName() const;
    Qt::LayoutDirection layoutDirection() const;
    bool isDarkTheme() const;

Q_SIGNALS:
    void transparencyChanged();
    void themeNameChanged();
    void styleColorChanged();
    void themeColorChanged();
    void fontSizeChanged();
    void fontFamilyChanged();
    void windowRadiusChanged();
    void iconThemeNameChanged();
    void layoutDirectionChanged();

private:
    ThemeGlobalConfig();
    ~ThemeGlobalConfig();
    ThemeGlobalConfigPrivate *d = nullptr;

};
class DtThemePrivate;
//TODO 可通过AttachedProperty的方式将DtTheme注册,支持在qml中直接创建DtTheme类实现自定义token
class DtTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GradientColor* windowTextActive READ windowTextActive NOTIFY windowTextActiveChanged)
    Q_PROPERTY(GradientColor* windowTextInactive READ windowTextInactive NOTIFY windowTextInactiveChanged)
    Q_PROPERTY(GradientColor* windowTextDisable READ windowTextDisable NOTIFY windowTextDisableChanged)
    Q_PROPERTY(GradientColor* buttonActive READ buttonActive NOTIFY buttonActiveChanged)
    Q_PROPERTY(GradientColor* buttonInactive READ buttonInactive NOTIFY buttonInactiveChanged)
    Q_PROPERTY(GradientColor* buttonDisable READ buttonDisable NOTIFY buttonDisableChanged)
    Q_PROPERTY(GradientColor* lightActive READ lightActive NOTIFY lightActiveChanged)
    Q_PROPERTY(GradientColor* lightInactive READ lightInactive NOTIFY lightInactiveChanged)
    Q_PROPERTY(GradientColor* lightDisable READ lightDisable NOTIFY lightDisableChanged)
    Q_PROPERTY(GradientColor* darkActive READ darkActive NOTIFY darkActiveChanged)
    Q_PROPERTY(GradientColor* darkInactive READ darkInactive NOTIFY darkInactiveChanged)
    Q_PROPERTY(GradientColor* darkDisable READ darkDisable NOTIFY darkDisableChanged)
    Q_PROPERTY(GradientColor* baseActive READ baseActive NOTIFY baseActiveChanged)
    Q_PROPERTY(GradientColor* baseInactive READ baseInactive NOTIFY baseInactiveChanged)
    Q_PROPERTY(GradientColor* baseDisable READ baseDisable NOTIFY baseDisableChanged)
    Q_PROPERTY(GradientColor* midLightActive READ midLightActive NOTIFY midLightActiveChanged)
    Q_PROPERTY(GradientColor* midLightInactive READ midLightInactive NOTIFY midLightInactiveChanged)
    Q_PROPERTY(GradientColor* midLightDisable READ midLightDisable NOTIFY midLightDisableChanged)
    Q_PROPERTY(GradientColor* midActive READ midActive NOTIFY midActiveChanged)
    Q_PROPERTY(GradientColor* midInactive READ midInactive NOTIFY midInactiveChanged)
    Q_PROPERTY(GradientColor* midDisable READ midDisable NOTIFY midDisableChanged)
    Q_PROPERTY(GradientColor* textActive READ textActive NOTIFY textActiveChanged)
    Q_PROPERTY(GradientColor* textInactive READ textInactive NOTIFY textInactiveChanged)
    Q_PROPERTY(GradientColor* textDisable READ textDisable NOTIFY textDisableChanged)
    Q_PROPERTY(GradientColor* brightTextActive READ brightTextActive NOTIFY brightTextActiveChanged)
    Q_PROPERTY(GradientColor* brightTextInactive READ brightTextInactive NOTIFY brightTextInactiveChanged)
    Q_PROPERTY(GradientColor* brightTextDisable READ brightTextDisable NOTIFY brightTextDisableChanged)
    Q_PROPERTY(GradientColor* buttonTextActive READ buttonTextActive NOTIFY buttonTextActiveChanged)
    Q_PROPERTY(GradientColor* buttonTextInactive READ buttonTextInactive NOTIFY buttonTextInactiveChanged)
    Q_PROPERTY(GradientColor* buttonTextDisable READ buttonTextDisable NOTIFY buttonTextDisableChanged)
    Q_PROPERTY(GradientColor* windowActive READ windowActive NOTIFY windowActiveChanged)
    Q_PROPERTY(GradientColor* windowInactive READ windowInactive NOTIFY windowInactiveChanged)
    Q_PROPERTY(GradientColor* windowDisable READ windowDisable NOTIFY windowDisableChanged)
    Q_PROPERTY(GradientColor* shadowActive READ shadowActive NOTIFY shadowActiveChanged)
    Q_PROPERTY(GradientColor* shadowInactive READ shadowInactive NOTIFY shadowInactiveChanged)
    Q_PROPERTY(GradientColor* shadowDisable READ shadowDisable NOTIFY shadowDisableChanged)
    Q_PROPERTY(GradientColor* highlightActive READ highlightActive NOTIFY highlightActiveChanged)
    Q_PROPERTY(GradientColor* highlightInactive READ highlightInactive NOTIFY highlightInactiveChanged)
    Q_PROPERTY(GradientColor* highlightDisable READ highlightDisable NOTIFY highlightDisableChanged)
    Q_PROPERTY(GradientColor* linkActive READ linkActive NOTIFY linkActiveChanged)
    Q_PROPERTY(GradientColor* linkInactive READ linkInactive NOTIFY linkInactiveChanged)
    Q_PROPERTY(GradientColor* linkDisable READ linkDisable NOTIFY linkDisableChanged)
    Q_PROPERTY(GradientColor* linkVisitedDisable READ linkVisitedDisable NOTIFY linkVisitedDisableChanged)
    Q_PROPERTY(GradientColor* alternateBaseActive READ alternateBaseActive NOTIFY alternateBaseActiveChanged)
    Q_PROPERTY(GradientColor* alternateBaseInactive READ alternateBaseInactive NOTIFY alternateBaseInactiveChanged)
    Q_PROPERTY(GradientColor* alternateBaseDisable READ alternateBaseDisable NOTIFY alternateBaseDisableChanged)
    Q_PROPERTY(GradientColor* noRoleActive READ noRoleActive NOTIFY noRoleActiveChanged)
    Q_PROPERTY(GradientColor* noRoleInactive READ noRoleInactive NOTIFY noRoleInactiveChanged)
    Q_PROPERTY(GradientColor* noRoleDisable READ noRoleDisable NOTIFY noRoleDisableChanged)
    Q_PROPERTY(GradientColor* toolTipBaseActive READ toolTipBaseActive NOTIFY toolTipBaseActiveChanged)
    Q_PROPERTY(GradientColor* toolTipBaseInactive READ toolTipBaseInactive NOTIFY toolTipBaseInactiveChanged)
    Q_PROPERTY(GradientColor* toolTipBaseDisable READ toolTipBaseDisable NOTIFY toolTipBaseDisableChanged)
    Q_PROPERTY(GradientColor* toolTipTextActive READ toolTipTextActive NOTIFY toolTipTextActiveChanged)
    Q_PROPERTY(GradientColor* toolTipTextInactive READ toolTipTextInactive NOTIFY toolTipTextInactiveChanged)
    Q_PROPERTY(GradientColor* toolTipTextDisable READ toolTipTextDisable NOTIFY toolTipTextDisableChanged)
    Q_PROPERTY(GradientColor* placeholderTextActive READ placeholderTextActive NOTIFY placeholderTextActiveChanged)
    Q_PROPERTY(GradientColor* placeholderTextInactive READ placeholderTextInactive NOTIFY placeholderTextInactiveChanged)
    Q_PROPERTY(GradientColor* placeholderTextDisable READ placeholderTextDisable NOTIFY placeholderTextDisableChanged)
    Q_PROPERTY(GradientColor* kWhite READ kWhite NOTIFY kWhiteChanged)
    Q_PROPERTY(GradientColor* kBlack READ kBlack NOTIFY kBlackChanged)
    Q_PROPERTY(GradientColor* kGray0 READ kGray0 NOTIFY kGray0Changed)
    Q_PROPERTY(GradientColor* kGray1 READ kGray1 NOTIFY kGray1Changed)
    Q_PROPERTY(GradientColor* kGray2 READ kGray2 NOTIFY kGray2Changed)
    Q_PROPERTY(GradientColor* kGray3 READ kGray3 NOTIFY kGray3Changed)
    Q_PROPERTY(GradientColor* kGray4 READ kGray4 NOTIFY kGray4Changed)
    Q_PROPERTY(GradientColor* kGray5 READ kGray5 NOTIFY kGray5Changed)
    Q_PROPERTY(GradientColor* kGray6 READ kGray6 NOTIFY kGray6Changed)
    Q_PROPERTY(GradientColor* kGray7 READ kGray7 NOTIFY kGray7Changed)
    Q_PROPERTY(GradientColor* kGray8 READ kGray8 NOTIFY kGray8Changed)
    Q_PROPERTY(GradientColor* kGray9 READ kGray9 NOTIFY kGray9Changed)
    Q_PROPERTY(GradientColor* kGray10 READ kGray10 NOTIFY kGray10Changed)
    Q_PROPERTY(GradientColor* kGray11 READ kGray11 NOTIFY kGray11Changed)
    Q_PROPERTY(GradientColor* kGray12 READ kGray12 NOTIFY kGray12Changed)
    Q_PROPERTY(GradientColor* kGray13 READ kGray13 NOTIFY kGray13Changed)
    Q_PROPERTY(GradientColor* kGray14 READ kGray14 NOTIFY kGray14Changed)
    Q_PROPERTY(GradientColor* kGray15 READ kGray15 NOTIFY kGray15Changed)
    Q_PROPERTY(GradientColor* kGray16 READ kGray16 NOTIFY kGray16Changed)
    Q_PROPERTY(GradientColor* kGray17 READ kGray17 NOTIFY kGray17Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha0 READ kGrayAlpha0 NOTIFY kGrayAlpha0Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha1 READ kGrayAlpha1 NOTIFY kGrayAlpha1Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha2 READ kGrayAlpha2 NOTIFY kGrayAlpha2Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha3 READ kGrayAlpha3 NOTIFY kGrayAlpha3Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha4 READ kGrayAlpha4 NOTIFY kGrayAlpha4Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha5 READ kGrayAlpha5 NOTIFY kGrayAlpha5Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha6 READ kGrayAlpha6 NOTIFY kGrayAlpha6Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha7 READ kGrayAlpha7 NOTIFY kGrayAlpha7Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha8 READ kGrayAlpha8 NOTIFY kGrayAlpha8Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha9 READ kGrayAlpha9 NOTIFY kGrayAlpha9Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha10 READ kGrayAlpha10 NOTIFY kGrayAlpha10Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha11 READ kGrayAlpha11 NOTIFY kGrayAlpha11Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha12 READ kGrayAlpha12 NOTIFY kGrayAlpha12Changed)
    Q_PROPERTY(GradientColor* kGrayAlpha13 READ kGrayAlpha13 NOTIFY kGrayAlpha13Changed)
    Q_PROPERTY(GradientColor* kFontStrong READ kFontStrong NOTIFY kFontStrongChanged)
    Q_PROPERTY(GradientColor* kFontPrimary READ kFontPrimary NOTIFY kFontPrimaryChanged)
    Q_PROPERTY(GradientColor* kFontPrimaryDisable READ kFontPrimaryDisable NOTIFY kFontPrimaryDisableChanged)
    Q_PROPERTY(GradientColor* kFontSecondary READ kFontSecondary NOTIFY kFontSecondaryChanged)
    Q_PROPERTY(GradientColor* kFontSecondaryDisable READ kFontSecondaryDisable NOTIFY kFontSecondaryDisableChanged)
    Q_PROPERTY(GradientColor* kFontWhite READ kFontWhite NOTIFY kFontWhiteChanged)
    Q_PROPERTY(GradientColor* kFontWhiteDisable READ kFontWhiteDisable NOTIFY kFontWhiteDisableChanged)
    Q_PROPERTY(GradientColor* kFontWhiteSecondary READ kFontWhiteSecondary NOTIFY kFontWhiteSecondaryChanged)
    Q_PROPERTY(GradientColor* kFontWhiteSecondaryDisable READ kFontWhiteSecondaryDisable NOTIFY kFontWhiteSecondaryDisableChanged)
    Q_PROPERTY(GradientColor* kBrandNormal READ kBrandNormal NOTIFY kBrandNormalChanged)
    Q_PROPERTY(GradientColor* kSuccessNormal READ kSuccessNormal NOTIFY kSuccessNormalChanged)
    Q_PROPERTY(GradientColor* kWarningNormal READ kWarningNormal NOTIFY kWarningNormalChanged)
    Q_PROPERTY(GradientColor* kErrorNormal READ kErrorNormal NOTIFY kErrorNormalChanged)
    Q_PROPERTY(GradientColor* kBrand1 READ kBrand1 NOTIFY kBrand1Changed)
    Q_PROPERTY(GradientColor* kBrand2 READ kBrand2 NOTIFY kBrand2Changed)
    Q_PROPERTY(GradientColor* kBrand3 READ kBrand3 NOTIFY kBrand3Changed)
    Q_PROPERTY(GradientColor* kBrand4 READ kBrand4 NOTIFY kBrand4Changed)
    Q_PROPERTY(GradientColor* kBrand5 READ kBrand5 NOTIFY kBrand5Changed)
    Q_PROPERTY(GradientColor* kBrand6 READ kBrand6 NOTIFY kBrand6Changed)
    Q_PROPERTY(GradientColor* kBrand7 READ kBrand7 NOTIFY kBrand7Changed)
    Q_PROPERTY(GradientColor* kContainGeneralNormal READ kContainGeneralNormal NOTIFY kContainGeneralNormalChanged)
    Q_PROPERTY(GradientColor* kContainSecondaryNormal READ kContainSecondaryNormal NOTIFY kContainSecondaryNormalChanged)
    Q_PROPERTY(GradientColor* kContainSecondaryAlphaNormal READ kContainSecondaryAlphaNormal NOTIFY kContainSecondaryAlphaNormalChanged)
    Q_PROPERTY(GradientColor* kComponentNormal READ kComponentNormal NOTIFY kComponentNormalChanged)
    Q_PROPERTY(GradientColor* kComponentDisable READ kComponentDisable NOTIFY kComponentDisableChanged)
    Q_PROPERTY(GradientColor* kComponentAlphaNormal READ kComponentAlphaNormal NOTIFY kComponentAlphaNormalChanged)
    Q_PROPERTY(GradientColor* kComponentAlphaDisable READ kComponentAlphaDisable NOTIFY kComponentAlphaDisableChanged)
    Q_PROPERTY(GradientColor* kLineWindowActive READ kLineWindowActive NOTIFY kLineWindowActiveChanged)
    Q_PROPERTY(GradientColor* kLineComponentNormal READ kLineComponentNormal NOTIFY kLineComponentNormalChanged)
    Q_PROPERTY(GradientColor* kLineComponentHover READ kLineComponentHover NOTIFY kLineComponentHoverChanged)
    Q_PROPERTY(GradientColor* kLineComponentClick READ kLineComponentClick NOTIFY kLineComponentClickChanged)
    Q_PROPERTY(GradientColor* kLineComponentDisable READ kLineComponentDisable NOTIFY kLineComponentDisableChanged)
    Q_PROPERTY(GradientColor* kLineBrandNormal READ kLineBrandNormal NOTIFY kLineBrandNormalChanged)
    Q_PROPERTY(GradientColor* kLineBrandHover READ kLineBrandHover NOTIFY kLineBrandHoverChanged)
    Q_PROPERTY(GradientColor* kLineBrandClick READ kLineBrandClick NOTIFY kLineBrandClickChanged)
    Q_PROPERTY(GradientColor* kLineBrandDisable READ kLineBrandDisable NOTIFY kLineBrandDisableChanged)
    Q_PROPERTY(GradientColor* kModalMask READ kModalMask NOTIFY kModalMaskChanged)
    Q_PROPERTY(int normalLine READ normalLine NOTIFY normalLineChanged)
    Q_PROPERTY(int focusLine READ focusLine NOTIFY focusLineChanged)
    Q_PROPERTY(int kRadiusMin READ kRadiusMin NOTIFY kRadiusMinChanged)
    Q_PROPERTY(int kRadiusNormal  READ kRadiusNormal NOTIFY kRadiusNormalChanged)
    Q_PROPERTY(int kRadiusMax READ kRadiusMax NOTIFY kRadiusMaxChanged)
    Q_PROPERTY(int kRadiusMenu READ kRadiusMenu NOTIFY kRadiusMenuChanged)
    Q_PROPERTY(int kRadiusWindow READ kRadiusWindow NOTIFY kRadiusWindowChanged)
    Q_PROPERTY(int kMarginMin  READ kMarginMin NOTIFY kMarginMinChanged)
    Q_PROPERTY(int kMarginNormal READ kMarginNormal NOTIFY kMarginNormalChanged)
    Q_PROPERTY(int kMarginBig READ kMarginBig NOTIFY kMarginBigChanged)
    Q_PROPERTY(int kMarginWindow READ kMarginWindow NOTIFY kMarginWindowChanged)
    Q_PROPERTY(int kMarginComponent READ kMarginComponent NOTIFY kMarginComponentChanged)
    Q_PROPERTY(int kPaddingMinLeft READ kPaddingMinLeft NOTIFY kPaddingMinLeftChanged)
    Q_PROPERTY(int kPaddingMinRight READ kPaddingMinRight NOTIFY kPaddingMinRightChanged)
    Q_PROPERTY(int kPaddingMinTop READ kPaddingMinTop NOTIFY kPaddingMinTopChanged)
    Q_PROPERTY(int kPaddingMinBottom READ kPaddingMinBottom NOTIFY kPaddingMinBottomChanged)
    Q_PROPERTY(int kPaddingNormalLeft READ kPaddingNormalLeft NOTIFY kPaddingNormalLeftChanged)
    Q_PROPERTY(int kPaddingNormalRight READ kPaddingNormalRight NOTIFY kPaddingNormalRightChanged)
    Q_PROPERTY(int kPaddingNormalTop READ kPaddingNormalTop NOTIFY kPaddingNormalTopChanged)
    Q_PROPERTY(int kPaddingNormalBottom READ kPaddingNormalBottom NOTIFY kPaddingNormalBottomChanged)
    Q_PROPERTY(int kPadding8Left READ kPadding8Left NOTIFY kPadding8LeftChanged)
    Q_PROPERTY(int kPadding8Right READ kPadding8Right NOTIFY kPadding8RightChanged)
    Q_PROPERTY(int kPadding8Top READ kPadding8Top NOTIFY kPadding8TopChanged)
    Q_PROPERTY(int kPadding8Bottom READ kPadding8Bottom NOTIFY kPadding8BottomChanged)
    Q_PROPERTY(int kPaddingWindowLeft READ kPaddingWindowLeft NOTIFY kPaddingWindowLeftChanged)
    Q_PROPERTY(int kPaddingWindowRight READ kPaddingWindowRight NOTIFY kPaddingWindowRightChanged)
    Q_PROPERTY(int kPaddingWindowTop READ kPaddingWindowTop NOTIFY kPaddingWindowTopChanged)
    Q_PROPERTY(int kPaddingWindowBottom READ kPaddingWindowBottom NOTIFY kPaddingWindowBottomChanged)

    Q_PROPERTY(GradientColor* linkVisitedActive READ linkVisitedActive NOTIFY linkVisitedActiveChanged)
    Q_PROPERTY(GradientColor* linkVisitedInactive READ linkVisitedInactive NOTIFY linkVisitedInactiveChanged)
    Q_PROPERTY(GradientColor* kBrandHover READ kBrandHover NOTIFY kBrandHoverChanged)
    Q_PROPERTY(GradientColor* kBrandClick READ kBrandClick NOTIFY kBrandClickChanged)
    Q_PROPERTY(GradientColor* kBrandFocus READ kBrandFocus NOTIFY kBrandFocusChanged)
    Q_PROPERTY(GradientColor* kSuccessHover READ kSuccessHover NOTIFY kSuccessHoverChanged)
    Q_PROPERTY(GradientColor* kSuccessClick READ kSuccessClick NOTIFY kSuccessClickChanged)
    Q_PROPERTY(GradientColor* kWarningHover READ kWarningHover NOTIFY kWarningHoverChanged)
    Q_PROPERTY(GradientColor* kWarningClick READ kWarningClick NOTIFY kWarningClickChanged)
    Q_PROPERTY(GradientColor* kErrorHover READ kErrorHover NOTIFY kErrorHoverChanged)
    Q_PROPERTY(GradientColor* kErrorClick READ kErrorClick NOTIFY kErrorClickChanged)
    Q_PROPERTY(GradientColor* kContainHover READ kContainHover NOTIFY kContainHoverChanged)
    Q_PROPERTY(GradientColor* kContainClick READ kContainClick NOTIFY kContainClickChanged)
    Q_PROPERTY(GradientColor* kComponentHover READ kComponentHover NOTIFY kComponentHoverChanged)
    Q_PROPERTY(GradientColor* kComponentClick READ kComponentClick NOTIFY kComponentClickChanged)
    Q_PROPERTY(GradientColor* kComponentAlphaHover READ kComponentAlphaHover NOTIFY kComponentAlphaHoverChanged)
    Q_PROPERTY(GradientColor* kComponentAlphaClick READ kComponentAlphaClick NOTIFY kComponentAlphaClickChanged)

    Q_PROPERTY(GradientColor* kErrorDisable READ kErrorDisable NOTIFY kErrorDisableChanged)
    Q_PROPERTY(GradientColor* kContainGeneralAlphaNormal READ kContainGeneralAlphaNormal NOTIFY kContainGeneralAlphaNormalChanged)
    Q_PROPERTY(GradientColor* kSuccessDisable READ kSuccessDisable NOTIFY kSuccessDisableChanged)
    Q_PROPERTY(GradientColor* kLineWindowInactive READ kLineWindowInactive NOTIFY kLineWindowInactiveChanged)
    Q_PROPERTY(GradientColor* kContainAlphaClick READ kContainAlphaClick NOTIFY kContainAlphaClickChanged)
    Q_PROPERTY(GradientColor* kWarningDisable READ kWarningDisable NOTIFY kWarningDisableChanged)
    Q_PROPERTY(GradientColor* kDivider READ kDivider NOTIFY kDividerChanged)
    Q_PROPERTY(GradientColor* kContainAlphaHover READ kContainAlphaHover NOTIFY kContainAlphaHoverChanged)
    Q_PROPERTY(GradientColor* highlightedTextActive READ highlightedTextActive NOTIFY highlightedTextActiveChanged)
    Q_PROPERTY(GradientColor* highlightedTextInactive READ highlightedTextInactive NOTIFY highlightedTextInactiveChanged)
    Q_PROPERTY(GradientColor* highlightedTextDisable READ highlightedTextDisable NOTIFY highlightedTextDisableChanged)
    Q_PROPERTY(GradientColor* kBrandDisable READ kBrandDisable NOTIFY kBrandDisableChanged)
    Q_PROPERTY(GradientColor* kLineDisable READ kLineDisable NOTIFY kLineDisableChanged)
    Q_PROPERTY(GradientColor* kLineNormal READ kLineNormal NOTIFY kLineNormalChanged)

    Q_PROPERTY(qreal transparency READ transparency NOTIFY transparencyChanged)
    Q_PROPERTY(qreal fontSize READ fontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY fontFamilyChanged)

    Q_PROPERTY(GradientColor* kBrand8 READ kBrand8 NOTIFY kBrand8Changed)
    Q_PROPERTY(GradientColor* kBrand9 READ kBrand9 NOTIFY kBrand9Changed)
    Q_PROPERTY(GradientColor* kLineSelectboxNormal READ kLineSelectboxNormal NOTIFY kLineSelectboxNormalChanged)
    Q_PROPERTY(GradientColor* kLineSelectboxHover READ kLineSelectboxHover NOTIFY kLineSelectboxHoverChanged)
    Q_PROPERTY(GradientColor* kLineSelectboxClick READ kLineSelectboxClick NOTIFY kLineSelectboxClickChanged)
    Q_PROPERTY(GradientColor* kLineSelectboxSelected READ kLineSelectboxSelected NOTIFY kLineSelectboxSelectedChanged)
    Q_PROPERTY(GradientColor* kLineSelectboxDisable READ kLineSelectboxDisable NOTIFY kLineSelectboxDisableChanged)
    Q_PROPERTY(GradientColor* kFontWhitePlaceholderTextDisable READ kFontWhitePlaceholderTextDisable NOTIFY kFontWhitePlaceholderTextDisableChanged)
    Q_PROPERTY(GradientColor* kContainGeneralInactive READ kContainGeneralInactive NOTIFY kContainGeneralInactiveChanged)
    Q_PROPERTY(GradientColor* kFontPlaceholderTextDisable READ kFontPlaceholderTextDisable NOTIFY kFontPlaceholderTextDisableChanged)
    Q_PROPERTY(GradientColor* kFontWhitePlaceholderText READ kFontWhitePlaceholderText NOTIFY kFontWhitePlaceholderTextChanged)
    Q_PROPERTY(GradientColor* kLineInputDisable READ kLineInputDisable NOTIFY kLineInputDisableChanged)
    Q_PROPERTY(GradientColor* kLineDock READ kLineDock NOTIFY kLineDockChanged)
    Q_PROPERTY(GradientColor* kLineMenu READ kLineMenu NOTIFY kLineMenuChanged)
    Q_PROPERTY(GradientColor* kLineTable READ kLineTable NOTIFY kLineTableChanged)
    Q_PROPERTY(GradientColor* kLineInputNormal READ kLineInputNormal NOTIFY kLineInputNormalChanged)
    Q_PROPERTY(GradientColor* kLineInputHover READ kLineInputHover NOTIFY kLineInputHoverChanged)
    Q_PROPERTY(GradientColor* kLineInputClick READ kLineInputClick NOTIFY kLineInputClickChanged)
    Q_PROPERTY(GradientColor* kOrange1 READ kOrange1 NOTIFY kOrange1Changed)
    Q_PROPERTY(GradientColor* kGreen1 READ kGreen1 NOTIFY kGreen1Changed)
    Q_PROPERTY(GradientColor* kRed1 READ kRed1 NOTIFY kRed1Changed)
    Q_PROPERTY(GradientColor* kDividerWhite READ kDividerWhite NOTIFY kDividerWhiteChanged)
    Q_PROPERTY(GradientColor* kLineSuccessNormal READ kLineSuccessNormal NOTIFY kLineSuccessNormalChanged)
    Q_PROPERTY(GradientColor* kLineSuccessHover READ kLineSuccessHover NOTIFY kLineSuccessHoverChanged)
    Q_PROPERTY(GradientColor* kLineSuccessClick READ kLineSuccessClick NOTIFY kLineSuccessClickChanged)
    Q_PROPERTY(GradientColor* kLineSuccessDisable READ kLineSuccessDisable NOTIFY kLineSuccessDisableChanged)
    Q_PROPERTY(GradientColor* kLineErrorNormal READ kLineErrorNormal NOTIFY kLineErrorNormalChanged)
    Q_PROPERTY(GradientColor* kLineErrorHover READ kLineErrorHover NOTIFY kLineErrorHoverChanged)
    Q_PROPERTY(GradientColor* kLineErrorClick READ kLineErrorClick NOTIFY kLineErrorClickChanged)
    Q_PROPERTY(GradientColor* kLineWarningNormal READ kLineWarningNormal NOTIFY kLineWarningNormalChanged)
    Q_PROPERTY(GradientColor* kLineWarningHover READ kLineWarningHover NOTIFY kLineWarningHoverChanged)
    Q_PROPERTY(GradientColor* kLineWarningClick READ kLineWarningClick NOTIFY kLineWarningClickChanged)
    Q_PROPERTY(GradientColor* kLineWarningDisable READ kLineWarningDisable NOTIFY kLineWarningDisableChanged)
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection NOTIFY layoutDirectionChanged)

    Q_PROPERTY(ShadowData* kShadowPrimaryActive READ kShadowPrimaryActive NOTIFY kShadowPrimaryActiveChanged)
    Q_PROPERTY(ShadowData* kShadowPrimaryInactive READ kShadowPrimaryInactive NOTIFY kShadowPrimaryInactiveChanged)
    Q_PROPERTY(ShadowData* kShadowSecondaryActive READ kShadowSecondaryActive NOTIFY kShadowSecondaryActiveChanged)
    Q_PROPERTY(ShadowData* kShadowSecondaryInactive READ kShadowSecondaryInactive NOTIFY kShadowSecondaryInactiveChanged)
    Q_PROPERTY(ShadowData* kShadowMin READ kShadowMin NOTIFY kShadowMinChanged)
    Q_PROPERTY(ShadowData* kShadowMenu READ kShadowMenu NOTIFY kShadowMenuChanged)
    Q_PROPERTY(ShadowData* kShadowComponent READ kShadowComponent NOTIFY kShadowComponentChanged)

    //2025.05.14添加
    Q_PROPERTY(GradientColor* kComponentSelectedNormal READ kComponentSelectedNormal NOTIFY kComponentSelectedNormalChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedHover READ kComponentSelectedHover NOTIFY kComponentSelectedHoverChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedClick READ kComponentSelectedClick NOTIFY kComponentSelectedClickChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedDisable READ kComponentSelectedDisable NOTIFY kComponentSelectedDisableChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedAlphaNormal READ kComponentSelectedAlphaNormal NOTIFY kComponentSelectedAlphaNormalChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedAlphaHover READ kComponentSelectedAlphaHover NOTIFY kComponentSelectedAlphaHoverChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedAlphaClick READ kComponentSelectedAlphaClick NOTIFY kComponentSelectedAlphaClickChanged)
    Q_PROPERTY(GradientColor* kComponentSelectedAlphaDisable READ kComponentSelectedAlphaDisable NOTIFY kComponentSelectedAlphaDisableChanged)
    Q_PROPERTY(GradientColor* kComponentInput READ kComponentInput NOTIFY kComponentInputChanged)
    Q_PROPERTY(GradientColor* kComponentInputAlpha READ kComponentInputAlpha NOTIFY kComponentInputAlphaChanged)

    //2025.06.25添加
    Q_PROPERTY(GradientColor* kSelectAlphaWhite READ kSelectAlphaWhite NOTIFY kSelectAlphaWhiteChanged)
    Q_PROPERTY(GradientColor* kSelectAlphaWhiteHover READ kSelectAlphaWhiteHover NOTIFY kSelectAlphaWhiteHoverChanged)
    Q_PROPERTY(GradientColor* kSelectAlphaWhiteClick READ kSelectAlphaWhiteClick NOTIFY kSelectAlphaWhiteClickChanged)
    Q_PROPERTY(GradientColor* kSelectAlphaWhiteDisable READ kSelectAlphaWhiteDisable NOTIFY kSelectAlphaWhiteDisableChanged)
    Q_PROPERTY(GradientColor* kMenu READ kMenu NOTIFY kMenuChanged)
    Q_PROPERTY(GradientColor* kLineErrorDisable READ kLineErrorDisable NOTIFY kLineErrorDisableChanged)
    Q_PROPERTY(GradientColor* kContainSecondaryInactive READ kContainSecondaryInactive NOTIFY kContainSecondaryInactiveChanged)
    Q_PROPERTY(GradientColor* kFontPlaceholderText READ kFontPlaceholderText NOTIFY kFontPlaceholderTextChanged)
    //2026.04.22添加
    Q_PROPERTY(GradientColor* kPanel READ kPanel NOTIFY kPanelChanged)
    Q_PROPERTY(GradientColor* kBrand10 READ kBrand10 NOTIFY kBrand10Changed)
public:
    static DtTheme *self(QQmlEngine *engine);
    ~DtTheme();

    GradientColor* windowTextActive() const;
    GradientColor* windowTextInactive() const;
    GradientColor* windowTextDisable() const;
    GradientColor* buttonActive() const;
    GradientColor* buttonInactive() const;
    GradientColor* buttonDisable() const;
    GradientColor* lightActive() const;
    GradientColor* lightInactive() const;
    GradientColor* lightDisable() const;
    GradientColor* midLightActive() const;
    GradientColor* midLightInactive() const;
    GradientColor* midLightDisable() const;
    GradientColor* darkActive() const;
    GradientColor* darkInactive() const;
    GradientColor* darkDisable() const;
    GradientColor* midActive() const;
    GradientColor* midInactive() const;
    GradientColor* midDisable() const;
    GradientColor* textActive() const;
    GradientColor* textInactive() const;
    GradientColor* textDisable() const;
    GradientColor* brightTextActive() const;
    GradientColor* brightTextInactive() const;
    GradientColor* brightTextDisable() const;
    GradientColor* buttonTextActive() const;
    GradientColor* buttonTextInactive() const;
    GradientColor* buttonTextDisable() const;
    GradientColor* baseActive() const;
    GradientColor* baseInactive() const;
    GradientColor* baseDisable() const;
    GradientColor* windowActive() const;
    GradientColor* windowInactive() const;
    GradientColor* windowDisable() const;
    GradientColor* shadowActive() const;
    GradientColor* shadowInactive() const;
    GradientColor* shadowDisable() const;
    GradientColor* highlightActive() const;
    GradientColor* highlightInactive() const;
    GradientColor* highlightDisable() const;
    GradientColor* highlightedTextActive() const;
    GradientColor* highlightedTextInactive() const;
    GradientColor* highlightedTextDisable() const;
    GradientColor* linkActive() const;
    GradientColor* linkInactive() const;
    GradientColor* linkDisable() const;
    GradientColor* linkVisitedDisable() const;
    GradientColor* alternateBaseActive() const;
    GradientColor* alternateBaseInactive() const;
    GradientColor* alternateBaseDisable() const;
    GradientColor* noRoleActive() const;
    GradientColor* noRoleInactive() const;
    GradientColor* noRoleDisable() const;
    GradientColor* toolTipBaseActive() const;
    GradientColor* toolTipBaseInactive() const;
    GradientColor* toolTipBaseDisable() const;
    GradientColor* toolTipTextActive() const;
    GradientColor* toolTipTextInactive() const;
    GradientColor* toolTipTextDisable() const;
    GradientColor* placeholderTextActive() const;
    GradientColor* placeholderTextInactive() const;
    GradientColor* placeholderTextDisable() const;

    GradientColor* kWhite() const;
    GradientColor* kBlack() const;
    GradientColor* kGray0() const;
    GradientColor* kGray1() const;
    GradientColor* kGray2() const;
    GradientColor* kGray3() const;
    GradientColor* kGray4() const;
    GradientColor* kGray5() const;
    GradientColor* kGray6() const;
    GradientColor* kGray7() const;
    GradientColor* kGray8() const;
    GradientColor* kGray9() const;
    GradientColor* kGray10() const;
    GradientColor* kGray11() const;
    GradientColor* kGray12() const;
    GradientColor* kGray13() const;
    GradientColor* kGray14() const;
    GradientColor* kGray15() const;
    GradientColor* kGray16() const;
    GradientColor* kGray17() const;
    GradientColor* kGrayAlpha0() const;
    GradientColor* kGrayAlpha1() const;
    GradientColor* kGrayAlpha2() const;
    GradientColor* kGrayAlpha3() const;
    GradientColor* kGrayAlpha4() const;
    GradientColor* kGrayAlpha5() const;
    GradientColor* kGrayAlpha6() const;
    GradientColor* kGrayAlpha7() const;
    GradientColor* kGrayAlpha8() const;
    GradientColor* kGrayAlpha9() const;
    GradientColor* kGrayAlpha10() const;
    GradientColor* kGrayAlpha11() const;
    GradientColor* kGrayAlpha12() const;
    GradientColor* kGrayAlpha13() const;

    GradientColor* kFontStrong() const;
    GradientColor* kFontPrimary() const;
    GradientColor* kFontPrimaryDisable() const;
    GradientColor* kFontSecondary() const;
    GradientColor* kFontSecondaryDisable() const;
    GradientColor* kFontWhite() const;
    GradientColor* kFontWhiteDisable() const;
    GradientColor* kFontWhiteSecondary() const;
    GradientColor* kFontWhiteSecondaryDisable() const;

    GradientColor* kBrandNormal() const;
    GradientColor* kSuccessNormal() const;
    GradientColor* kWarningNormal() const;
    GradientColor* kErrorNormal() const;
    GradientColor* kBrand1() const;
    GradientColor* kBrand2() const;
    GradientColor* kBrand3() const;
    GradientColor* kBrand4() const;
    GradientColor* kBrand5() const;
    GradientColor* kBrand6() const;
    GradientColor* kBrand7() const;

    GradientColor* kContainGeneralNormal() const;
    GradientColor* kContainSecondaryNormal() const;
    GradientColor* kContainSecondaryAlphaNormal() const;
    GradientColor* kComponentNormal() const;
    GradientColor* kComponentDisable() const;
    GradientColor* kComponentAlphaNormal() const;
    GradientColor* kComponentAlphaDisable() const;
    GradientColor* kLineWindowActive() const;
    GradientColor* kLineComponentNormal() const;
    GradientColor* kLineComponentHover() const;
    GradientColor* kLineComponentClick() const;
    GradientColor* kLineComponentDisable() const;
    GradientColor* kLineBrandNormal() const;
    GradientColor* kLineBrandHover() const;
    GradientColor* kLineBrandClick() const;
    GradientColor* kLineBrandDisable() const;
    GradientColor* kModalMask() const;
    int normalLine() const;
    int focusLine() const;
    int kRadiusMin() const;
    int kRadiusNormal() const;
    int kRadiusMax() const;
    int kRadiusMenu() const;
    int kRadiusWindow() const;
    int kMarginMin() const;
    int kMarginNormal() const;
    int kMarginBig() const;
    int kMarginWindow() const;
    int kMarginComponent() const;
    int kPaddingMinLeft() const;
    int kPaddingMinTop() const;
    int kPaddingMinRight() const;
    int kPaddingMinBottom() const;
    int kPaddingNormalLeft() const;
    int kPaddingNormalTop() const;
    int kPaddingNormalRight() const;
    int kPaddingNormalBottom() const;
    int kPadding8Left() const;
    int kPadding8Top() const;
    int kPadding8Right() const;
    int kPadding8Bottom() const;
    int kPaddingWindowLeft() const;
    int kPaddingWindowTop() const;
    int kPaddingWindowRight() const;
    int kPaddingWindowBottom() const;

    GradientColor* linkVisitedActive() const;
    GradientColor* linkVisitedInactive() const;
    GradientColor* kBrandHover() const;
    GradientColor* kBrandClick() const;
    GradientColor* kBrandFocus() const;
    GradientColor* kSuccessHover() const;
    GradientColor* kSuccessClick() const;
    GradientColor* kWarningHover() const;
    GradientColor* kWarningClick() const;
    GradientColor* kErrorHover() const;
    GradientColor* kErrorClick() const;
    GradientColor* kContainHover() const;
    GradientColor* kContainClick() const;
    GradientColor* kComponentHover() const;
    GradientColor* kComponentClick() const;
    GradientColor* kComponentAlphaHover() const;
    GradientColor* kComponentAlphaClick() const;

    GradientColor* kErrorDisable() const;
    GradientColor* kContainGeneralAlphaNormal() const;
    GradientColor* kSuccessDisable() const;
    GradientColor* kLineWindowInactive() const;
    GradientColor* kContainAlphaClick() const;
    GradientColor* kWarningDisable() const;
    GradientColor* kDivider() const;
    GradientColor* kContainAlphaHover() const;
    GradientColor* kBrandDisable() const;
    GradientColor* kLineDisable() const;
    GradientColor* kLineNormal() const;

    qreal transparency() const;
    qreal fontSize() const;
    QString fontFamily() const;
    Qt::LayoutDirection layoutDirection() const;

    GradientColor* kBrand8() const;
    GradientColor* kBrand9() const;
    GradientColor* kLineSelectboxNormal() const;
    GradientColor* kLineSelectboxHover() const;
    GradientColor* kLineSelectboxClick() const;
    GradientColor* kLineSelectboxSelected() const;
    GradientColor* kLineSelectboxDisable() const;
    GradientColor* kFontWhitePlaceholderTextDisable() const;
    GradientColor* kContainGeneralInactive() const;
    GradientColor* kFontPlaceholderTextDisable() const;
    GradientColor* kFontWhitePlaceholderText() const;
    GradientColor* kLineInputDisable() const;
    GradientColor* kLineDock() const;
    GradientColor* kLineMenu() const;
    GradientColor* kLineTable() const;
    GradientColor* kLineInputNormal() const;
    GradientColor* kLineInputHover() const;
    GradientColor* kLineInputClick() const;
    GradientColor* kOrange1() const;
    GradientColor* kGreen1() const;
    GradientColor* kRed1() const;
    GradientColor* kDividerWhite() const;
    GradientColor* kLineSuccessNormal() const;
    GradientColor* kLineSuccessHover() const;
    GradientColor* kLineSuccessClick() const;
    GradientColor* kLineSuccessDisable() const;
    GradientColor* kLineErrorNormal() const;
    GradientColor* kLineErrorHover() const;
    GradientColor* kLineErrorClick() const;
    GradientColor* kLineWarningNormal() const;
    GradientColor* kLineWarningHover() const;
    GradientColor* kLineWarningClick() const;
    GradientColor* kLineWarningDisable() const;

    ShadowData* kShadowPrimaryActive() const;
    ShadowData* kShadowPrimaryInactive() const;
    ShadowData* kShadowSecondaryActive() const;
    ShadowData* kShadowSecondaryInactive() const;
    ShadowData* kShadowMin() const;
    ShadowData* kShadowMenu() const;
    ShadowData* kShadowComponent() const;

    GradientColor* kComponentSelectedNormal() const;
    GradientColor* kComponentSelectedHover() const;
    GradientColor* kComponentSelectedClick() const;
    GradientColor* kComponentSelectedDisable() const;
    GradientColor* kComponentSelectedAlphaNormal() const;
    GradientColor* kComponentSelectedAlphaHover() const;
    GradientColor* kComponentSelectedAlphaClick() const;
    GradientColor* kComponentSelectedAlphaDisable() const;
    GradientColor* kComponentInput() const;
    GradientColor* kComponentInputAlpha() const;

    GradientColor* kSelectAlphaWhiteDisable() const;
    GradientColor* kSelectAlphaWhiteHover() const;
    GradientColor* kSelectAlphaWhiteClick() const;
    GradientColor* kContainSecondaryInactive() const;
    GradientColor* kSelectAlphaWhite() const;
    GradientColor* kFontPlaceholderText() const;
    GradientColor* kLineErrorDisable() const;
    GradientColor* kMenu() const;

    GradientColor* kPanel() const;
    GradientColor* kBrand10() const;

    bool isDarkTheme();
private:
    DtTheme(QQmlEngine *engine, QObject *parent = nullptr);

    void proportySet();
    void addFuncPointer();
    void setTheme();
    void onThemeColorChanged();
    void onWindowRadiusChanged();
    void updateHighLightColor(QColor highlightColor);
    void windowTextActiveSet(QVariant value);
    void windowTextInactiveSet(QVariant value);
    void windowTextDisableSet(QVariant value);
    void buttonActiveSet(QVariant value);
    void buttonInactiveSet(QVariant value);
    void buttonDisableSet(QVariant value);
    void lightActiveSet(QVariant value);
    void lightInactiveSet(QVariant value);
    void lightDisableSet(QVariant value);
    void midLightActiveSet(QVariant value);
    void midLightInactiveSet(QVariant value);
    void midLightDisableSet(QVariant value);
    void darkActiveSet(QVariant value);
    void darkInactiveSet(QVariant value);
    void darkDisableSet(QVariant value);
    void midActiveSet(QVariant value);
    void midInactiveSet(QVariant value);
    void midDisableSet(QVariant value);
    void textActiveSet(QVariant value);
    void textInactiveSet(QVariant value);
    void textDisableSet(QVariant value);
    void brightTextActiveSet(QVariant value);
    void brightTextInactiveSet(QVariant value);
    void brightTextDisableSet(QVariant value);
    void buttonTextActiveSet(QVariant value);
    void buttonTextInactiveSet(QVariant value);
    void buttonTextDisableSet(QVariant value);
    void baseActiveSet(QVariant value);
    void baseInactiveSet(QVariant value);
    void baseDisableSet(QVariant value);
    void windowActiveSet(QVariant value);
    void windowInactiveSet(QVariant value);
    void windowDisableSet(QVariant value);
    void shadowActiveSet(QVariant value);
    void shadowInactiveSet(QVariant value);
    void shadowDisableSet(QVariant value);
    void highlightActiveSet(QVariant value);
    void highlightInactiveSet(QVariant value);
    void highlightDisableSet(QVariant value);
    void highlightedTextActiveSet(QVariant value);
    void highlightedTextInactiveSet(QVariant value);
    void highlightedTextDisableSet(QVariant value);
    void linkActiveSet(QVariant value);
    void linkInactiveSet(QVariant value);
    void linkDisableSet(QVariant value);
    void linkVisitedActiveSet(QVariant value);
    void linkVisitedInactiveSet(QVariant value);
    void linkVisitedDisableSet(QVariant value);
    void alternateBaseActiveSet(QVariant value);
    void alternateBaseInactiveSet(QVariant value);
    void alternateBaseDisableSet(QVariant value);
    void noRoleActiveSet(QVariant value);
    void noRoleInactiveSet(QVariant value);
    void noRoleDisableSet(QVariant value);
    void toolTipBaseActiveSet(QVariant value);
    void toolTipBaseInactiveSet(QVariant value);
    void toolTipBaseDisableSet(QVariant value);
    void toolTipTextActiveSet(QVariant value);
    void toolTipTextInactiveSet(QVariant value);
    void toolTipTextDisableSet(QVariant value);
    void placeholderTextActiveSet(QVariant value);
    void placeholderTextInactiveSet(QVariant value);
    void placeholderTextDisableSet(QVariant value);

    void kWhiteSet(QVariant value);
    void kBlackSet(QVariant value);
    void kGray0Set(QVariant value);
    void kGray1Set(QVariant value);
    void kGray2Set(QVariant value);
    void kGray3Set(QVariant value);
    void kGray4Set(QVariant value);
    void kGray5Set(QVariant value);
    void kGray6Set(QVariant value);
    void kGray7Set(QVariant value);
    void kGray8Set(QVariant value);
    void kGray9Set(QVariant value);
    void kGray10Set(QVariant value);
    void kGray11Set(QVariant value);
    void kGray12Set(QVariant value);
    void kGray13Set(QVariant value);
    void kGray14Set(QVariant value);
    void kGray15Set(QVariant value);
    void kGray16Set(QVariant value);
    void kGray17Set(QVariant value);
    void kGrayAlpha0Set(QVariant value);
    void kGrayAlpha1Set(QVariant value);
    void kGrayAlpha2Set(QVariant value);
    void kGrayAlpha3Set(QVariant value);
    void kGrayAlpha4Set(QVariant value);
    void kGrayAlpha5Set(QVariant value);
    void kGrayAlpha6Set(QVariant value);
    void kGrayAlpha7Set(QVariant value);
    void kGrayAlpha8Set(QVariant value);
    void kGrayAlpha9Set(QVariant value);
    void kGrayAlpha10Set(QVariant value);
    void kGrayAlpha11Set(QVariant value);
    void kGrayAlpha12Set(QVariant value);
    void kGrayAlpha13Set(QVariant value);

    void kFontStrongSet(QVariant value);
    void kFontPrimarySet(QVariant value);
    void kFontPrimaryDisableSet(QVariant value);
    void kFontSecondarySet(QVariant value);
    void kFontSecondaryDisableSet(QVariant value);
    void kFontWhiteSet(QVariant value);
    void kFontWhiteDisableSet(QVariant value);
    void kFontWhiteSecondarySet(QVariant value);
    void kFontWhiteSecondaryDisableSet(QVariant value);

    void kBrandNormalSet(QVariant value);
    void kBrandHoverSet(QVariant value);
    void kBrandClickSet(QVariant value);
    void kBrandFocusSet(QVariant value);
    void kSuccessNormalSet(QVariant value);
    void kSuccessHoverSet(QVariant value);
    void kSuccessClickSet(QVariant value);
    void kWarningNormalSet(QVariant value);
    void kWarningHoverSet(QVariant value);
    void kWarningClickSet(QVariant value);
    void kErrorNormalSet(QVariant value);
    void kErrorHoverSet(QVariant value);
    void kErrorClickSet(QVariant value);
    void kBrand1Set(QVariant value);
    void kBrand2Set(QVariant value);
    void kBrand3Set(QVariant value);
    void kBrand4Set(QVariant value);
    void kBrand5Set(QVariant value);
    void kBrand6Set(QVariant value);
    void kBrand7Set(QVariant value);

    void kContainHoverSet(QVariant value);
    void kContainClickSet(QVariant value);
    void kContainGeneralNormalSet(QVariant value);
    void kContainSecondaryNormalSet(QVariant value);
    void kContainSecondaryAlphaNormalSet(QVariant value);
    void kComponentNormalSet(QVariant value);
    void kComponentHoverSet(QVariant value);
    void kComponentClickSet(QVariant value);
    void kComponentDisableSet(QVariant value);
    void kComponentAlphaNormalSet(QVariant value);
    void kComponentAlphaHoverSet(QVariant value);
    void kComponentAlphaClickSet(QVariant value);
    void kComponentAlphaDisableSet(QVariant value);
    void kLineWindowActiveSet(QVariant value);
    void kLineComponentNormalSet(QVariant value);
    void kLineComponentHoverSet(QVariant value);
    void kLineComponentClickSet(QVariant value);
    void kLineComponentDisableSet(QVariant value);
    void kLineBrandNormalSet(QVariant value);
    void kLineBrandHoverSet(QVariant value);
    void kLineBrandClickSet(QVariant value);
    void kLineBrandDisableSet(QVariant value);
    void kModalMaskSet(QVariant value);
    void normalLineSet(QVariant value);
    void focusLineSet(QVariant value);
    void kRadiusMinSet(QVariant value);
    void kRadiusNormalSet(QVariant value);
    void kRadiusMaxSet(QVariant value);
    void kRadiusMenuSet(QVariant value);
    void kRadiusWindowSet(QVariant value);
    void kMarginMinSet(QVariant value);
    void kMarginNormalSet(QVariant value);
    void kMarginBigSet(QVariant value);
    void kMarginWindowSet(QVariant value);
    void kMarginComponentSet(QVariant value);
    void kPaddingMinLeftSet(QVariant value);
    void kPaddingMinTopSet(QVariant value);
    void kPaddingMinRightSet(QVariant value);
    void kPaddingMinBottomSet(QVariant value);
    void kPaddingNormalLeftSet(QVariant value);
    void kPaddingNormalTopSet(QVariant value);
    void kPaddingNormalRightSet(QVariant value);
    void kPaddingNormalBottomSet(QVariant value);
    void kPadding8LeftSet(QVariant value);
    void kPadding8TopSet(QVariant value);
    void kPadding8RightSet(QVariant value);
    void kPadding8BottomSet(QVariant value);
    void kPaddingWindowLeftSet(QVariant value);
    void kPaddingWindowTopSet(QVariant value);
    void kPaddingWindowRightSet(QVariant value);
    void kPaddingWindowBottomSet(QVariant value);

    void kErrorDisableSet(QVariant value);
    void kContainGeneralAlphaNormalSet(QVariant value);
    void kSuccessDisableSet(QVariant value);
    void kLineWindowInactiveSet(QVariant value);
    void kContainAlphaClickSet(QVariant value);
    void kWarningDisableSet(QVariant value);
    void kDividerSet(QVariant value);
    void kContainAlphaHoverSet(QVariant value);
    void kBrandDisableSet(QVariant value);
    void kLineDisableSet(QVariant value);
    void kLineNormalSet(QVariant value);

    void kBrand8Set(QVariant value);
    void kBrand9Set(QVariant value);
    void kLineSelectboxNormalSet(QVariant value);
    void kLineSelectboxHoverSet(QVariant value);
    void kLineSelectboxClickSet(QVariant value);
    void kLineSelectboxSelectedSet(QVariant value);
    void kLineSelectboxDisableSet(QVariant value);
    void kFontWhitePlaceholderTextDisableSet(QVariant value);
    void kContainGeneralInactiveSet(QVariant value);
    void kFontPlaceholderTextDisableSet(QVariant value);
    void kFontWhitePlaceholderTextSet(QVariant value);
    void kLineInputDisableSet(QVariant value);
    void kLineDockSet(QVariant value);
    void kLineMenuSet(QVariant value);
    void kLineTableSet(QVariant value);
    void kLineInputNormalSet(QVariant value);
    void kLineInputHoverSet(QVariant value);
    void kLineInputClickSet(QVariant value);
    void kOrange1Set(QVariant value);
    void kGreen1Set(QVariant value);
    void kRed1Set(QVariant value);
    void kDividerWhiteSet(QVariant value);
    void kLineSuccessNormalSet(QVariant value);
    void kLineSuccessHoverSet(QVariant value);
    void kLineSuccessClickSet(QVariant value);
    void kLineSuccessDisableSet(QVariant value);
    void kLineErrorNormalSet(QVariant value);
    void kLineErrorHoverSet(QVariant value);
    void kLineErrorClickSet(QVariant value);
    void kLineWarningNormalSet(QVariant value);
    void kLineWarningHoverSet(QVariant value);
    void kLineWarningClickSet(QVariant value);
    void kLineWarningDisableSet(QVariant value);

    void kShadowPrimaryActiveSet(QVariant value);
    void kShadowPrimaryInactiveSet(QVariant value);
    void kShadowSecondaryActiveSet(QVariant value);
    void kShadowSecondaryInactiveSet(QVariant value);
    void kShadowMinSet(QVariant value);
    void kShadowMenuSet(QVariant value);
    void kShadowComponentSet(QVariant value);

    void kComponentSelectedNormalSet(QVariant value);
    void kComponentSelectedHoverSet(QVariant value);
    void kComponentSelectedClickSet(QVariant value);
    void kComponentSelectedDisableSet(QVariant value);
    void kComponentSelectedAlphaNormalSet(QVariant value);
    void kComponentSelectedAlphaHoverSet(QVariant value);
    void kComponentSelectedAlphaClickSet(QVariant value);
    void kComponentSelectedAlphaDisableSet(QVariant value);
    void kComponentInputSet(QVariant value);
    void kComponentInputAlphaSet(QVariant value);

    void kSelectAlphaWhiteDisableSet(QVariant value);
    void kSelectAlphaWhiteHoverSet(QVariant value);
    void kSelectAlphaWhiteClickSet(QVariant value);
    void kMenuSet(QVariant value);
    void kLineErrorDisableSet(QVariant value);
    void kContainSecondaryInactiveSet(QVariant value);
    void kSelectAlphaWhiteSet(QVariant value);
    void kFontPlaceholderTextSet(QVariant value);

    void kPanelSet(QVariant value);
    void kBrand10Set(QVariant value);
Q_SIGNALS:
    void windowTextActiveChanged();
    void windowTextInactiveChanged();
    void windowTextDisableChanged();
    void buttonActiveChanged();
    void buttonInactiveChanged();
    void buttonDisableChanged();
    void lightActiveChanged();
    void lightInactiveChanged();
    void lightDisableChanged();
    void midLightActiveChanged();
    void midLightInactiveChanged();
    void midLightDisableChanged();
    void darkActiveChanged();
    void darkInactiveChanged();
    void darkDisableChanged();
    void midActiveChanged();
    void midInactiveChanged();
    void midDisableChanged();
    void textActiveChanged();
    void textInactiveChanged();
    void textDisableChanged();
    void brightTextActiveChanged();
    void brightTextInactiveChanged();
    void brightTextDisableChanged();
    void buttonTextActiveChanged();
    void buttonTextInactiveChanged();
    void buttonTextDisableChanged();
    void baseActiveChanged();
    void baseInactiveChanged();
    void baseDisableChanged();
    void windowActiveChanged();
    void windowInactiveChanged();
    void windowDisableChanged();
    void shadowActiveChanged();
    void shadowInactiveChanged();
    void shadowDisableChanged();
    void highlightActiveChanged();
    void highlightInactiveChanged();
    void highlightDisableChanged();
    void highlightedTextActiveChanged();
    void highlightedTextInactiveChanged();
    void highlightedTextDisableChanged();
    void linkActiveChanged();
    void linkInactiveChanged();
    void linkDisableChanged();
    void linkVisitedActiveChanged();
    void linkVisitedInactiveChanged();
    void linkVisitedDisableChanged();
    void alternateBaseActiveChanged();
    void alternateBaseInactiveChanged();
    void alternateBaseDisableChanged();
    void noRoleActiveChanged();
    void noRoleInactiveChanged();
    void noRoleDisableChanged();
    void toolTipBaseActiveChanged();
    void toolTipBaseInactiveChanged();
    void toolTipBaseDisableChanged();
    void toolTipTextActiveChanged();
    void toolTipTextInactiveChanged();
    void toolTipTextDisableChanged();
    void placeholderTextActiveChanged();
    void placeholderTextInactiveChanged();
    void placeholderTextDisableChanged();

    void kWhiteChanged();
    void kBlackChanged();
    void kGray0Changed();
    void kGray1Changed();
    void kGray2Changed();
    void kGray3Changed();
    void kGray4Changed();
    void kGray5Changed();
    void kGray6Changed();
    void kGray7Changed();
    void kGray8Changed();
    void kGray9Changed();
    void kGray10Changed();
    void kGray11Changed();
    void kGray12Changed();
    void kGray13Changed();
    void kGray14Changed();
    void kGray15Changed();
    void kGray16Changed();
    void kGray17Changed();
    void kGrayAlpha0Changed();
    void kGrayAlpha1Changed();
    void kGrayAlpha2Changed();
    void kGrayAlpha3Changed();
    void kGrayAlpha4Changed();
    void kGrayAlpha5Changed();
    void kGrayAlpha6Changed();
    void kGrayAlpha7Changed();
    void kGrayAlpha8Changed();
    void kGrayAlpha9Changed();
    void kGrayAlpha10Changed();
    void kGrayAlpha11Changed();
    void kGrayAlpha12Changed();
    void kGrayAlpha13Changed();

    void kFontStrongChanged();
    void kFontPrimaryChanged();
    void kFontPrimaryDisableChanged();
    void kFontSecondaryChanged();
    void kFontSecondaryDisableChanged();
    void kFontWhiteChanged();
    void kFontWhiteDisableChanged();
    void kFontWhiteSecondaryChanged();
    void kFontWhiteSecondaryDisableChanged();

    void kBrandNormalChanged();
    void kBrandHoverChanged();
    void kBrandClickChanged();
    void kBrandFocusChanged();
    void kSuccessNormalChanged();
    void kSuccessHoverChanged();
    void kSuccessClickChanged();
    void kWarningNormalChanged();
    void kWarningHoverChanged();
    void kWarningClickChanged();
    void kErrorNormalChanged();
    void kErrorHoverChanged();
    void kErrorClickChanged();
    void kBrand1Changed();
    void kBrand2Changed();
    void kBrand3Changed();
    void kBrand4Changed();
    void kBrand5Changed();
    void kBrand6Changed();
    void kBrand7Changed();

    void kContainHoverChanged();
    void kContainClickChanged();
    void kContainGeneralNormalChanged();
    void kContainSecondaryNormalChanged();
    void kContainSecondaryAlphaNormalChanged();
    void kComponentNormalChanged();
    void kComponentHoverChanged();
    void kComponentClickChanged();
    void kComponentDisableChanged();
    void kComponentAlphaNormalChanged();
    void kComponentAlphaHoverChanged();
    void kComponentAlphaClickChanged();
    void kComponentAlphaDisableChanged();
    void kLineWindowActiveChanged();
    void kLineComponentNormalChanged();
    void kLineComponentHoverChanged();
    void kLineComponentClickChanged();
    void kLineComponentDisableChanged();
    void kLineBrandNormalChanged();
    void kLineBrandHoverChanged();
    void kLineBrandClickChanged();
    void kLineBrandDisableChanged();
    void kModalMaskChanged();
    void normalLineChanged();
    void focusLineChanged();
    void kRadiusMinChanged();
    void kRadiusNormalChanged();
    void kRadiusMaxChanged();
    void kRadiusMenuChanged();
    void kRadiusWindowChanged();
    void kMarginMinChanged();
    void kMarginNormalChanged();
    void kMarginBigChanged();
    void kMarginWindowChanged();
    void kMarginComponentChanged();
    void kPaddingMinLeftChanged();
    void kPaddingMinTopChanged();
    void kPaddingMinRightChanged();
    void kPaddingMinBottomChanged();
    void kPaddingNormalLeftChanged();
    void kPaddingNormalTopChanged();
    void kPaddingNormalRightChanged();
    void kPaddingNormalBottomChanged();
    void kPadding8LeftChanged();
    void kPadding8TopChanged();
    void kPadding8RightChanged();
    void kPadding8BottomChanged();
    void kPaddingWindowLeftChanged();
    void kPaddingWindowTopChanged();
    void kPaddingWindowRightChanged();
    void kPaddingWindowBottomChanged();
    void transparencyChanged();

    void kErrorDisableChanged();
    void kContainGeneralAlphaNormalChanged();
    void kSuccessDisableChanged();
    void kLineWindowInactiveChanged();
    void kContainAlphaClickChanged();
    void kWarningDisableChanged();
    void kDividerChanged();
    void kContainAlphaHoverChanged();
    void kBrandDisableChanged();
    void kLineDisableChanged();
    void kLineNormalChanged();
    void fontSizeChanged();
    void fontFamilyChanged();
    void iconThemeNameChanged();
    void layoutDirectionChanged();
    void themeNameChanged();

    void kBrand8Changed();
    void kBrand9Changed();
    void kLineSelectboxNormalChanged();
    void kLineSelectboxHoverChanged();
    void kLineSelectboxClickChanged();
    void kLineSelectboxSelectedChanged();
    void kLineSelectboxDisableChanged();
    void kFontWhitePlaceholderTextDisableChanged();
    void kContainGeneralInactiveChanged();
    void kFontPlaceholderTextDisableChanged();
    void kFontWhitePlaceholderTextChanged();
    void kLineInputDisableChanged();
    void kLineDockChanged();
    void kLineMenuChanged();
    void kLineTableChanged();
    void kLineInputNormalChanged();
    void kLineInputHoverChanged();
    void kLineInputClickChanged();
    void kOrange1Changed();
    void kGreen1Changed();
    void kRed1Changed();
    void kDividerWhiteChanged();
    void kLineSuccessNormalChanged();
    void kLineSuccessHoverChanged();
    void kLineSuccessClickChanged();
    void kLineSuccessDisableChanged();
    void kLineErrorNormalChanged();
    void kLineErrorHoverChanged();
    void kLineErrorClickChanged();
    void kLineWarningNormalChanged();
    void kLineWarningHoverChanged();
    void kLineWarningClickChanged();
    void kLineWarningDisableChanged();

    void kShadowPrimaryActiveChanged();
    void kShadowPrimaryInactiveChanged();
    void kShadowSecondaryActiveChanged();
    void kShadowSecondaryInactiveChanged();
    void kShadowMinChanged();
    void kShadowMenuChanged();
    void kShadowComponentChanged();

    void kComponentSelectedNormalChanged();
    void kComponentSelectedHoverChanged();
    void kComponentSelectedClickChanged();
    void kComponentSelectedDisableChanged();
    void kComponentSelectedAlphaNormalChanged();
    void kComponentSelectedAlphaHoverChanged();
    void kComponentSelectedAlphaClickChanged();
    void kComponentSelectedAlphaDisableChanged();
    void kComponentInputChanged();
    void kComponentInputAlphaChanged();

    void kSelectAlphaWhiteDisableChanged();
    void kSelectAlphaWhiteHoverChanged();
    void kSelectAlphaWhiteClickChanged();
    void kMenuChanged();
    void kLineErrorDisableChanged();
    void kContainSecondaryInactiveChanged();
    void kSelectAlphaWhiteChanged();
    void kFontPlaceholderTextChanged();

    void kPanelChanged();
    void kBrand10Changed();
private:
    DtThemePrivate *d = nullptr;
};
} // UkuiQuick

#endif //DT_THEME_H
