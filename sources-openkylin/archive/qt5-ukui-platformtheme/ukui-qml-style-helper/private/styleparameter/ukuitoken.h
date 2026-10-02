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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef UKUITOKEN_H
#define UKUITOKEN_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIToken : public QQuickItem
{
    Q_OBJECT

    // ========== 基础调色板属性组 ==========
    Q_PROPERTY(QBrush alternateBaseActive READ alternateBaseActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush alternateBaseDisable READ alternateBaseDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush alternateBaseInactive READ alternateBaseInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush baseActive READ baseActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush baseDisable READ baseDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush baseInactive READ baseInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush brightTextActive READ brightTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush brightTextDisable READ brightTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush brightTextInactive READ brightTextInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonActive READ buttonActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonDisable READ buttonDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonInactive READ buttonInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonTextActive READ buttonTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonTextDisable READ buttonTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush buttonTextInactive READ buttonTextInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush darkActive READ darkActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush darkDisable READ darkDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush darkInactive READ darkInactive NOTIFY tokenChanged)
    Q_PROPERTY(int focusline READ focusline NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightActive READ highLightActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightDisable READ highLightDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightInactive READ highLightInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightedTextActive READ highLightedTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightedTextDisable READ highLightedTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush highLightedTextInactive READ highLightedTextInactive NOTIFY tokenChanged)

    // ========== 品牌色/组件色属性组 ==========
    Q_PROPERTY(QBrush kBlack READ kBlack NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand1 READ kBrand1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand2 READ kBrand2 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand3 READ kBrand3 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand4 READ kBrand4 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand5 READ kBrand5 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand6 READ kBrand6 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand7 READ kBrand7 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand8 READ kBrand8 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrand9 READ kBrand9 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrandClick READ kBrandClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrandDisable READ kBrandDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrandFocus READ kBrandFocus NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrandHover READ kBrandHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kBrandNormal READ kBrandNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentAlphaClick READ kComponentAlphaClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentAlphaDisable READ kComponentAlphaDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentAlphaHover READ kComponentAlphaHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentAlphaNormal READ kComponentAlphaNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentClick READ kComponentClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentDisable READ kComponentDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentHover READ kComponentHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentNormal READ kComponentNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedAlphaDisable READ kComponentSelectedAlphaDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedAlphaNormal READ kComponentSelectedAlphaNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedAlphaHover READ kComponentSelectedAlphaHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedAlphaClick READ kComponentSelectedAlphaClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentInput READ kComponentInput NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentInputAlpha READ kComponentInputAlpha NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedDisable READ kComponentSelectedDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedNormal READ kComponentSelectedNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedHover READ kComponentSelectedHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kComponentSelectedClick READ kComponentSelectedClick NOTIFY tokenChanged)

    // ========== 容器/分割线属性组 ==========
    Q_PROPERTY(QBrush kContainAlphaClick READ kContainAlphaClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainAlphaHover READ kContainAlphaHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainClick READ kContainClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainGeneralAlphaNormal READ kContainGeneralAlphaNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainGeneralInactive READ kContainGeneralInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainGeneralNormal READ kContainGeneralNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainHover READ kContainHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainSecondaryAlphaNormal READ kContainSecondaryAlphaNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainSecondaryInactive READ kContainSecondaryInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kContainSecondaryNormal READ kContainSecondaryNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kDivider READ kDivider NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kDividerWhite READ kDividerWhite NOTIFY tokenChanged)

    // ========== 状态色（错误/成功/警告）属性组 ==========
    Q_PROPERTY(QBrush kErrorClick READ kErrorClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kErrorDisable READ kErrorDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kErrorHover READ kErrorHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kErrorNormal READ kErrorNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSuccessClick READ kSuccessClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSuccessDisable READ kSuccessDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSuccessHover READ kSuccessHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSuccessNormal READ kSuccessNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kWarningClick READ kWarningClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kWarningDisable READ kWarningDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kWarningHover READ kWarningHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kWarningNormal READ kWarningNormal NOTIFY tokenChanged)

    // ========== 字体色属性组 ==========
    Q_PROPERTY(QBrush kFontPlaceholdertextDisable READ kFontPlaceholdertextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontPlaceholdertext READ kFontPlaceholdertext NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontPrimaryDisable READ kFontPrimaryDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontPrimary READ kFontPrimary NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontSecondaryDisable READ kFontSecondaryDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontSecondary READ kFontSecondary NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontStrong READ kFontStrong NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhiteDisable READ kFontWhiteDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhitePlaceholdertextDisable READ kFontWhitePlaceholdertextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhiteplaceholdertext READ kFontWhiteplaceholdertext NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhite READ kFontWhite NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhiteSecondaryDisable READ kFontWhiteSecondaryDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kFontWhiteSecondary READ kFontWhiteSecondary NOTIFY tokenChanged)

    // ========== 灰色系属性组 ==========
    Q_PROPERTY(QBrush kGray0 READ kGray0 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray1 READ kGray1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray2 READ kGray2 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray3 READ kGray3 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray4 READ kGray4 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray5 READ kGray5 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray6 READ kGray6 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray7 READ kGray7 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray8 READ kGray8 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray9 READ kGray9 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray10 READ kGray10 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray11 READ kGray11 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray12 READ kGray12 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray13 READ kGray13 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray14 READ kGray14 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray15 READ kGray15 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray16 READ kGray16 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGray17 READ kGray17 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha0 READ kGrayAlpha0 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha1 READ kGrayAlpha1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha2 READ kGrayAlpha2 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha3 READ kGrayAlpha3 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha4 READ kGrayAlpha4 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha5 READ kGrayAlpha5 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha6 READ kGrayAlpha6 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha7 READ kGrayAlpha7 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha8 READ kGrayAlpha8 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha9 READ kGrayAlpha9 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha10 READ kGrayAlpha10 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha11 READ kGrayAlpha11 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha12 READ kGrayAlpha12 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kGrayAlpha13 READ kGrayAlpha13 NOTIFY tokenChanged)

    // ========== 特殊色/线条属性组 ==========
    Q_PROPERTY(QBrush kGreen1 READ kGreen1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kOrange1 READ kOrange1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kRed1 READ kRed1 NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineBrandClick READ kLineBrandClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineBrandDisable READ kLineBrandDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineBrandHover READ kLineBrandHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineBrandNormal READ kLineBrandNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineComponentClick READ kLineComponentClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineComponentDisable READ kLineComponentDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineComponentHover READ kLineComponentHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineComponentNormal READ kLineComponentNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineDisable READ kLineDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineDock READ kLineDock NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineErrorClick READ kLineErrorClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineErrorDisable READ kLineErrorDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineErrorHover READ kLineErrorHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineErrorNormal READ kLineErrorNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineInputClick READ kLineInputClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineInputDisable READ kLineInputDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineInputHover READ kLineInputHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineInputNormal READ kLineInputNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineMenu READ kLineMenu NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineNormal READ kLineNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSelectboxClick READ kLineSelectboxClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSelectboxDisable READ kLineSelectboxDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSelectboxHover READ kLineSelectboxHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSelectboxNormal READ kLineSelectboxNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSelectboxSelected READ kLineSelectboxSelected NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSuccessClick READ kLineSuccessClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSuccessDisable READ kLineSuccessDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSuccessHover READ kLineSuccessHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineSuccessNormal READ kLineSuccessNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineTable READ kLineTable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWarningClick READ kLineWarningClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWarningDisable READ kLineWarningDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWarningHover READ kLineWarningHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWarningNormal READ kLineWarningNormal NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWindowActive READ kLineWindowActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kLineWindowInactive READ kLineWindowInactive NOTIFY tokenChanged)

    // ========== 菜单/阴影属性组 ==========
    Q_PROPERTY(QBrush kMenu READ kMenu NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kModalmask READ kModalmask NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSelectAlphaWhiteClick READ kSelectAlphaWhiteClick NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSelectAlphaWhiteDisable READ kSelectAlphaWhiteDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSelectAlphaWhiteHover READ kSelectAlphaWhiteHover NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kSelectAlphaWhite READ kSelectAlphaWhite NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowComponent READ kshadowComponent NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowMenu READ kshadowMenu NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowMin READ kshadowMin NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowPrimaryActive READ kshadowPrimaryActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowPrimaryInactive READ kshadowPrimaryInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowSecondaryActive READ kshadowSecondaryActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kshadowSecondaryInactive READ kshadowSecondaryInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush kWhite READ kWhite NOTIFY tokenChanged)

    // ========== 通用状态属性组 ==========
    Q_PROPERTY(QBrush lightActive READ lightActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush lightDisable READ lightDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush lightInactive READ lightInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkActive READ linkActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkDisable READ linkDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkInactive READ linkInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkVisitedActive READ linkVisitedActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkVisitedDisable READ linkVisitedDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush linkVisitedInactive READ linkVisitedInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midActive READ midActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midDisable READ midDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midInactive READ midInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midlightActive READ midlightActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midlightDisable READ midlightDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush midlightInactive READ midlightInactive NOTIFY tokenChanged)
    Q_PROPERTY(int normalline READ normalline NOTIFY tokenChanged)
    Q_PROPERTY(QBrush noRoleActive READ noRoleActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush noRoleDisable READ noRoleDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush noRoleInactive READ noRoleInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush placeholderTextActive READ placeholderTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush placeholderTextDisable READ placeholderTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush placeholderTextInactive READ placeholderTextInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush shadow READ shadow NOTIFY tokenChanged)
    Q_PROPERTY(QBrush shadowActive READ shadowActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush shadowDisable READ shadowDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush shadowInactive READ shadowInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush textActive READ textActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush textDisable READ textDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush textInactive READ textInactive NOTIFY tokenChanged)

    // ========== 提示框/窗口属性组 ==========
    Q_PROPERTY(QBrush toolTipBaseActive READ toolTipBaseActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush toolTipBaseDisable READ toolTipBaseDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush toolTipBaseInactive READ toolTipBaseInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush toolTipTextActive READ toolTipTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush toolTipTextDisable READ toolTipTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush toolTipTextInactive READ toolTipTextInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowActive READ windowActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowDisable READ windowDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowInactive READ windowInactive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowTextActive READ windowTextActive NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowTextDisable READ windowTextDisable NOTIFY tokenChanged)
    Q_PROPERTY(QBrush windowTextInactive READ windowTextInactive NOTIFY tokenChanged)

    // ========== 圆角尺寸属性组 ==========
    Q_PROPERTY(int kradiusMax READ kradiusMax NOTIFY tokenChanged)
    Q_PROPERTY(int kradiusMenu READ kradiusMenu NOTIFY tokenChanged)
    Q_PROPERTY(int kradiusMin READ kradiusMin NOTIFY tokenChanged)
    Q_PROPERTY(int kradiusNormal READ kradiusNormal NOTIFY tokenChanged)
    Q_PROPERTY(int kradiusWindow READ kradiusWindow NOTIFY tokenChanged)

public:
    explicit UKUIToken(QQuickItem *parent = nullptr);
    ~UKUIToken();

    static UKUIToken* qmlAttachedProperties(QObject* parent);
    void initDTBrush(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    // ========== 基础调色板属性 - 读写方法 ==========
    const QBrush &alternateBaseActive() const;
    const QBrush &alternateBaseDisable() const;
    const QBrush &alternateBaseInactive() const;
    const QBrush &baseActive() const;
    const QBrush &baseDisable() const;
    const QBrush &baseInactive() const;
    const QBrush &brightTextActive() const;
    const QBrush &brightTextDisable() const;
    const QBrush &brightTextInactive() const;
    const QBrush &buttonActive() const;
    const QBrush &buttonDisable() const;
    const QBrush &buttonInactive() const;
    const QBrush &buttonTextActive() const;
    const QBrush &buttonTextDisable() const;
    const QBrush &buttonTextInactive() const;
    const QBrush &darkActive() const;
    const QBrush &darkDisable() const;
    const QBrush &darkInactive() const;
    int focusline() const;
    const QBrush &highLightActive() const;
    const QBrush &highLightDisable() const;
    const QBrush &highLightInactive() const;
    const QBrush &highLightedTextActive() const;
    const QBrush &highLightedTextDisable() const;
    const QBrush &highLightedTextInactive() const;

    // ========== 品牌色/组件色属性 - 读写方法 ==========
    const QBrush &kBlack() const;
    const QBrush &kBrand1() const;
    const QBrush &kBrand2() const;
    const QBrush &kBrand3() const;
    const QBrush &kBrand4() const;
    const QBrush &kBrand5() const;
    const QBrush &kBrand6() const;
    const QBrush &kBrand7() const;
    const QBrush &kBrand8() const;
    const QBrush &kBrand9() const;
    const QBrush &kBrandClick() const;
    const QBrush &kBrandDisable() const;
    const QBrush &kBrandFocus() const;
    const QBrush &kBrandHover() const;
    const QBrush &kBrandNormal() const;
    const QBrush &kComponentAlphaClick() const;
    const QBrush &kComponentAlphaDisable() const;
    const QBrush &kComponentAlphaHover() const;
    const QBrush &kComponentAlphaNormal() const;
    const QBrush &kComponentClick() const;
    const QBrush &kComponentDisable() const;
    const QBrush &kComponentHover() const;
    const QBrush &kComponentNormal() const;
    const QBrush &kComponentSelectedAlphaDisable() const;
    const QBrush &kComponentSelectedAlphaNormal() const;
    const QBrush &kComponentSelectedAlphaHover() const;
    const QBrush &kComponentSelectedAlphaClick() const;
    const QBrush &kComponentInput() const;
    const QBrush &kComponentInputAlpha() const;
    const QBrush &kComponentSelectedDisable() const;
    const QBrush &kComponentSelectedNormal() const;
    const QBrush &kComponentSelectedHover() const;
    const QBrush &kComponentSelectedClick() const;

    // ========== 容器/分割线属性 - 读写方法 ==========
    const QBrush &kContainAlphaClick() const;
    const QBrush &kContainAlphaHover() const;
    const QBrush &kContainClick() const;
    const QBrush &kContainGeneralAlphaNormal() const;
    const QBrush &kContainGeneralInactive() const;
    const QBrush &kContainGeneralNormal() const;
    const QBrush &kContainHover() const;
    const QBrush &kContainSecondaryAlphaNormal() const;
    const QBrush &kContainSecondaryInactive() const;
    const QBrush &kContainSecondaryNormal() const;
    const QBrush &kDivider() const;
    const QBrush &kDividerWhite() const;

    // ========== 状态色属性 - 读写方法 ==========
    const QBrush &kErrorClick() const;
    const QBrush &kErrorDisable() const;
    const QBrush &kErrorHover() const;
    const QBrush &kErrorNormal() const;
    const QBrush &kSuccessClick() const;
    const QBrush &kSuccessDisable() const;
    const QBrush &kSuccessHover() const;
    const QBrush &kSuccessNormal() const;
    const QBrush &kWarningClick() const;
    const QBrush &kWarningDisable() const;
    const QBrush &kWarningHover() const;
    const QBrush &kWarningNormal() const;

    // ========== 字体色属性 - 读写方法 ==========
    const QBrush &kFontPlaceholdertextDisable() const;
    const QBrush &kFontPlaceholdertext() const;
    const QBrush &kFontPrimaryDisable() const;
    const QBrush &kFontPrimary() const;
    const QBrush &kFontSecondaryDisable() const;
    const QBrush &kFontSecondary() const;
    const QBrush &kFontStrong() const;
    const QBrush &kFontWhiteDisable() const;
    const QBrush &kFontWhitePlaceholdertextDisable() const;
    const QBrush &kFontWhiteplaceholdertext() const;
    const QBrush &kFontWhite() const;
    const QBrush &kFontWhiteSecondaryDisable() const;
    const QBrush &kFontWhiteSecondary() const;

    // ========== 灰色系属性 - 读写方法 ==========
    const QBrush &kGray0() const;
    const QBrush &kGray1() const;
    const QBrush &kGray2() const;
    const QBrush &kGray3() const;
    const QBrush &kGray4() const;
    const QBrush &kGray5() const;
    const QBrush &kGray6() const;
    const QBrush &kGray7() const;
    const QBrush &kGray8() const;
    const QBrush &kGray9() const;
    const QBrush &kGray10() const;
    const QBrush &kGray11() const;
    const QBrush &kGray12() const;
    const QBrush &kGray13() const;
    const QBrush &kGray14() const;
    const QBrush &kGray15() const;
    const QBrush &kGray16() const;
    const QBrush &kGray17() const;
    const QBrush &kGrayAlpha0() const;
    const QBrush &kGrayAlpha1() const;
    const QBrush &kGrayAlpha2() const;
    const QBrush &kGrayAlpha3() const;
    const QBrush &kGrayAlpha4() const;
    const QBrush &kGrayAlpha5() const;
    const QBrush &kGrayAlpha6() const;
    const QBrush &kGrayAlpha7() const;
    const QBrush &kGrayAlpha8() const;
    const QBrush &kGrayAlpha9() const;
    const QBrush &kGrayAlpha10() const;
    const QBrush &kGrayAlpha11() const;
    const QBrush &kGrayAlpha12() const;
    const QBrush &kGrayAlpha13() const;

    // ========== 特殊色/线条属性 - 读写方法 ==========
    const QBrush &kGreen1() const;
    const QBrush &kOrange1() const;
    const QBrush &kRed1() const;
    const QBrush &kLineBrandClick() const;
    const QBrush &kLineBrandDisable() const;
    const QBrush &kLineBrandHover() const;
    const QBrush &kLineBrandNormal() const;
    const QBrush &kLineComponentClick() const;
    const QBrush &kLineComponentDisable() const;
    const QBrush &kLineComponentHover() const;
    const QBrush &kLineComponentNormal() const;
    const QBrush &kLineDisable() const;
    const QBrush &kLineDock() const;
    const QBrush &kLineErrorClick() const;
    const QBrush &kLineErrorDisable() const;
    const QBrush &kLineErrorHover() const;
    const QBrush &kLineErrorNormal() const;
    const QBrush &kLineInputClick() const;
    const QBrush &kLineInputDisable() const;
    const QBrush &kLineInputHover() const;
    const QBrush &kLineInputNormal() const;
    const QBrush &kLineMenu() const;
    const QBrush &kLineNormal() const;
    const QBrush &kLineSelectboxClick() const;
    const QBrush &kLineSelectboxDisable() const;
    const QBrush &kLineSelectboxHover() const;
    const QBrush &kLineSelectboxNormal() const;
    const QBrush &kLineSelectboxSelected() const;
    const QBrush &kLineSuccessClick() const;
    const QBrush &kLineSuccessDisable() const;
    const QBrush &kLineSuccessHover() const;
    const QBrush &kLineSuccessNormal() const;
    const QBrush &kLineTable() const;
    const QBrush &kLineWarningClick() const;
    const QBrush &kLineWarningDisable() const;
    const QBrush &kLineWarningHover() const;
    const QBrush &kLineWarningNormal() const;
    const QBrush &kLineWindowActive() const;
    const QBrush &kLineWindowInactive() const;

    // ========== 菜单/阴影属性 - 读写方法 ==========
    const QBrush &kMenu() const;
    const QBrush &kModalmask() const;
    const QBrush &kSelectAlphaWhiteClick() const;
    const QBrush &kSelectAlphaWhiteDisable() const;
    const QBrush &kSelectAlphaWhiteHover() const;
    const QBrush &kSelectAlphaWhite() const;
    const QBrush &kshadowComponent() const;
    const QBrush &kshadowMenu() const;
    const QBrush &kshadowMin() const;
    const QBrush &kshadowPrimaryActive() const;
    const QBrush &kshadowPrimaryInactive() const;
    const QBrush &kshadowSecondaryActive() const;
    const QBrush &kshadowSecondaryInactive() const;
    const QBrush &kWhite() const;

    // ========== 通用状态属性 - 读写方法 ==========
    const QBrush &lightActive() const;
    const QBrush &lightDisable() const;
    const QBrush &lightInactive() const;
    const QBrush &linkActive() const;
    const QBrush &linkDisable() const;
    const QBrush &linkInactive() const;
    const QBrush &linkVisitedActive() const;
    const QBrush &linkVisitedDisable() const;
    const QBrush &linkVisitedInactive() const;
    const QBrush &midActive() const;
    const QBrush &midDisable() const;
    const QBrush &midInactive() const;
    const QBrush &midlightActive() const;
    const QBrush &midlightDisable() const;
    const QBrush &midlightInactive() const;
    int normalline() const;
    const QBrush &noRoleActive() const;
    const QBrush &noRoleDisable() const;
    const QBrush &noRoleInactive() const;
    const QBrush &placeholderTextActive() const;
    const QBrush &placeholderTextDisable() const;
    const QBrush &placeholderTextInactive() const;
    const QBrush &shadow() const;
    const QBrush &shadowActive() const;
    const QBrush &shadowDisable() const;
    const QBrush &shadowInactive() const;
    const QBrush &textActive() const;
    const QBrush &textDisable() const;
    const QBrush &textInactive() const;

    // ========== 提示框/窗口属性 - 读写方法 ==========
    const QBrush &toolTipBaseActive() const;
    const QBrush &toolTipBaseDisable() const;
    const QBrush &toolTipBaseInactive() const;
    const QBrush &toolTipTextActive() const;
    const QBrush &toolTipTextDisable() const;
    const QBrush &toolTipTextInactive() const;
    const QBrush &windowActive() const;
    const QBrush &windowDisable() const;
    const QBrush &windowInactive() const;
    const QBrush &windowTextActive() const;
    const QBrush &windowTextDisable() const;
    const QBrush &windowTextInactive() const;

    // ========== 圆角尺寸属性 - 读写方法 ==========
    int kradiusMax() const;
    int kradiusMenu() const;
    int kradiusMin() const;
    int kradiusNormal() const;
    int kradiusWindow() const;

signals:
    void tokenChanged();

private:
    // ========== 基础调色板属性 ==========
    QBrush m_alternateBaseActive;
    QBrush m_alternateBaseDisable;
    QBrush m_alternateBaseInactive;
    QBrush m_baseActive;
    QBrush m_baseDisable;
    QBrush m_baseInactive;
    QBrush m_brightTextActive;
    QBrush m_brightTextDisable;
    QBrush m_brightTextInactive;
    QBrush m_buttonActive;
    QBrush m_buttonDisable;
    QBrush m_buttonInactive;
    QBrush m_buttonTextActive;
    QBrush m_buttonTextDisable;
    QBrush m_buttonTextInactive;
    QBrush m_darkActive;
    QBrush m_darkDisable;
    QBrush m_darkInactive;
    int m_focusline = 0;
    QBrush m_highLightActive;
    QBrush m_highLightDisable;
    QBrush m_highLightInactive;
    QBrush m_highLightedTextActive;
    QBrush m_highLightedTextDisable;
    QBrush m_highLightedTextInactive;

    // ========== 品牌色/组件色属性 ==========
    QBrush m_kBlack;
    QBrush m_kBrand1;
    QBrush m_kBrand2;
    QBrush m_kBrand3;
    QBrush m_kBrand4;
    QBrush m_kBrand5;
    QBrush m_kBrand6;
    QBrush m_kBrand7;
    QBrush m_kBrand8;
    QBrush m_kBrand9;
    QBrush m_kBrandClick;
    QBrush m_kBrandDisable;
    QBrush m_kBrandFocus;
    QBrush m_kBrandHover;
    QBrush m_kBrandNormal;
    QBrush m_kComponentAlphaClick;
    QBrush m_kComponentAlphaDisable;
    QBrush m_kComponentAlphaHover;
    QBrush m_kComponentAlphaNormal;
    QBrush m_kComponentClick;
    QBrush m_kComponentDisable;
    QBrush m_kComponentHover;
    QBrush m_kComponentNormal;
    QBrush m_kComponentSelectedAlphaDisable;
    QBrush m_kComponentSelectedAlphaNormal;
    QBrush m_kComponentSelectedAlphaHover;
    QBrush m_kComponentSelectedAlphaClick;
    QBrush m_kComponentInput;
    QBrush m_kComponentInputAlpha;
    QBrush m_kComponentSelectedDisable;
    QBrush m_kComponentSelectedNormal;
    QBrush m_kComponentSelectedHover;
    QBrush m_kComponentSelectedClick;

    // ========== 容器/分割线属性 - 私有变量（续） ==========
    QBrush m_kContainAlphaClick;
    QBrush m_kContainAlphaHover;
    QBrush m_kContainClick;
    QBrush m_kContainGeneralAlphaNormal;
    QBrush m_kContainGeneralInactive;
    QBrush m_kContainGeneralNormal;
    QBrush m_kContainHover;
    QBrush m_kContainSecondaryAlphaNormal;
    QBrush m_kContainSecondaryInactive;
    QBrush m_kContainSecondaryNormal;
    QBrush m_kDivider;
    QBrush m_kDividerWhite;

    // ========== 状态色（错误/成功/警告）属性 ==========
    QBrush m_kErrorClick;
    QBrush m_kErrorDisable;
    QBrush m_kErrorHover;
    QBrush m_kErrorNormal;
    QBrush m_kSuccessClick;
    QBrush m_kSuccessDisable;
    QBrush m_kSuccessHover;
    QBrush m_kSuccessNormal;
    QBrush m_kWarningClick;
    QBrush m_kWarningDisable;
    QBrush m_kWarningHover;
    QBrush m_kWarningNormal;

    // ========== 字体色属性 ==========
    QBrush m_kFontPlaceholdertextDisable;
    QBrush m_kFontPlaceholdertext;
    QBrush m_kFontPrimaryDisable;
    QBrush m_kFontPrimary;
    QBrush m_kFontSecondaryDisable;
    QBrush m_kFontSecondary;
    QBrush m_kFontStrong;
    QBrush m_kFontWhiteDisable;
    QBrush m_kFontWhitePlaceholdertextDisable;
    QBrush m_kFontWhiteplaceholdertext;
    QBrush m_kFontWhite;
    QBrush m_kFontWhiteSecondaryDisable;
    QBrush m_kFontWhiteSecondary;

    // ========== 灰色系属性 ==========
    QBrush m_kGray0;
    QBrush m_kGray1;
    QBrush m_kGray2;
    QBrush m_kGray3;
    QBrush m_kGray4;
    QBrush m_kGray5;
    QBrush m_kGray6;
    QBrush m_kGray7;
    QBrush m_kGray8;
    QBrush m_kGray9;
    QBrush m_kGray10;
    QBrush m_kGray11;
    QBrush m_kGray12;
    QBrush m_kGray13;
    QBrush m_kGray14;
    QBrush m_kGray15;
    QBrush m_kGray16;
    QBrush m_kGray17;
    QBrush m_kGrayAlpha0;
    QBrush m_kGrayAlpha1;
    QBrush m_kGrayAlpha2;
    QBrush m_kGrayAlpha3;
    QBrush m_kGrayAlpha4;
    QBrush m_kGrayAlpha5;
    QBrush m_kGrayAlpha6;
    QBrush m_kGrayAlpha7;
    QBrush m_kGrayAlpha8;
    QBrush m_kGrayAlpha9;
    QBrush m_kGrayAlpha10;
    QBrush m_kGrayAlpha11;
    QBrush m_kGrayAlpha12;
    QBrush m_kGrayAlpha13;

    // ========== 特殊色/线条属性 ==========
    QBrush m_kGreen1;
    QBrush m_kOrange1;
    QBrush m_kRed1;
    QBrush m_kLineBrandClick;
    QBrush m_kLineBrandDisable;
    QBrush m_kLineBrandHover;
    QBrush m_kLineBrandNormal;
    QBrush m_kLineComponentClick;
    QBrush m_kLineComponentDisable;
    QBrush m_kLineComponentHover;
    QBrush m_kLineComponentNormal;
    QBrush m_kLineDisable;
    QBrush m_kLineDock;
    QBrush m_kLineErrorClick;
    QBrush m_kLineErrorDisable;
    QBrush m_kLineErrorHover;
    QBrush m_kLineErrorNormal;
    QBrush m_kLineInputClick;
    QBrush m_kLineInputDisable;
    QBrush m_kLineInputHover;
    QBrush m_kLineInputNormal;
    QBrush m_kLineMenu;
    QBrush m_kLineNormal;
    QBrush m_kLineSelectboxClick;
    QBrush m_kLineSelectboxDisable;
    QBrush m_kLineSelectboxHover;
    QBrush m_kLineSelectboxNormal;
    QBrush m_kLineSelectboxSelected;
    QBrush m_kLineSuccessClick;
    QBrush m_kLineSuccessDisable;
    QBrush m_kLineSuccessHover;
    QBrush m_kLineSuccessNormal;
    QBrush m_kLineTable;
    QBrush m_kLineWarningClick;
    QBrush m_kLineWarningDisable;
    QBrush m_kLineWarningHover;
    QBrush m_kLineWarningNormal;
    QBrush m_kLineWindowActive;
    QBrush m_kLineWindowInactive;

    // ========== 菜单/阴影属性 ==========
    QBrush m_kMenu;
    QBrush m_kModalmask;
    QBrush m_kSelectAlphaWhiteClick;
    QBrush m_kSelectAlphaWhiteDisable;
    QBrush m_kSelectAlphaWhiteHover;
    QBrush m_kSelectAlphaWhite;
    QBrush m_kshadowComponent;
    QBrush m_kshadowMenu;
    QBrush m_kshadowMin;
    QBrush m_kshadowPrimaryActive;
    QBrush m_kshadowPrimaryInactive;
    QBrush m_kshadowSecondaryActive;
    QBrush m_kshadowSecondaryInactive;
    QBrush m_kWhite;

    // ========== 通用状态属性 ==========
    QBrush m_lightActive;
    QBrush m_lightDisable;
    QBrush m_lightInactive;
    QBrush m_linkActive;
    QBrush m_linkDisable;
    QBrush m_linkInactive;
    QBrush m_linkVisitedActive;
    QBrush m_linkVisitedDisable;
    QBrush m_linkVisitedInactive;
    QBrush m_midActive;
    QBrush m_midDisable;
    QBrush m_midInactive;
    QBrush m_midlightActive;
    QBrush m_midlightDisable;
    QBrush m_midlightInactive;
    int m_normalline = 0;
    QBrush m_noRoleActive;
    QBrush m_noRoleDisable;
    QBrush m_noRoleInactive;
    QBrush m_placeholderTextActive;
    QBrush m_placeholderTextDisable;
    QBrush m_placeholderTextInactive;
    QBrush m_shadow;
    QBrush m_shadowActive;
    QBrush m_shadowDisable;
    QBrush m_shadowInactive;
    QBrush m_textActive;
    QBrush m_textDisable;
    QBrush m_textInactive;

    // ========== 提示框/窗口属性 ==========
    QBrush m_toolTipBaseActive;
    QBrush m_toolTipBaseDisable;
    QBrush m_toolTipBaseInactive;
    QBrush m_toolTipTextActive;
    QBrush m_toolTipTextDisable;
    QBrush m_toolTipTextInactive;
    QBrush m_windowActive;
    QBrush m_windowDisable;
    QBrush m_windowInactive;
    QBrush m_windowTextActive;
    QBrush m_windowTextDisable;
    QBrush m_windowTextInactive;

    // ========== 圆角尺寸属性 ==========
    int m_kradiusMax = 0;
    int m_kradiusMenu = 0;
    int m_kradiusMin = 0;
    int m_kradiusNormal = 0;
    int m_kradiusWindow = 0;

    // 全局配置实例指针
    UKUIGlobalDTConfig::GlobalDTConfig *m_instance = nullptr;
};
} // namespace UKUIQQC2Style

// 注册附加属性供QML使用
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIToken, QML_HAS_ATTACHED_PROPERTIES)
#endif // UKUITOKEN_H
