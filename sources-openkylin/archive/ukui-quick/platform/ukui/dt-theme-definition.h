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

#ifndef DTTHEMEDEFINITION_H
#define DTTHEMEDEFINITION_H

#include "gradient-color.h"
#include "shadow-data.h"
#define DESIGN_TOKENS \
    GradientColor m_windowTextActive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                      \
    GradientColor m_windowTextInactive = GradientColor(QColor(0, 0, 0, 0.28 * 255));                    \
    GradientColor m_windowTextDisable = GradientColor(QColor(0, 0, 0, 0.28 * 255));                     \
    GradientColor m_buttonActive = GradientColor(QColor(240, 240, 240, 1 * 255));                       \
    GradientColor m_buttonInactive = GradientColor(QColor(240, 240, 240, 1 * 255));                     \
    GradientColor m_buttonDisable = GradientColor(QColor(240, 240, 240, 045 * 255));                    \
    GradientColor m_lightActive = GradientColor(QColor(255, 255, 255, 1 * 255));                        \
    GradientColor m_lightInactive = GradientColor(QColor(255, 255, 255, 1 * 255));                      \
    GradientColor m_lightDisable = GradientColor(QColor(255, 255, 255, 0.45 * 255));                    \
    GradientColor m_midLightActive = GradientColor(QColor(214, 214, 214, 1 * 255));                     \
    GradientColor m_midLightInactive = GradientColor(QColor(214, 214, 214, 1 * 255));                   \
    GradientColor m_midLightDisable = GradientColor(QColor(204, 204, 204, 0.42 * 255));                 \
    GradientColor m_darkActive = GradientColor(QColor(0, 0, 0, 1 * 255));                               \
    GradientColor m_darkInactive = GradientColor(QColor(0, 0, 0, 1 * 255));                             \
    GradientColor m_darkDisable = GradientColor(QColor(0, 0, 0, 1 * 255));                              \
    GradientColor m_midActive = GradientColor(QColor(102, 102, 102, 1 * 255));                          \
    GradientColor m_midInactive = GradientColor(QColor(102, 102, 102, 1 * 255));                        \
    GradientColor m_midDisable = GradientColor(QColor(102, 102, 102, 0.4 * 255));                       \
    GradientColor m_textActive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                            \
    GradientColor m_textInactive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                          \
    GradientColor m_textDisable = GradientColor(QColor(0, 0, 0, 0.28 * 255));                           \
    GradientColor m_brightTextActive = GradientColor(QColor(0, 0, 0, 1 * 255));                         \
    GradientColor m_brightTextInactive = GradientColor(QColor(0, 0, 0, 1 * 255));                       \
    GradientColor m_brightTextDisable = GradientColor(QColor(0, 0, 0, 0.45 * 255));                     \
    GradientColor m_buttonTextActive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                      \
    GradientColor m_buttonTextInactive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                    \
    GradientColor m_buttonTextDisable = GradientColor(QColor(0, 0, 0, 0.28 * 255));                     \
    GradientColor m_baseActive = GradientColor(QColor(255, 255, 255, 1 * 255));                         \
    GradientColor m_baseInactive = GradientColor(QColor(250, 250, 250, 1 * 255));                       \
    GradientColor m_baseDisable = GradientColor(QColor(250, 250, 250, 1 * 255));                        \
    GradientColor m_windowActive = GradientColor(QColor(245, 245, 245, 1 * 255));                       \
    GradientColor m_windowInactive = GradientColor(QColor(240, 240, 240, 1 * 255));                     \
    GradientColor m_windowDisable = GradientColor(QColor(240, 240, 240, 1 * 255));                      \
    GradientColor m_shadowActive = GradientColor(QColor(0, 0, 0, 0.25 * 255));                          \
    GradientColor m_shadowInactive = GradientColor(QColor(0, 0, 0, 0.18 * 255));                        \
    GradientColor m_shadowDisable = GradientColor(QColor(0, 0, 0, 0.11 * 255));                         \
    GradientColor m_highlightActive = GradientColor(QColor(55, 118, 245, 1 * 255));                     \
    GradientColor m_highlightInactive = GradientColor(QColor(55, 118, 245, 1 * 255));                   \
    GradientColor m_highlightDisable = GradientColor(QColor(55, 118, 245, 0.4 * 255));                  \
    GradientColor m_highlightedTextActive = GradientColor(QColor(255, 255, 255, 0.98 * 255));           \
    GradientColor m_highlightedTextInactive = GradientColor(QColor(255, 255, 255, 0.98 * 255));         \
    GradientColor m_highlightedTextDisable = GradientColor(QColor(255, 255, 255, 0.68 * 255));          \
    GradientColor m_linkActive = GradientColor(QColor(46, 95, 209, 1 * 255));                           \
    GradientColor m_linkInactive = GradientColor(QColor(46, 95, 209, 1 * 255));                         \
    GradientColor m_linkDisable = GradientColor(QColor(46, 95, 209, 0.4 * 255));                        \
    GradientColor m_linkVisitedDisable = GradientColor(QColor(120, 48, 191, 0.4 * 255));                \
    GradientColor m_alternateBaseActive = GradientColor(QColor(0, 0, 0, 0.03 * 255));                   \
    GradientColor m_alternateBaseInactive = GradientColor(QColor(0, 0, 0, 0.01 * 255));                 \
    GradientColor m_alternateBaseDisable = GradientColor(QColor(0, 0, 0, 0.01 * 255));                  \
    GradientColor m_noRoleActive = GradientColor(QColor(250, 250, 250, 1 * 255));                       \
    GradientColor m_noRoleInactive = GradientColor(QColor(250, 250, 250, 1 * 255));                     \
    GradientColor m_noRoleDisable = GradientColor(QColor(250, 250, 250, 0.42 * 255));                   \
    GradientColor m_toolTipBaseActive = GradientColor(QColor(255, 255, 255, 1 * 255));                  \
    GradientColor m_toolTipBaseInactive = GradientColor(QColor(255, 255, 255, 1 * 255));                \
    GradientColor m_toolTipBaseDisable = GradientColor(QColor(255, 255, 255, 0.4 * 255));               \
    GradientColor m_toolTipTextActive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                     \
    GradientColor m_toolTipTextInactive = GradientColor(QColor(0, 0, 0, 0.88 * 255));                   \
    GradientColor m_toolTipTextDisable = GradientColor(QColor(0, 0, 0, 0.28 * 255));                    \
    GradientColor m_placeholderTextActive = GradientColor(QColor(0, 0, 0, 0.28 * 255));                 \
    GradientColor m_placeholderTextInactive = GradientColor(QColor(0, 0, 0, 0.28 * 255));               \
    GradientColor m_placeholderTextDisable = GradientColor(QColor(0, 0, 0, 0.12 * 255));                \
                                                                                                        \
    GradientColor m_kWhite = GradientColor(QColor(255, 255, 255, 1 * 255));                             \
    GradientColor m_kBlack = GradientColor(QColor(0, 0, 0, 1 * 255));                                   \
    GradientColor m_kGray0 = GradientColor(QColor(255, 255, 255, 1 * 255));                             \
    GradientColor m_kGray1 = GradientColor(QColor(250, 250, 250, 1 * 255));                             \
    GradientColor m_kGray2 = GradientColor(QColor(245, 245, 245, 1 * 255));                             \
    GradientColor m_kGray3 = GradientColor(QColor(240, 240, 240, 1 * 255));                             \
    GradientColor m_kGray4 = GradientColor(QColor(235, 235, 235, 1 * 255));                             \
    GradientColor m_kGray5 = GradientColor(QColor(229, 229, 229, 1 * 255));                             \
    GradientColor m_kGray6 = GradientColor(QColor(224, 224, 224, 1 * 255));                             \
    GradientColor m_kGray7 = GradientColor(QColor(214, 214, 214, 1 * 255));                             \
    GradientColor m_kGray8 = GradientColor(QColor(199, 199, 199, 1 * 255));                             \
    GradientColor m_kGray9 = GradientColor(QColor(184, 184, 184, 1 * 255));                             \
    GradientColor m_kGray10 = GradientColor(QColor(163, 163, 163, 1 * 255));                            \
    GradientColor m_kGray11 = GradientColor(QColor(143, 143, 143, 1 * 255));                            \
    GradientColor m_kGray12 = GradientColor(QColor(122, 122, 122, 1 * 255));                            \
    GradientColor m_kGray13 = GradientColor(QColor(102, 102, 102, 1 * 255));                            \
    GradientColor m_kGray14 = GradientColor(QColor(82, 82, 82, 1 * 255));                               \
    GradientColor m_kGray15 = GradientColor(QColor(61, 61, 61, 1 * 255));                               \
    GradientColor m_kGray16 = GradientColor(QColor(41, 41, 41, 1 * 255));                               \
    GradientColor m_kGray17 = GradientColor(QColor(26, 26, 26, 1 * 255));                               \
    GradientColor m_kGrayAlpha0 = GradientColor(QColor(0, 0, 0, 0));                                    \
    GradientColor m_kGrayAlpha1 = GradientColor(QColor(0, 0, 0, 0.04 * 255));                           \
    GradientColor m_kGrayAlpha2 = GradientColor(QColor(0, 0, 0, 0.06 * 255));                           \
    GradientColor m_kGrayAlpha3 = GradientColor(QColor(0, 0, 0, 0.08 * 255));                           \
    GradientColor m_kGrayAlpha4 = GradientColor(QColor(0, 0, 0, 0.12 * 255));                           \
    GradientColor m_kGrayAlpha5 = GradientColor(QColor(0, 0, 0, 0.16 * 255));                           \
    GradientColor m_kGrayAlpha6 = GradientColor(QColor(0, 0, 0, 0.24 * 255));                           \
    GradientColor m_kGrayAlpha7 = GradientColor(QColor(0, 0, 0, 0.30 * 255));                           \
    GradientColor m_kGrayAlpha8 = GradientColor(QColor(0, 0, 0, 0.38 * 255));                           \
    GradientColor m_kGrayAlpha9 = GradientColor(QColor(0, 0, 0, 0.42 * 255));                           \
    GradientColor m_kGrayAlpha10 = GradientColor(QColor(0, 0, 0, 0.55 * 255));                          \
    GradientColor m_kGrayAlpha11 = GradientColor(QColor(0, 0, 0, 0.9 * 255));                           \
    GradientColor m_kGrayAlpha12 = GradientColor(QColor(0, 0, 0, 0.02 * 255));                          \
    GradientColor m_kGrayAlpha13 = GradientColor(QColor(0, 0, 0, 0.75 * 255));                          \
                                                                                                        \
    GradientColor m_kFontStrong = GradientColor(QColor(0, 0, 0, 1 * 255));                              \
    GradientColor m_kFontPrimary = GradientColor(QColor(0, 0, 0, 0.88 * 255));                          \
    GradientColor m_kFontPrimaryDisable = GradientColor(QColor(0, 0, 0, 0.28 * 255));                   \
    GradientColor m_kFontSecondary = GradientColor(QColor(0, 0, 0, 0.4 * 255));                         \
    GradientColor m_kFontSecondaryDisable = GradientColor(QColor(0, 0, 0, 0.15 * 255));                 \
    GradientColor m_kFontWhite = GradientColor(QColor(255, 255, 255, 0.98 * 255));                      \
    GradientColor m_kFontWhiteDisable = GradientColor(QColor(255, 255, 255, 0.68 * 255));               \
    GradientColor m_kFontWhiteSecondary = GradientColor(QColor(255, 255, 255, 0.65 * 255));             \
    GradientColor m_kFontWhiteSecondaryDisable = GradientColor(QColor(255, 255, 255, 0.24 * 255));      \
                                                                                                        \
    GradientColor m_kBrandNormal = GradientColor(QColor(54, 118, 245, 1 * 255));                        \
    GradientColor m_kSuccessNormal = GradientColor(QColor(24, 168, 13, 1 * 255));                       \
    GradientColor m_kWarningNormal = GradientColor(QColor(255, 128, 31, 1 * 255));                      \
    GradientColor m_kErrorNormal = GradientColor(QColor(245, 73, 73, 1 * 255));                         \
    GradientColor m_kBrand1 = GradientColor(QColor(55, 144, 250, 1 * 255));                             \
    GradientColor m_kBrand2 = GradientColor(QColor(137, 38, 235, 1 * 255));                             \
    GradientColor m_kBrand3 = GradientColor(QColor(228, 74, 232, 1 * 255));                             \
    GradientColor m_kBrand4 = GradientColor(QColor(242, 181, 39, 1 * 255));                             \
    GradientColor m_kBrand5 = GradientColor(QColor(20, 166, 33, 1 * 255));                              \
    GradientColor m_kBrand6 = GradientColor(QColor(240, 73, 67, 1 * 255));                              \
    GradientColor m_kBrand7 = GradientColor(QColor(250, 125, 15, 1 * 255));                             \
                                                                                                        \
    GradientColor m_kContainGeneralNormal = GradientColor(QColor(255, 255, 255, 1 * 255));              \
    GradientColor m_kContainSecondaryNormal = GradientColor(QColor(250, 250, 250, 1 * 255));            \
    GradientColor m_kContainSecondaryAlphaNormal = GradientColor(QColor(0, 0, 0, 0.02 * 255));          \
    GradientColor m_kComponentNormal = GradientColor(QColor(240, 240, 240, 1 * 255));                   \
    GradientColor m_kComponentDisable = GradientColor(QColor(245, 245, 245, 0.45 * 255));               \
    GradientColor m_kComponentAlphaNormal = GradientColor(QColor(0, 0, 0, 0.04 * 255));                 \
    GradientColor m_kComponentAlphaDisable = GradientColor(QColor(0, 0, 0, 0.02 * 255));                \
    GradientColor m_kLineWindowActive = GradientColor(QColor(255, 255, 255, 0.7 * 255));                \
    GradientColor m_kLineNormal = GradientColor(QColor(0, 0, 0, 0.06 * 255));                           \
    GradientColor m_kLineComponentNormal = GradientColor(QColor(0, 0, 0, 0.06 * 255));                  \
    GradientColor m_kLineComponentHover = GradientColor(QColor(0, 0, 0, 0.08 * 255));                   \
    GradientColor m_kLineComponentClick = GradientColor(QColor(0, 0, 0, 0.12 * 255));                   \
    GradientColor m_kLineComponentDisable = GradientColor(QColor(0, 0, 0, 0.03 * 255));                 \
    GradientColor m_kLineBrandNormal = GradientColor(QColor(54, 118, 245, 0.55 * 255));                 \
    GradientColor m_kLineBrandHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(54,118,245,0.55*255));\
    GradientColor m_kLineBrandClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(54,118,245,0.55*255));\
    GradientColor m_kLineBrandDisable = GradientColor(QColor(54, 118, 245, 0.23 * 255));                \
    GradientColor m_kModalMask = GradientColor(QColor(0, 0, 0, 0.24 * 255));                            \
                                                                                                        \
    int m_normalLine = 1;                                                                               \
    int m_focusLine = 2;                                                                                \
                                                                                                        \
    int m_kRadiusMin = 4;                                                                               \
    int m_kRadiusNormal = 6;                                                                            \
    int m_kRadiusMax = 8;                                                                               \
    int m_kRadiusMenu = 8;                                                                              \
    int m_kRadiusWindow = 12;                                                                           \
                                                                                                        \
    int m_kMarginMin = 4;                                                                               \
    int m_kMarginNormal = 8;                                                                            \
    int m_kMarginBig = 16;                                                                              \
    int m_kMarginWindow = 24;                                                                           \
    int m_kMarginComponent = 40;                                                                        \
                                                                                                        \
    int m_kPaddingMinLeft = 8;                                                                          \
    int m_kPaddingMinTop = 2;                                                                           \
    int m_kPaddingMinRight = 8;                                                                         \
    int m_kPaddingMinBottom = 2;                                                                        \
    int m_kPaddingNormalLeft = 16;                                                                      \
    int m_kPaddingNormalTop = 2;                                                                        \
    int m_kPaddingNormalRight = 16;                                                                     \
    int m_kPaddingNormalBottom = 2;                                                                     \
    int m_kPadding8Left = 8;                                                                            \
    int m_kPadding8Top = 8;                                                                             \
    int m_kPadding8Right = 8;                                                                           \
    int m_kPadding8Bottom = 8;                                                                          \
    int m_kPaddingWindowLeft = 24;                                                                      \
    int m_kPaddingWindowTop = 16;                                                                       \
    int m_kPaddingWindowRight = 24;                                                                     \
    int m_kPaddingWindowBottom = 24;                                                                    \
                                                                                                        \
    GradientColor m_kErrorHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(245,73,73,1*255));\
    GradientColor m_kErrorClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(245,73,73,1*255));\
    GradientColor m_kBrandClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(54,118,245,1*255));\
    GradientColor m_kBrandFocus = GradientColor(QColor(0, 0, 0, 0.75 * 255));                           \
    GradientColor m_kBrandHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(54,118,245,1*255));\
    GradientColor m_kContainHover = GradientColor(QColor(245, 245, 245, 1 * 255));                      \
    GradientColor m_kContainClick = GradientColor(QColor(235, 235, 235, 1 * 255));                      \
    GradientColor m_kComponentHover = GradientColor(QColor(245, 245, 245, 1 * 255));                    \
    GradientColor m_kComponentClick = GradientColor(QColor(224, 224, 224, 1 * 255));                    \
    GradientColor m_kComponentAlphaHover = GradientColor(QColor(0,0,0,0.08*255));                       \
    GradientColor m_kComponentAlphaClick = GradientColor(QColor(0,0,0,0.12*255));                       \
    GradientColor m_kSuccessHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(24,168,13,1*255));\
    GradientColor m_kSuccessClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(0,180,250,1*255));\
    GradientColor m_kWarningHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(255,128,31,1*255));\
    GradientColor m_kWarningClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(255,128,31,1*255));\
    GradientColor m_linkVisitedActive = GradientColor(QColor(120, 48, 191, 1 * 255));                   \
    GradientColor m_linkVisitedInactive = GradientColor(QColor(120, 48, 191, 1 * 255));                 \
    GradientColor m_kSuccessDisable = GradientColor(QColor(24, 168, 13, 0.45 * 255));                   \
    GradientColor m_kWarningDisable = GradientColor(QColor(255, 128, 31, 0.45 * 255));                  \
    GradientColor m_kErrorDisable = GradientColor(QColor(245, 73, 73, 0.45 * 255));                     \
    GradientColor m_kLineWindowInactive = GradientColor(QColor(255, 255, 255, 0.4 * 255));              \
    GradientColor m_kContainAlphaClick = GradientColor(QColor(0, 0, 0, 0.08 * 255));                    \
    GradientColor m_kContainAlphaHover = GradientColor(QColor(0, 0, 0, 0.04 * 255));                    \
    GradientColor m_kBrandDisable = GradientColor(QColor(54, 118, 245, 0.4 * 255));                     \
    GradientColor m_kLineDisable = GradientColor(QColor(0, 0, 0, 0.03 * 255));                          \
    GradientColor m_kDivider = GradientColor(QColor(0, 0, 0, 0.08 * 255));                              \
    GradientColor m_kContainGeneralAlphaNormal = GradientColor(QColor(0, 0, 0, 0));                     \
                                                                                                        \
    GradientColor m_kBrand8 = GradientColor(QColor(54, 118, 245, 1 * 255));                             \
    GradientColor m_kBrand9 = GradientColor(QColor(6, 192, 199, 1 * 255));                              \
    GradientColor m_kLineSelectboxNormal = GradientColor(QColor(0, 0, 0, 0.2 * 255));                   \
    GradientColor m_kLineSelectboxHover = GradientColor(QColor(0, 0, 0, 0.24 * 255));                   \
    GradientColor m_kLineSelectboxClick = GradientColor(QColor(0, 0, 0, 0.28 * 255));                   \
    GradientColor m_kLineSelectboxSelected = GradientColor(QColor(0, 0, 0, 0 * 255));                   \
    GradientColor m_kLineSelectboxDisable = GradientColor(QColor(0, 0, 0, 0.08 * 255));                 \
    GradientColor m_kFontWhitePlaceholderTextDisable = GradientColor(QColor(255, 255, 255, 0.13 * 255));\
    GradientColor m_kContainGeneralInactive = GradientColor(QColor(250, 250, 250, 1 * 255));            \
    GradientColor m_kFontPlaceholderTextDisable = GradientColor(QColor(0, 0, 0, 0.12 * 255));           \
    GradientColor m_kFontWhitePlaceholderText = GradientColor(QColor(255, 255, 255, 0.35 * 255));       \
    GradientColor m_kLineInputDisable = GradientColor(QColor(0, 0, 0, 0.04 * 255));                     \
    GradientColor m_kLineDock = GradientColor(QColor(255, 255, 255, 0.55 * 255));                       \
    GradientColor m_kLineMenu = GradientColor(QColor(255, 255, 255, 0.7 * 255));                        \
    GradientColor m_kLineTable = GradientColor(QColor(0, 0, 0, 0.12 * 255));                            \
    GradientColor m_kLineInputNormal = GradientColor(QColor(0, 0, 0, 0.08 * 255));                      \
    GradientColor m_kLineInputHover = GradientColor(QColor(0, 0, 0, 0.12 * 255));                       \
    GradientColor m_kLineInputClick = GradientColor(QColor(54, 118, 245, 1 * 255));                     \
    GradientColor m_kOrange1 = GradientColor(QColor(255, 128, 31, 1 * 255));                            \
    GradientColor m_kGreen1 = GradientColor(QColor(24, 168, 13, 1 * 255));                              \
    GradientColor m_kRed1 = GradientColor(QColor(235, 70, 70, 1 * 255));                                \
    GradientColor m_kDividerWhite = GradientColor(QColor(255, 255, 255, 0.12 * 255));                   \
                                                                                                        \
    GradientColor m_kLineSuccessNormal = GradientColor(QColor(24, 168, 13, 0.55 * 255));                \
    GradientColor m_kLineSuccessHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(24,168,13,0.55*255));\
    GradientColor m_kLineSuccessClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(24,168,13,0.55*255));\
    GradientColor m_kLineSuccessDisable = GradientColor(QColor(24, 168, 13, 0.23 * 255));               \
    GradientColor m_kLineErrorNormal = GradientColor(QColor(245, 73, 73, 0.55 * 255));                  \
    GradientColor m_kLineErrorHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(245,73,73,0.55*255));\
    GradientColor m_kLineErrorClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(245,73,73,0.55*255));\
    GradientColor m_kLineWarningNormal = GradientColor(QColor(255, 128, 31, 0.55 * 255));               \
    GradientColor m_kLineWarningHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(255,128,31, 0.55*255));\
    GradientColor m_kLineWarningClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(255,128,31, 0.55*255));\
    GradientColor m_kLineWarningDisable = GradientColor(QColor(255, 128, 31, 0.23 * 255));              \
    ShadowData m_kShadowPrimaryActive = ShadowData(0, 18, 38, 0, QColor(0, 0, 0, 0.13 * 255));          \
    ShadowData m_kShadowPrimaryInactive = ShadowData(0, 8, 16, 0, QColor(0, 0, 0, 0.18 * 255));         \
    ShadowData m_kShadowSecondaryActive = ShadowData(0, 12, 26, 0, QColor(0, 0, 0, 0.25 * 255));        \
    ShadowData m_kShadowSecondaryInactive = ShadowData(0, 6, 13, 0, QColor(0, 0, 0, 0.18 * 255));       \
    ShadowData m_kShadowMenu = ShadowData(0, 6, 14, 0, QColor(0, 0, 0, 0.2 * 255));                     \
    ShadowData m_kShadowMin = ShadowData(0, 2, 6, 0, QColor(0, 0, 0, 0.15 * 255));                      \
    ShadowData m_kShadowComponent = ShadowData(0, 1, 2, 0, QColor(0, 0, 0, 0.1 * 255));                 \
    GradientColor m_kComponentSelectedNormal = GradientColor(QColor(240, 240, 240, 1 * 255));           \
    GradientColor m_kComponentSelectedHover = GradientColor(QColor(235, 235, 235, 1 * 255));            \
    GradientColor m_kComponentSelectedClick = GradientColor(QColor(224, 224, 224, 1 * 255));            \
    GradientColor m_kComponentSelectedDisable = GradientColor(QColor(224, 224, 224, 0.45 * 255));       \
    GradientColor m_kComponentSelectedAlphaNormal = GradientColor(QColor(0, 0, 0, 0.08 * 255));         \
    GradientColor m_kComponentSelectedAlphaHover = GradientColor(QColor(0, 0, 0, 0.12 * 255));          \
    GradientColor m_kComponentSelectedAlphaClick = GradientColor(QColor(0, 0, 0, 0.16 * 255));          \
    GradientColor m_kComponentSelectedAlphaDisable = GradientColor(QColor(0, 0, 0, 0.04 * 255));        \
    GradientColor m_kComponentInput = GradientColor(QColor(255, 255, 255, 1 * 255));                    \
    GradientColor m_kComponentInputAlpha = GradientColor(QColor(255, 255, 255, 0.5 * 255));             \
    GradientColor m_kSelectAlphaWhiteDisable = GradientColor(QColor(255, 255, 255, 0.45 * 255));        \
    GradientColor m_kSelectAlphaWhiteHover = GradientColor(0, QColor(0,0,0,0.1*255), QColor(0,0,0,0.1*255), QColor(255,255,255,1*255));\
    GradientColor m_kSelectAlphaWhiteClick = GradientColor(0, QColor(0,0,0,0.2*255), QColor(0,0,0,0.2*255), QColor(255,255,255,1*255));\
    GradientColor m_kMenu = GradientColor(QColor(250, 250, 250, 1 * 255));                              \
    GradientColor m_kLineErrorDisable = GradientColor(QColor(245, 73, 73, 0.23 * 255));                 \
    GradientColor m_kContainSecondaryInactive = GradientColor(QColor(245, 245, 245, 1 * 255));          \
    GradientColor m_kSelectAlphaWhite = GradientColor(QColor(255, 255, 255, 1 * 255));                  \
    GradientColor m_kFontPlaceholderText = GradientColor(QColor(0, 0, 0, 0.28 * 255));                  \
    GradientColor m_kBrand10 = GradientColor(QColor(25, 131, 255, 1 * 255));                            \
    GradientColor m_kPanel = GradientColor(QColor(255, 255, 255, 0.65 * 255));                          \


namespace UkuiQuick {

class DtThemeDefinition : public QObject
{
    Q_OBJECT
public:
    DtThemeDefinition(QObject *parent = nullptr);

};
}
#endif //DTTHEMEDEFINITION_H
