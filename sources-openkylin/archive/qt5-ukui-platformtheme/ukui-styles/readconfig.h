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

#ifndef READCONFIG_H
#define READCONFIG_H

#include <QObject>
#include <QColor>
#include <QPalette>
#include <QtQuick/QQuickItem>
#include <QMetaType>

namespace UKUIGlobalDTConfig {


class GlobalDTConfigPrivate  : public QObject
{
    Q_OBJECT

public:

    explicit GlobalDTConfigPrivate();
    ~GlobalDTConfigPrivate();

    inline bool getColorValue(QBrush &dtBrush, const QString key);
    inline bool getValue(int &value, const QString key) const;
    inline QColor stringToColor(QString s) const;
    inline QColor mixColor(QList<QColor> colorList);
    inline QColor composeColor(QColor foreColor, QColor backColor);

    bool load(const QString &cfg);
    void initUKUIGlobalThemeParameters();
    void updateColorsBasedOnAccentColor(const QColor& accentColor);
    QColor processCSSColor(const QString& cssString, const QColor& replacementColor);
    void init();
    void adjustHighlightColor(QBrush highlightColor, QString hoverStr, QString clickStr);
    qreal configMixQreal(qreal a, qreal b, qreal bias);
    QColor configMixColor(const QColor &c1, const QColor &c2, qreal bias);
    QString getKeyValue(const QString key);


    QBrush alternateBaseActive              = QBrush(Qt::NoBrush);
    QBrush alternateBaseDisable             = QBrush(Qt::NoBrush);
    QBrush alternateBaseInactive            = QBrush(Qt::NoBrush);
    QBrush baseActive                       = QBrush(Qt::NoBrush);
    QBrush baseDisable                      = QBrush(Qt::NoBrush);
    QBrush baseInactive                     = QBrush(Qt::NoBrush);
    QBrush brightTextActive                 = QBrush(Qt::NoBrush);
    QBrush brightTextDisable                = QBrush(Qt::NoBrush);
    QBrush brightTextInactive               = QBrush(Qt::NoBrush);
    QBrush buttonActive                     = QBrush(Qt::NoBrush);
    QBrush buttonDisable                    = QBrush(Qt::NoBrush);
    QBrush buttonInactive                   = QBrush(Qt::NoBrush);
    QBrush buttonTextActive                 = QBrush(Qt::NoBrush);
    QBrush buttonTextDisable                = QBrush(Qt::NoBrush);
    QBrush buttonTextInactive               = QBrush(Qt::NoBrush);
    QBrush darkActive                       = QBrush(Qt::NoBrush);
    QBrush darkDisable                      = QBrush(Qt::NoBrush);
    QBrush darkInactive                     = QBrush(Qt::NoBrush);
    int    focusline                                             ;
    QBrush highLightActive                  = QBrush(Qt::NoBrush);
    QBrush highLightDisable                 = QBrush(Qt::NoBrush);
    QBrush highLightInactive                = QBrush(Qt::NoBrush);
    QBrush highLightedTextActive            = QBrush(Qt::NoBrush);
    QBrush highLightedTextDisable           = QBrush(Qt::NoBrush);
    QBrush highLightedTextInactive          = QBrush(Qt::NoBrush);
    QBrush kBlack                           = QBrush(Qt::NoBrush);
    QBrush kBrand1                          = QBrush(Qt::NoBrush);
    QBrush kBrand2                          = QBrush(Qt::NoBrush);
    QBrush kBrand3                          = QBrush(Qt::NoBrush);
    QBrush kBrand4                          = QBrush(Qt::NoBrush);
    QBrush kBrand5                          = QBrush(Qt::NoBrush);
    QBrush kBrand6                          = QBrush(Qt::NoBrush);
    QBrush kBrand7                          = QBrush(Qt::NoBrush);
    QBrush kBrand8                          = QBrush(Qt::NoBrush);
    QBrush kBrand9                          = QBrush(Qt::NoBrush);
    QBrush kBrandClick                      = QBrush(Qt::NoBrush);
    QBrush kBrandDisable                    = QBrush(Qt::NoBrush);
    QBrush kBrandFocus                      = QBrush(Qt::NoBrush);
    QBrush kBrandHover                      = QBrush(Qt::NoBrush);
    QBrush kBrandNormal                     = QBrush(Qt::NoBrush);
    QBrush kComponentAlphaClick             = QBrush(Qt::NoBrush);
    QBrush kComponentAlphaDisable           = QBrush(Qt::NoBrush);
    QBrush kComponentAlphaHover             = QBrush(Qt::NoBrush);
    QBrush kComponentAlphaNormal            = QBrush(Qt::NoBrush);
    QBrush kComponentClick                  = QBrush(Qt::NoBrush);
    QBrush kComponentDisable                = QBrush(Qt::NoBrush);
    QBrush kComponentHover                  = QBrush(Qt::NoBrush);
    QBrush kComponentNormal                 = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedAlphaDisable   = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedAlphaNormal    = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedAlphaHover     = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedAlphaClick     = QBrush(Qt::NoBrush);

    QBrush kComponentInput                  = QBrush(Qt::NoBrush);
    QBrush kComponentInputAlpha             = QBrush(Qt::NoBrush);

    QBrush kComponentSelectedDisable        = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedNormal         = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedHover          = QBrush(Qt::NoBrush);
    QBrush kComponentSelectedClick          = QBrush(Qt::NoBrush);

    QBrush kContainAlphaClick               = QBrush(Qt::NoBrush);
    QBrush kContainAlphaHover               = QBrush(Qt::NoBrush);
    QBrush kContainClick                    = QBrush(Qt::NoBrush);
    QBrush kContainGeneralAlphaNormal       = QBrush(Qt::NoBrush);
    QBrush kContainGeneralInactive          = QBrush(Qt::NoBrush);
    QBrush kContainGeneralNormal            = QBrush(Qt::NoBrush);
    QBrush kContainHover                    = QBrush(Qt::NoBrush);
    QBrush kContainSecondaryAlphaNormal     = QBrush(Qt::NoBrush);
    QBrush kContainSecondaryInactive        = QBrush(Qt::NoBrush);
    QBrush kContainSecondaryNormal          = QBrush(Qt::NoBrush);
    QBrush kDivider                         = QBrush(Qt::NoBrush);
    QBrush kDividerWhite                    = QBrush(Qt::NoBrush);
    QBrush kErrorClick                      = QBrush(Qt::NoBrush);
    QBrush kErrorDisable                    = QBrush(Qt::NoBrush);
    QBrush kErrorHover                      = QBrush(Qt::NoBrush);
    QBrush kErrorNormal                     = QBrush(Qt::NoBrush);
    QBrush kFontPlaceholdertextDisable      = QBrush(Qt::NoBrush);
    QBrush kFontPlaceholdertext             = QBrush(Qt::NoBrush);
    QBrush kFontPrimaryDisable              = QBrush(Qt::NoBrush);
    QBrush kFontPrimary                     = QBrush(Qt::NoBrush);
    QBrush kFontSecondaryDisable            = QBrush(Qt::NoBrush);
    QBrush kFontSecondary                   = QBrush(Qt::NoBrush);
    QBrush kFontStrong                      = QBrush(Qt::NoBrush);
    QBrush kFontWhiteDisable                = QBrush(Qt::NoBrush);
    QBrush kFontWhitePlaceholdertextDisable = QBrush(Qt::NoBrush);
    QBrush kFontWhiteplaceholdertext        = QBrush(Qt::NoBrush);
    QBrush kFontWhite                       = QBrush(Qt::NoBrush);
    QBrush kFontWhiteSecondaryDisable       = QBrush(Qt::NoBrush);
    QBrush kFontWhiteSecondary              = QBrush(Qt::NoBrush);
    QBrush kGray0                           = QBrush(Qt::NoBrush);
    QBrush kGray1                           = QBrush(Qt::NoBrush);
    QBrush kGray2                           = QBrush(Qt::NoBrush);
    QBrush kGray3                           = QBrush(Qt::NoBrush);
    QBrush kGray4                           = QBrush(Qt::NoBrush);
    QBrush kGray5                           = QBrush(Qt::NoBrush);
    QBrush kGray6                           = QBrush(Qt::NoBrush);
    QBrush kGray7                           = QBrush(Qt::NoBrush);
    QBrush kGray8                           = QBrush(Qt::NoBrush);
    QBrush kGray9                           = QBrush(Qt::NoBrush);
    QBrush kGray10                          = QBrush(Qt::NoBrush);
    QBrush kGray11                          = QBrush(Qt::NoBrush);
    QBrush kGray12                          = QBrush(Qt::NoBrush);
    QBrush kGray13                          = QBrush(Qt::NoBrush);
    QBrush kGray14                          = QBrush(Qt::NoBrush);
    QBrush kGray15                          = QBrush(Qt::NoBrush);
    QBrush kGray16                          = QBrush(Qt::NoBrush);
    QBrush kGray17                          = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha0                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha10                     = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha11                     = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha12                     = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha13                     = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha1                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha2                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha3                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha4                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha5                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha6                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha7                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha8                      = QBrush(Qt::NoBrush);
    QBrush kGrayAlpha9                      = QBrush(Qt::NoBrush);
    QBrush kGreen1                          = QBrush(Qt::NoBrush);
    QBrush kLineBrandClick                  = QBrush(Qt::NoBrush);
    QBrush kLineBrandDisable                = QBrush(Qt::NoBrush);
    QBrush kLineBrandHover                  = QBrush(Qt::NoBrush);
    QBrush kLineBrandNormal                 = QBrush(Qt::NoBrush);
    QBrush kLineComponentClick              = QBrush(Qt::NoBrush);
    QBrush kLineComponentDisable            = QBrush(Qt::NoBrush);
    QBrush kLineComponentHover              = QBrush(Qt::NoBrush);
    QBrush kLineComponentNormal             = QBrush(Qt::NoBrush);
    QBrush kLineDisable                     = QBrush(Qt::NoBrush);
    QBrush kLineDock                        = QBrush(Qt::NoBrush);
    QBrush kLineErrorClick                  = QBrush(Qt::NoBrush);
    QBrush kLineErrorDisable                = QBrush(Qt::NoBrush);
    QBrush kLineErrorHover                  = QBrush(Qt::NoBrush);
    QBrush kLineErrorNormal                 = QBrush(Qt::NoBrush);
    QBrush kLineInputClick                  = QBrush(Qt::NoBrush);
    QBrush kLineInputDisable                = QBrush(Qt::NoBrush);
    QBrush kLineInputHover                  = QBrush(Qt::NoBrush);
    QBrush kLineInputNormal                 = QBrush(Qt::NoBrush);
    QBrush kLineMenu                        = QBrush(Qt::NoBrush);
    QBrush kLineNormal                      = QBrush(Qt::NoBrush);
    QBrush kLineSelectboxClick              = QBrush(Qt::NoBrush);
    QBrush kLineSelectboxDisable            = QBrush(Qt::NoBrush);
    QBrush kLineSelectboxHover              = QBrush(Qt::NoBrush);
    QBrush kLineSelectboxNormal             = QBrush(Qt::NoBrush);
    QBrush kLineSelectboxSelected           = QBrush(Qt::NoBrush);
    QBrush kLineSuccessClick                = QBrush(Qt::NoBrush);
    QBrush kLineSuccessDisable              = QBrush(Qt::NoBrush);
    QBrush kLineSuccessHover                = QBrush(Qt::NoBrush);
    QBrush kLineSuccessNormal               = QBrush(Qt::NoBrush);
    QBrush kLineTable                       = QBrush(Qt::NoBrush);
    QBrush kLineWarningClick                = QBrush(Qt::NoBrush);
    QBrush kLineWarningDisable              = QBrush(Qt::NoBrush);
    QBrush kLineWarningHover                = QBrush(Qt::NoBrush);
    QBrush kLineWarningNormal               = QBrush(Qt::NoBrush);
    QBrush kLineWindowActive                = QBrush(Qt::NoBrush);
    QBrush kLineWindowInactive              = QBrush(Qt::NoBrush);
    QBrush kMenu                            = QBrush(Qt::NoBrush);
    QBrush kModalmask                       = QBrush(Qt::NoBrush);
    QBrush kOrange1                         = QBrush(Qt::NoBrush);
    int    kradiusMax                                 ;
    int    kradiusMenu                                ;
    int    kradiusMin                                 ;
    int    kradiusNormal                              ;
    int    kradiusWindow                              ;
    QBrush kRed1                            = QBrush(Qt::NoBrush);
    QBrush kSelectAlphaWhiteClick           = QBrush(Qt::NoBrush);
    QBrush kSelectAlphaWhiteDisable         = QBrush(Qt::NoBrush);
    QBrush kSelectAlphaWhiteHover           = QBrush(Qt::NoBrush);
    QBrush kSelectAlphaWhite                = QBrush(Qt::NoBrush);
    QBrush kshadowComponent                 = QBrush(Qt::NoBrush);
    QBrush kshadowMenu                      = QBrush(Qt::NoBrush);
    QBrush kshadowMin                       = QBrush(Qt::NoBrush);
    QBrush kshadowPrimaryActive             = QBrush(Qt::NoBrush);
    QBrush kshadowPrimaryInactive           = QBrush(Qt::NoBrush);
    QBrush kshadowSecondaryActive           = QBrush(Qt::NoBrush);
    QBrush kshadowSecondaryInactive         = QBrush(Qt::NoBrush);
    QBrush kSuccessClick                    = QBrush(Qt::NoBrush);
    QBrush kSuccessDisable                  = QBrush(Qt::NoBrush);
    QBrush kSuccessHover                    = QBrush(Qt::NoBrush);
    QBrush kSuccessNormal                   = QBrush(Qt::NoBrush);
    QBrush kWarningClick                    = QBrush(Qt::NoBrush);
    QBrush kWarningDisable                  = QBrush(Qt::NoBrush);
    QBrush kWarningHover                    = QBrush(Qt::NoBrush);
    QBrush kWarningNormal                   = QBrush(Qt::NoBrush);
    QBrush kWhite                           = QBrush(Qt::NoBrush);
    QBrush lightActive                      = QBrush(Qt::NoBrush);
    QBrush lightDisable                     = QBrush(Qt::NoBrush);
    QBrush lightInactive                    = QBrush(Qt::NoBrush);
    QBrush linkActive                       = QBrush(Qt::NoBrush);
    QBrush linkDisable                      = QBrush(Qt::NoBrush);
    QBrush linkInactive                     = QBrush(Qt::NoBrush);
    QBrush linkVisitedActive                = QBrush(Qt::NoBrush);
    QBrush linkVisitedDisable               = QBrush(Qt::NoBrush);
    QBrush linkVisitedInactive              = QBrush(Qt::NoBrush);
    QBrush midActive                        = QBrush(Qt::NoBrush);
    QBrush midDisable                       = QBrush(Qt::NoBrush);
    QBrush midInactive                      = QBrush(Qt::NoBrush);
    QBrush midlightActive                   = QBrush(Qt::NoBrush);
    QBrush midlightDisable                  = QBrush(Qt::NoBrush);
    QBrush midlightInactive                 = QBrush(Qt::NoBrush);
    int    normalline                                            ;
    QBrush noRoleActive                     = QBrush(Qt::NoBrush);
    QBrush noRoleDisable                    = QBrush(Qt::NoBrush);
    QBrush noRoleInactive                   = QBrush(Qt::NoBrush);
    QBrush placeholderTextActive            = QBrush(Qt::NoBrush);
    QBrush placeholderTextDisable           = QBrush(Qt::NoBrush);
    QBrush placeholderTextInactive          = QBrush(Qt::NoBrush);
    QBrush shadow                           = QBrush(Qt::NoBrush);
    QBrush shadowActive                     = QBrush(Qt::NoBrush);
    QBrush shadowDisable                    = QBrush(Qt::NoBrush);
    QBrush shadowInactive                   = QBrush(Qt::NoBrush);
    QBrush textActive                       = QBrush(Qt::NoBrush);
    QBrush textDisable                      = QBrush(Qt::NoBrush);
    QBrush textInactive                     = QBrush(Qt::NoBrush);

    QBrush toolTipBaseActive                = QBrush(Qt::NoBrush);
    QBrush toolTipBaseDisable               = QBrush(Qt::NoBrush);
    QBrush toolTipBaseInactive              = QBrush(Qt::NoBrush);
    QBrush toolTipTextActive                = QBrush(Qt::NoBrush);
    QBrush toolTipTextDisable               = QBrush(Qt::NoBrush);
    QBrush toolTipTextInactive              = QBrush(Qt::NoBrush);
    QBrush windowActive                     = QBrush(Qt::NoBrush);
    QBrush windowDisable                    = QBrush(Qt::NoBrush);
    QBrush windowInactive                   = QBrush(Qt::NoBrush);
    QBrush windowTextActive                 = QBrush(Qt::NoBrush);
    QBrush windowTextDisable                = QBrush(Qt::NoBrush);
    QBrush windowTextInactive               = QBrush(Qt::NoBrush);


    bool m_ukui_dt_loadConfig = false;
    QString m_cfgPath;
    QString m_widgetThemeName = "default";
    QString m_lightMode = "light";
    QString m_ukui_dt_path;
    QString m_dt_configData;
    QMap<QString, QVariant> dtMap;
};

class GlobalDTConfig : public QObject
{
    Q_OBJECT
public:
    explicit GlobalDTConfig();
    ~GlobalDTConfig();

    //QT palette
    QBrush windowTextActive() const;
    QBrush windowTextInactive() const;
    QBrush windowTextDisable() const;
    QBrush buttonActive() const;
    QBrush buttonInactive() const;
    QBrush buttonDisable() const;
    QBrush lightActive() const;
    QBrush lightInactive() const;
    QBrush lightDisable() const;
    QBrush midlightActive() const;
    QBrush midlightInactive() const;
    QBrush midlightDisable() const;
    QBrush darkActive() const;
    QBrush darkInactive() const;
    QBrush darkDisable() const;
    QBrush midActive() const;
    QBrush midInactive() const;
    QBrush midDisable() const;
    QBrush textActive() const;
    QBrush textInactive() const;
    QBrush textDisable() const;
    QBrush brightTextActive() const;
    QBrush brightTextInactive() const;
    QBrush brightTextDisable() const;
    QBrush buttonTextActive() const;
    QBrush buttonTextInactive() const;
    QBrush buttonTextDisable() const;
    QBrush baseActive() const;
    QBrush baseInactive() const;
    QBrush baseDisable() const;
    QBrush windowActive() const;
    QBrush windowInactive() const;
    QBrush windowDisable() const;
    QBrush shadowActive() const;
    QBrush shadowInactive() const;
    QBrush shadowDisable() const;
    QBrush highLightActive() const;
    QBrush highLightInactive() const;
    QBrush highLightDisable() const;
    QBrush highLightedTextActive() const;
    QBrush highLightedTextInactive() const;
    QBrush highLightedTextDisable() const;
    QBrush linkActive() const;
    QBrush linkInactive() const;
    QBrush linkDisable() const;
    QBrush linkVisitedActive() const;
    QBrush linkVisitedInactive() const;
    QBrush linkVisitedDisable() const;
    QBrush alternateBaseActive() const;
    QBrush alternateBaseInactive() const;
    QBrush alternateBaseDisable() const;
    QBrush noRoleActive() const;
    QBrush noRoleInactive() const;
    QBrush noRoleDisable() const;
    QBrush toolTipBaseActive() const;
    QBrush toolTipBaseInactive() const;
    QBrush toolTipBaseDisable() const;
    QBrush toolTipTextActive() const;
    QBrush toolTipTextInactive() const;
    QBrush toolTipTextDisable() const;
    QBrush placeholderTextActive() const;
    QBrush placeholderTextInactive() const;
    QBrush placeholderTextDisable() const;

    //custompalette
    QBrush kBlack() const;
    QBrush kBrand1() const;
    QBrush kBrand2() const;
    QBrush kBrand3() const;
    QBrush kBrand4() const;
    QBrush kBrand5() const;
    QBrush kBrand6() const;
    QBrush kBrand7() const;
    QBrush kBrandClick() const;
    QBrush kBrandFocus() const;
    QBrush kBrandHover() const;
    QBrush kBrandNormal() const;
    QBrush kComponentAlphaClick() const;
    QBrush kComponentAlphaDisable() const;
    QBrush kComponentAlphaHover() const;
    QBrush kComponentAlphaNormal() const;
    QBrush kComponentClick() const;
    QBrush kComponentDisable() const;
    QBrush kComponentHover() const;
    QBrush kComponentNormal() const;

    QBrush kComponentInput() const;
    QBrush kComponentInputAlpha() const;

    QBrush kComponentSelectedAlphaDisable() const;
    QBrush kComponentSelectedAlphaNormal() const;
    QBrush kComponentSelectedAlphaHover() const;
    QBrush kComponentSelectedAlphaClick() const;

    QBrush kComponentSelectedDisable() const;
    QBrush kComponentSelectedNormal() const;
    QBrush kComponentSelectedHover() const;
    QBrush kComponentSelectedClick() const;

    QBrush kContainClick() const;
    QBrush kContainHover() const;
    QBrush kContainGeneralNormal() const;
    QBrush kContainSecondaryNormal() const;
    QBrush kErrorClick() const;
    QBrush kErrorHover() const;
    QBrush kErrorNormal() const;
    QBrush kFontPrimary() const;
    QBrush kFontPrimaryDisable() const;
    QBrush kFontSecondary() const;
    QBrush kFontSecondaryDisable() const;
    QBrush kFontStrong() const;
    QBrush kFontWhite() const;
    QBrush kFontWhiteDisable() const;
    QBrush kFontWhiteSecondary() const;
    QBrush kFontWhiteSecondaryDisable() const;
    QBrush kGray0() const;
    QBrush kGray1() const;
    QBrush kGray2() const;
    QBrush kGray3() const;
    QBrush kGray4() const;
    QBrush kGray5() const;
    QBrush kGray6() const;
    QBrush kGray7() const;
    QBrush kGray8() const;
    QBrush kGray9() const;
    QBrush kGray10() const;
    QBrush kGray11() const;
    QBrush kGray12() const;
    QBrush kGray13() const;
    QBrush kGray14() const;
    QBrush kGray15() const;
    QBrush kGray16() const;
    QBrush kGray17() const;
    QBrush kGrayAlpha0() const;
    QBrush kGrayAlpha1() const;
    QBrush kGrayAlpha2() const;
    QBrush kGrayAlpha3() const;
    QBrush kGrayAlpha4() const;
    QBrush kGrayAlpha5() const;
    QBrush kGrayAlpha6() const;
    QBrush kGrayAlpha7() const;
    QBrush kGrayAlpha8() const;
    QBrush kGrayAlpha9() const;
    QBrush kGrayAlpha10() const;
    QBrush kGrayAlpha11() const;
    QBrush kGrayAlpha12() const;
    QBrush kGrayAlpha13() const;
    QBrush kLineBrandClick() const;
    QBrush kLineBrandDisable() const;
    QBrush kLineBrandHover() const;
    QBrush kLineBrandNormal() const;
    QBrush kLineComponentClick() const;
    QBrush kLineComponentDisable() const;
    QBrush kLineComponentHover() const;
    QBrush kLineComponentNormal() const;
    QBrush kLineDisable() const;
    QBrush kLineNormal() const;
    QBrush kLineWindowActive() const;
    QBrush kModalmask() const;
    QBrush kSuccessNormal() const;
    QBrush kWarningNormal() const;
    QBrush kWhite() const;

    //line and margin
    int normalline() const;
    int focusline() const;

    //radius
    int kradiusMin() const;
    int kradiusMenu() const;
    int kradiusNormal() const;
    int kradiusWindow() const;

    QBrush kBrand8() const;

    QBrush kBrand9() const;

    QBrush kBrandDisable() const;

    QBrush kContainAlphaClick() const;

    QBrush kContainAlphaHover() const;

    QBrush kContainGeneralAlphaNormal() const;

    QBrush kContainGeneralInactive() const;

    QBrush kContainSecondaryAlphaNormal() const;

    QBrush kContainSecondaryInactive() const;

    QBrush kDivider() const;

    QBrush kDividerWhite() const;

    QBrush kErrorDisable() const;

    QBrush kFontPlaceholdertextDisable() const;

    QBrush kFontPlaceholdertext() const;

    QBrush kFontWhitePlaceholdertextDisable() const;

    QBrush kFontWhiteplaceholdertext() const;

    QBrush kGreen1() const;

    QBrush kLineDock() const;

    QBrush kLineErrorClick() const;

    QBrush kLineErrorDisable() const;

    QBrush kLineErrorHover() const;

    QBrush kLineErrorNormal() const;

    QBrush kLineInputClick() const;

    QBrush kLineInputDisable() const;

    QBrush kLineInputHover() const;

    QBrush kLineInputNormal() const;

    QBrush kLineMenu() const;

    QBrush kLineSelectboxClick() const;

    QBrush kLineSelectboxDisable() const;

    QBrush kLineSelectboxHover() const;

    QBrush kLineSelectboxNormal() const;

    QBrush kLineSelectboxSelected() const;

    QBrush kLineSuccessClick() const;

    QBrush kLineSuccessDisable() const;

    QBrush kLineSuccessHover() const;

    QBrush kLineSuccessNormal() const;

    QBrush kLineTable() const;

    QBrush kLineWarningClick() const;

    QBrush kLineWarningDisable() const;

    QBrush kLineWarningHover() const;

    QBrush kLineWarningNormal() const;

    QBrush kLineWindowInactive() const;

    QBrush kMenu() const;

    QBrush kOrange1() const;

    int kradiusMax() const;

    QBrush kRed1() const;

    QBrush kSelectAlphaWhiteClick() const;

    QBrush kSelectAlphaWhiteDisable() const;

    QBrush kSelectAlphaWhiteHover() const;

    QBrush kSelectAlphaWhite() const;

    QBrush kshadowComponent() const;

    QBrush kshadowMenu() const;

    QBrush kshadowMin() const;

    QBrush kshadowPrimaryActive() const;

    QBrush kshadowPrimaryInactive() const;

    QBrush kshadowSecondaryActive() const;

    QBrush kshadowSecondaryInactive() const;

    QBrush kSuccessClick() const;

    QBrush kSuccessDisable() const;

    QBrush kSuccessHover() const;

    QBrush kWarningClick() const;

    QBrush kWarningDisable() const;

    QBrush kWarningHover() const;

    QBrush shadow() const;

    QPalette appPalette();

    void init();

    /** 从设计令牌 CSS 加载并解析；cfgPathOrUri 可为本地路径或 file:// URL（如 file:///usr/share/config/themeconfig/token/kdefault-light.css）。 */
    bool loadDesignTokenFile(const QString &cfgPathOrUri);

Q_SIGNALS:
    void tokenChanged();

private:

    GlobalDTConfigPrivate *d {nullptr};
};

}

Q_DECLARE_METATYPE(QBrush*)
Q_DECLARE_METATYPE(QSharedPointer<QBrush>)
Q_DECLARE_METATYPE(int*)
Q_DECLARE_METATYPE(QSharedPointer<int>)

#endif // READCONFIG_H
