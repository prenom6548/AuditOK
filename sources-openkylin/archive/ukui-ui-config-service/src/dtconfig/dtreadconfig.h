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

#ifndef DTTMPREADCONFIG_H
#define DTTMPREADCONFIG_H

#include <QObject>
#include <QColor>
#include <QPalette>
#include <QMetaType>


class DTThemeData : public QObject
{
    Q_OBJECT

private:

    typedef struct {
        QColor  startColor;
        QColor  endColor;
    } DTGradient;

    typedef struct{
        QColor color;
        DTGradient gradient;
        int intValue;
    } DTData;

public:

    typedef enum _DTDateType
    {
        None = 0,               //error
        Integer = 1,            //unsigned integer data
        Color,                  //solid color
        Gradient                //gradient color
    }DTDateType;

    DTThemeData(QColor startColor,
            QColor endColor,
            DTDateType dtType = DTDateType::None,
            QObject *parent = nullptr);

    DTThemeData(QColor solidColor = QColor(),
            DTDateType colorType = DTDateType::None,
            QObject *parent = nullptr);

    DTThemeData(int inValue,
            DTDateType colorType = DTDateType::None,
            QObject *parent = nullptr);

    DTThemeData(const DTThemeData& other);

    DTThemeData& operator=(const DTThemeData &other);

    bool operator==(const DTThemeData &other);
    bool operator!=(const DTThemeData &other);

    void setType(DTDateType type);
    DTDateType dateType();

    void setColor(QColor color);
    void setStartColor(QColor startColor);
    void setEndColor(QColor endColor);
    void setIntValue(int value);

    QColor color();
    QColor startColor();
    QColor endColor();
    int intValue();

    QColor mixColor(QList<QColor> colorList);
    QColor composeColor(QColor foreColor, QColor backColor);

private:
    DTDateType m_dtType = DTDateType::None;
    DTData m_dtDate;
};

class GlobalDTConfigPrivate;

namespace UKUIThemeConfigDaemon {


class GlobalDTConfig : public QObject
{
    Q_OBJECT

    //QT palette
    Q_PROPERTY(DTThemeData windowTextActive          READ windowTextActive           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData windowTextInactive        READ windowTextInactive         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData windowTextDisable         READ windowTextDisable          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonActive              READ buttonActive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonInactive            READ buttonInactive             NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonDisable             READ buttonDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData lightActive               READ lightActive                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData lightInactive             READ lightInactive              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData lightDisable              READ lightDisable               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midlightActive            READ midlightActive             NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midlightInactive          READ midlightInactive           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midlightDisable           READ midlightDisable            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData darkActive                READ darkActive                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData darkInactive              READ darkInactive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData darkDisable               READ darkDisable                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midActive                 READ midActive                  NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midInactive               READ midInactive                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData midDisable                READ midDisable                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData textActive                READ textActive                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData textInactive              READ textInactive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData textDisable               READ textDisable                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData brightTextActive          READ brightTextActive           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData brightTextInactive        READ brightTextInactive         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData brightTextDisable         READ brightTextDisable          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonTextActive          READ buttonTextActive           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonTextInactive        READ buttonTextInactive         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData buttonTextDisable         READ buttonTextDisable          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData baseActive                READ baseActive                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData baseInactive              READ baseInactive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData baseDisable               READ baseDisable                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData windowActive              READ windowActive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData windowInactive            READ windowInactive             NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData windowDisable             READ windowDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData shadowActive              READ shadowActive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData shadowInactive            READ shadowInactive             NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData shadowDisable             READ shadowDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightActive           READ highLightActive            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightInactive         READ highLightInactive          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightDisable          READ highLightDisable           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightedTextActive     READ highLightedTextActive      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightedTextInactive   READ highLightedTextInactive    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData highLightedTextDisable    READ highLightedTextDisable     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkActive                READ linkActive                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkInactive              READ linkInactive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkDisable               READ linkDisable                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkVisitedActive         READ linkVisitedActive          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkVisitedInactive       READ linkVisitedInactive        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData linkVisitedDisable        READ linkVisitedDisable         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData alternateBaseActive       READ alternateBaseActive        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData alternateBaseInactive     READ alternateBaseInactive      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData alternateBaseDisable      READ alternateBaseDisable       NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData noRoleActive              READ noRoleActive               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData noRoleInactive            READ noRoleInactive             NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData noRoleDisable             READ noRoleDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipBaseActive         READ toolTipBaseActive          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipBaseInactive       READ toolTipBaseInactive        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipBaseDisable        READ toolTipBaseDisable         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipTextActive         READ toolTipTextActive          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipTextInactive       READ toolTipTextInactive        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData toolTipTextDisable        READ toolTipTextDisable         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData placeholderTextActive     READ placeholderTextActive      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData placeholderTextInactive   READ placeholderTextInactive    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData placeholderTextDisable    READ placeholderTextDisable     NOTIFY tokenChanged)

    //custom palette
    Q_PROPERTY(DTThemeData kBlack                        READ kBlack                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand1                       READ kBrand1                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand2                       READ kBrand2                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand3                       READ kBrand3                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand4                       READ kBrand4                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand5                       READ kBrand5                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand6                       READ kBrand6                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrand7                       READ kBrand7                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrandClick                   READ kBrandClick                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrandFocus                   READ kBrandFocus                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrandHover                   READ kBrandHover                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kBrandNormal                  READ kBrandNormal                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentAlphaClick          READ kComponentAlphaClick           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentAlphaDisable        READ kComponentAlphaDisable         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentAlphaHover          READ kComponentAlphaHover           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentAlphaNormal         READ kComponentAlphaNormal          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentClick               READ kComponentClick                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentDisable             READ kComponentDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentHover               READ kComponentHover                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kComponentNormal              READ kComponentNormal               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kContainClick                 READ kContainClick                  NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kContainHover                 READ kContainHover                  NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kContainGeneralNormal         READ kContainGeneralNormal          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kContainSecondaryAlpphaNormal READ kContainSecondaryAlpphaNormal  NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kContainSecondaryNormal       READ kContainSecondaryNormal        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kErrorClick                   READ kErrorClick                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kErrorHover                   READ kErrorHover                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kErrorNormal                  READ kErrorNormal                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontPrimary                  READ kFontPrimary                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontPrimaryDisable           READ kFontPrimaryDisable            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontSecondary                READ kFontSecondary                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontSecondaryDisable         READ kFontSecondaryDisable          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontStrong                   READ kFontStrong                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontWhite                    READ kFontWhite                     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontWhiteDisable             READ kFontWhiteDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontWhiteSecondary           READ kFontWhiteSecondary            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kFontWhiteSecondaryDisable    READ kFontWhiteSecondaryDisable     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray0                        READ kGray0                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray1                        READ kGray1                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray2                        READ kGray2                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray3                        READ kGray3                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray4                        READ kGray4                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray5                        READ kGray5                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray6                        READ kGray6                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray7                        READ kGray7                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray8                        READ kGray8                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray9                        READ kGray9                         NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray10                       READ kGray10                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray11                       READ kGray11                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray12                       READ kGray12                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray13                       READ kGray13                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray14                       READ kGray14                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray15                       READ kGray15                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray16                       READ kGray16                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGray17                       READ kGray17                        NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha0                   READ kGrayAlpha0                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha1                   READ kGrayAlpha1                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha2                   READ kGrayAlpha2                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha3                   READ kGrayAlpha3                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha4                   READ kGrayAlpha4                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha5                   READ kGrayAlpha5                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha6                   READ kGrayAlpha6                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha7                   READ kGrayAlpha7                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha8                   READ kGrayAlpha8                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha9                   READ kGrayAlpha9                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha10                  READ kGrayAlpha10                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha11                  READ kGrayAlpha11                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha12                  READ kGrayAlpha12                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kGrayAlpha13                  READ kGrayAlpha13                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineBrandClick               READ kLineBrandClick                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineBrandDisable             READ kLineBrandDisable              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineBrandHover               READ kLineBrandHover                NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineBrandNormal              READ kLineBrandNormal               NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineComponentClick           READ kLineComponentClick            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineComponentDisable         READ kLineComponentDisable          NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineComponentHover           READ kLineComponentHover            NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineComponentNormal          READ kLineComponentNormal           NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineDisable                  READ kLineDisable                   NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineNormal                   READ kLineNormal                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineWindow                   READ kLineWindow                    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kLineWindowActive             READ kLineWindowActive              NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kModalmask                    READ kModalmask                     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kSuccessNormal                READ kSuccessNormal                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kWarningNormal                READ kWarningNormal                 NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kWhite                        READ kWhite                         NOTIFY tokenChanged)

    //line and margin
    Q_PROPERTY(DTThemeData normalline       READ normalline     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData focusline        READ focusline      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData tokenGap4        READ tokenGap4      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData tokenGap8        READ tokenGap8      NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData tokenGap16       READ tokenGap16     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData tokenGap24       READ tokenGap24     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData tokenGap40       READ tokenGap40     NOTIFY tokenChanged)

    //radius
    Q_PROPERTY(DTThemeData kradiusMin       READ kradiusMin     NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kradiusMenu      READ kradiusMenu    NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kradiusNormal    READ kradiusNormal  NOTIFY tokenChanged)
    Q_PROPERTY(DTThemeData kradiusWindow    READ kradiusWindow  NOTIFY tokenChanged)

public:
    explicit GlobalDTConfig();
    ~GlobalDTConfig();

    //QT palette
    DTThemeData windowTextActive() const;
    DTThemeData windowTextInactive() const;
    DTThemeData windowTextDisable() const;
    DTThemeData buttonActive() const;
    DTThemeData buttonInactive() const;
    DTThemeData buttonDisable() const;
    DTThemeData lightActive() const;
    DTThemeData lightInactive() const;
    DTThemeData lightDisable() const;
    DTThemeData midlightActive() const;
    DTThemeData midlightInactive() const;
    DTThemeData midlightDisable() const;
    DTThemeData darkActive() const;
    DTThemeData darkInactive() const;
    DTThemeData darkDisable() const;
    DTThemeData midActive() const;
    DTThemeData midInactive() const;
    DTThemeData midDisable() const;
    DTThemeData textActive() const;
    DTThemeData textInactive() const;
    DTThemeData textDisable() const;
    DTThemeData brightTextActive() const;
    DTThemeData brightTextInactive() const;
    DTThemeData brightTextDisable() const;
    DTThemeData buttonTextActive() const;
    DTThemeData buttonTextInactive() const;
    DTThemeData buttonTextDisable() const;
    DTThemeData baseActive() const;
    DTThemeData baseInactive() const;
    DTThemeData baseDisable() const;
    DTThemeData windowActive() const;
    DTThemeData windowInactive() const;
    DTThemeData windowDisable() const;
    DTThemeData shadowActive() const;
    DTThemeData shadowInactive() const;
    DTThemeData shadowDisable() const;
    DTThemeData highLightActive() const;
    DTThemeData highLightInactive() const;
    DTThemeData highLightDisable() const;
    DTThemeData highLightedTextActive() const;
    DTThemeData highLightedTextInactive() const;
    DTThemeData highLightedTextDisable() const;
    DTThemeData linkActive() const;
    DTThemeData linkInactive() const;
    DTThemeData linkDisable() const;
    DTThemeData linkVisitedActive() const;
    DTThemeData linkVisitedInactive() const;
    DTThemeData linkVisitedDisable() const;
    DTThemeData alternateBaseActive() const;
    DTThemeData alternateBaseInactive() const;
    DTThemeData alternateBaseDisable() const;
    DTThemeData noRoleActive() const;
    DTThemeData noRoleInactive() const;
    DTThemeData noRoleDisable() const;
    DTThemeData toolTipBaseActive() const;
    DTThemeData toolTipBaseInactive() const;
    DTThemeData toolTipBaseDisable() const;
    DTThemeData toolTipTextActive() const;
    DTThemeData toolTipTextInactive() const;
    DTThemeData toolTipTextDisable() const;
    DTThemeData placeholderTextActive() const;
    DTThemeData placeholderTextInactive() const;
    DTThemeData placeholderTextDisable() const;

    //custompalette
    DTThemeData kBlack() const;
    DTThemeData kBrand1() const;
    DTThemeData kBrand2() const;
    DTThemeData kBrand3() const;
    DTThemeData kBrand4() const;
    DTThemeData kBrand5() const;
    DTThemeData kBrand6() const;
    DTThemeData kBrand7() const;
    DTThemeData kBrandClick() const;
    DTThemeData kBrandFocus() const;
    DTThemeData kBrandHover() const;
    DTThemeData kBrandNormal() const;
    DTThemeData kComponentAlphaClick() const;
    DTThemeData kComponentAlphaDisable() const;
    DTThemeData kComponentAlphaHover() const;
    DTThemeData kComponentAlphaNormal() const;
    DTThemeData kComponentClick() const;
    DTThemeData kComponentDisable() const;
    DTThemeData kComponentHover() const;
    DTThemeData kComponentNormal() const;
    DTThemeData kContainClick() const;
    DTThemeData kContainHover() const;
    DTThemeData kContainGeneralNormal() const;
    DTThemeData kContainSecondaryAlpphaNormal() const;
    DTThemeData kContainSecondaryNormal() const;
    DTThemeData kErrorClick() const;
    DTThemeData kErrorHover() const;
    DTThemeData kErrorNormal() const;
    DTThemeData kFontPrimary() const;
    DTThemeData kFontPrimaryDisable() const;
    DTThemeData kFontSecondary() const;
    DTThemeData kFontSecondaryDisable() const;
    DTThemeData kFontStrong() const;
    DTThemeData kFontWhite() const;
    DTThemeData kFontWhiteDisable() const;
    DTThemeData kFontWhiteSecondary() const;
    DTThemeData kFontWhiteSecondaryDisable() const;
    DTThemeData kGray0() const;
    DTThemeData kGray1() const;
    DTThemeData kGray2() const;
    DTThemeData kGray3() const;
    DTThemeData kGray4() const;
    DTThemeData kGray5() const;
    DTThemeData kGray6() const;
    DTThemeData kGray7() const;
    DTThemeData kGray8() const;
    DTThemeData kGray9() const;
    DTThemeData kGray10() const;
    DTThemeData kGray11() const;
    DTThemeData kGray12() const;
    DTThemeData kGray13() const;
    DTThemeData kGray14() const;
    DTThemeData kGray15() const;
    DTThemeData kGray16() const;
    DTThemeData kGray17() const;
    DTThemeData kGrayAlpha0() const;
    DTThemeData kGrayAlpha1() const;
    DTThemeData kGrayAlpha2() const;
    DTThemeData kGrayAlpha3() const;
    DTThemeData kGrayAlpha4() const;
    DTThemeData kGrayAlpha5() const;
    DTThemeData kGrayAlpha6() const;
    DTThemeData kGrayAlpha7() const;
    DTThemeData kGrayAlpha8() const;
    DTThemeData kGrayAlpha9() const;
    DTThemeData kGrayAlpha10() const;
    DTThemeData kGrayAlpha11() const;
    DTThemeData kGrayAlpha12() const;
    DTThemeData kGrayAlpha13() const;
    DTThemeData kLineBrandClick() const;
    DTThemeData kLineBrandDisable() const;
    DTThemeData kLineBrandHover() const;
    DTThemeData kLineBrandNormal() const;
    DTThemeData kLineComponentClick() const;
    DTThemeData kLineComponentDisable() const;
    DTThemeData kLineComponentHover() const;
    DTThemeData kLineComponentNormal() const;
    DTThemeData kLineDisable() const;
    DTThemeData kLineNormal() const;
    DTThemeData kLineWindow() const;
    DTThemeData kLineWindowActive() const;
    DTThemeData kModalmask() const;
    DTThemeData kSuccessNormal() const;
    DTThemeData kWarningNormal() const;
    DTThemeData kWhite() const;

    //line and margin
    DTThemeData normalline() const;
    DTThemeData focusline() const;
    DTThemeData tokenGap4() const;
    DTThemeData tokenGap8() const;
    DTThemeData tokenGap16() const;
    DTThemeData tokenGap24() const;
    DTThemeData tokenGap40() const;

    //radius
    DTThemeData kradiusMin() const;
    DTThemeData kradiusMenu() const;
    DTThemeData kradiusNormal() const;
    DTThemeData kradiusWindow() const;

    QMap<QString, DTThemeData> getDTDateMap() const;
Q_SIGNALS:
    void tokenChanged();

private:

    GlobalDTConfigPrivate *d {nullptr};
};

}

Q_DECLARE_METATYPE(DTThemeData)
#endif // READCONFIG_H
