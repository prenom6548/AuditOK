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

#include "dtreadconfig.h"
#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QGSettings>
#include <QMetaEnum>
#include <QCoreApplication>
#include <QBrush>
#include <QPainter>
#include <QLoggingCategory>


Q_LOGGING_CATEGORY(DTReadConfig, "dt-readConfig")

#define ThemeConfigPath "/usr/share/config/globaltheme/token/"

using namespace UKUIThemeConfigDaemon;

class GlobalDTConfigPrivate
{
public:
    explicit GlobalDTConfigPrivate();
    ~GlobalDTConfigPrivate();

    inline bool getColorValue(DTThemeData &DTThemeData, const QString key);
    inline bool getValue(DTThemeData &value, const QString key);
    inline QColor stringToColor(QString s) const;

    bool load();
    void initUKUIGlobalThemeParameters();

    //qt palette
    DTThemeData windowTextActive         ;
    DTThemeData windowTextInactive       ;
    DTThemeData windowTextDisable        ;
    DTThemeData buttonActive             ;
    DTThemeData buttonInactive           ;
    DTThemeData buttonDisable            ;
    DTThemeData lightActive              ;
    DTThemeData lightInactive            ;
    DTThemeData lightDisable             ;
    DTThemeData midlightActive           ;
    DTThemeData midlightInactive         ;
    DTThemeData midlightDisable          ;
    DTThemeData darkActive               ;
    DTThemeData darkInactive             ;
    DTThemeData darkDisable              ;
    DTThemeData midActive                ;
    DTThemeData midInactive              ;
    DTThemeData midDisable               ;
    DTThemeData textActive               ;
    DTThemeData textInactive             ;
    DTThemeData textDisable              ;
    DTThemeData brightTextActive         ;
    DTThemeData brightTextInactive       ;
    DTThemeData brightTextDisable        ;
    DTThemeData buttonTextActive         ;
    DTThemeData buttonTextInactive       ;
    DTThemeData buttonTextDisable        ;
    DTThemeData baseActive               ;
    DTThemeData baseInactive             ;
    DTThemeData baseDisable              ;
    DTThemeData windowActive             ;
    DTThemeData windowInactive           ;
    DTThemeData windowDisable            ;
    DTThemeData shadowActive             ;
    DTThemeData shadowInactive           ;
    DTThemeData shadowDisable            ;
    DTThemeData highLightActive          ;
    DTThemeData highLightInactive        ;
    DTThemeData highLightDisable         ;
    DTThemeData highLightedTextActive    ;
    DTThemeData highLightedTextInactive  ;
    DTThemeData highLightedTextDisable   ;
    DTThemeData linkActive               ;
    DTThemeData linkInactive             ;
    DTThemeData linkDisable              ;
    DTThemeData linkVisitedActive        ;
    DTThemeData linkVisitedInactive      ;
    DTThemeData linkVisitedDisable       ;
    DTThemeData alternateBaseActive      ;
    DTThemeData alternateBaseInactive    ;
    DTThemeData alternateBaseDisable     ;
    DTThemeData noRoleActive             ;
    DTThemeData noRoleInactive           ;
    DTThemeData noRoleDisable            ;
    DTThemeData toolTipBaseActive        ;
    DTThemeData toolTipBaseInactive      ;
    DTThemeData toolTipBaseDisable       ;
    DTThemeData toolTipTextActive        ;
    DTThemeData toolTipTextInactive      ;
    DTThemeData toolTipTextDisable       ;
    DTThemeData placeholderTextActive    ;
    DTThemeData placeholderTextInactive  ;
    DTThemeData placeholderTextDisable   ;

    //custom palette
    DTThemeData kComponentNormal                 ;
    DTThemeData kLineBrandDisable                ;
    DTThemeData kFontWhiteSecondary              ;
    DTThemeData kGray12                          ;
    DTThemeData kContainSecondaryAlpphaNormal    ;
    DTThemeData kGray16                          ;
    DTThemeData kBrand1                          ;
    DTThemeData kLineComponentNormal             ;
    DTThemeData kLineBrandClick                  ;
    DTThemeData kGray3                           ;
    DTThemeData kGray0                           ;
    DTThemeData kLineDisable                     ;
    DTThemeData kGrayAlpha2                      ;
    DTThemeData kGrayAlpha0                      ;
    DTThemeData kBrand2                          ;
    DTThemeData kLineNormal                      ;
    DTThemeData kGrayAlpha5                      ;
    DTThemeData kGrayAlpha12                     ;
    DTThemeData kErrorClick                      ;
    DTThemeData kLineWindowActive               ;
    DTThemeData kComponentAlphaClick             ;
    DTThemeData kBrand3                          ;
    DTThemeData kSuccessNormal                   ;
    DTThemeData kGrayAlpha1                      ;
    DTThemeData kFontWhiteSecondaryDisable       ;
    DTThemeData kGray4                           ;
    DTThemeData kContainClick                    ;
    DTThemeData kGray8                           ;
    DTThemeData kGrayAlpha4                      ;
    DTThemeData kModalmask                       ;
    DTThemeData kLineBrandNormal                 ;
    DTThemeData kBrand5                          ;
    DTThemeData kGray10                          ;
    DTThemeData kWarningNormal                   ;
    DTThemeData kGray9                           ;
    DTThemeData kGray1                           ;
    DTThemeData kGray14                          ;
    DTThemeData kLineComponentClick              ;
    DTThemeData kWhite                           ;
    DTThemeData kGray7                           ;
    DTThemeData kBrandFocus                      ;
    DTThemeData kGrayAlpha9                      ;
    DTThemeData kGrayAlpha10                     ;
    DTThemeData kGrayAlpha6                      ;
    DTThemeData kContainHover                    ;
    DTThemeData kLineComponentDisable            ;
    DTThemeData kFontWhite                       ;
    DTThemeData kGrayAlpha11                     ;
    DTThemeData kGray6                           ;
    DTThemeData kFontWhiteDisable                ;
    DTThemeData kComponentDisable                ;
    DTThemeData kBlack                           ;
    DTThemeData kComponentAlphaDisable           ;
    DTThemeData kFontSecondaryDisable            ;
    DTThemeData kGrayAlpha8                      ;
    DTThemeData kFontPrimaryDisable              ;
    DTThemeData kGray5                           ;
    DTThemeData kFontSecondary                   ;
    DTThemeData kErrorNormal                     ;
    DTThemeData kComponentHover                  ;
    DTThemeData kComponentAlphaHover             ;
    DTThemeData kGrayAlpha3                      ;
    DTThemeData kGray13                          ;
    DTThemeData kGray11                          ;
    DTThemeData kErrorHover                      ;
    DTThemeData kContainGeneralNormal            ;
    DTThemeData kComponentAlphaNormal            ;
    DTThemeData kGray15                          ;
    DTThemeData kBrand7                          ;
    DTThemeData kGrayAlpha7                      ;
    DTThemeData kLineBrandHover                  ;
    DTThemeData kGrayAlpha13                     ;
    DTThemeData kLineComponentHover              ;
    DTThemeData kBrand6                          ;
    DTThemeData kGray17                          ;
    DTThemeData kComponentClick                  ;
    DTThemeData kBrandClick                      ;
    DTThemeData kFontStrong                      ;
    DTThemeData kLineWindow                      ;
    DTThemeData kGray2                           ;
    DTThemeData kBrand4                          ;
    DTThemeData kContainSecondaryNormal          ;
    DTThemeData kBrandHover                      ;
    DTThemeData kFontPrimary                     ;
    DTThemeData kBrandNormal                     ;

    //line and margin
    DTThemeData normalline      ;
    DTThemeData focusline       ;
    DTThemeData tokenGap4       ;
    DTThemeData tokenGap8       ;
    DTThemeData tokenGap16      ;
    DTThemeData tokenGap24      ;
    DTThemeData tokenGap40      ;

    //radius
    DTThemeData kradiusMin      ;
    DTThemeData kradiusMenu     ;
    DTThemeData kradiusNormal   ;
    DTThemeData kradiusWindow   ;

    bool m_ukui_dt_loadConfig = false;
    QString m_cfgPath;
    QString m_widgetThemeName = "default";
    QString m_lightMode = "light";
    QString m_dt_configData;
    QMap<QString, DTThemeData> dtDataMap;
};


DTThemeData::DTThemeData(QColor startColor, QColor endColor, DTDateType dtType, QObject *parent)
{
    m_dtType = dtType;
    m_dtDate.gradient.startColor = startColor;
    m_dtDate.gradient.endColor = endColor;
}

DTThemeData::DTThemeData(QColor solidColor, DTDateType dtType, QObject *parent)
{
    m_dtType = dtType;
    m_dtDate.color = solidColor;
}

DTThemeData::DTThemeData(int inValue, DTDateType dtType, QObject *parent)
{
    m_dtType = dtType;
    m_dtDate.intValue = inValue;
}

DTThemeData::DTThemeData(const DTThemeData &other)
{
    if(this != &other){
        this->m_dtType = other.m_dtType;
        this->m_dtDate.color = other.m_dtDate.color;
        this->m_dtDate.gradient.startColor = other.m_dtDate.gradient.startColor;
        this->m_dtDate.gradient.endColor = other.m_dtDate.gradient.endColor;
        this->m_dtDate.intValue = other.m_dtDate.intValue;
    }
}

DTThemeData& DTThemeData::operator=(const DTThemeData &other)
{
    if (this != &other) {
        this->m_dtType = other.m_dtType;
        this->m_dtDate.color = other.m_dtDate.color;
        this->m_dtDate.gradient.startColor = other.m_dtDate.gradient.startColor;
        this->m_dtDate.gradient.endColor = other.m_dtDate.gradient.endColor;
        this->m_dtDate.intValue = other.m_dtDate.intValue;
    }
    return *this;
}

bool DTThemeData::operator==(const DTThemeData &other)
{
    return (this->m_dtType == other.m_dtType &&
            this->m_dtDate.color == other.m_dtDate.color &&
            this->m_dtDate.gradient.startColor == other.m_dtDate.gradient.startColor &&
            this->m_dtDate.gradient.endColor == other.m_dtDate.gradient.endColor &&
            this->m_dtDate.intValue == other.m_dtDate.intValue);
}

bool DTThemeData::operator!=(const DTThemeData &other)
{
    return !(this->m_dtType == other.m_dtType &&
             this->m_dtDate.color == other.m_dtDate.color &&
             this->m_dtDate.gradient.startColor == other.m_dtDate.gradient.startColor &&
             this->m_dtDate.gradient.endColor == other.m_dtDate.gradient.endColor &&
             this->m_dtDate.intValue == other.m_dtDate.intValue);
}

void DTThemeData::setType(DTDateType type)
{
    m_dtType = type;
}

void DTThemeData::setColor(QColor color)
{
    m_dtDate.color = color;
}

void DTThemeData::setStartColor(QColor startColor)
{
    m_dtDate.gradient.startColor = startColor;
}

void DTThemeData::setEndColor(QColor endColor)
{
    m_dtDate.gradient.endColor = endColor;
}

void DTThemeData::setIntValue(int value)
{
    m_dtDate.intValue = value;
}

DTThemeData::DTDateType DTThemeData::dateType()
{
    return m_dtType;
}

QColor DTThemeData::startColor()
{
    return m_dtDate.gradient.startColor;
}

QColor DTThemeData::endColor()
{
    return m_dtDate.gradient.endColor;
}

QColor DTThemeData::color()
{
    return m_dtDate.color;
}

int DTThemeData::intValue()
{
    return m_dtDate.intValue;
}

QColor DTThemeData::mixColor(QList<QColor> colorList)
{
    //bottom color is the last one on the list.
    QColor color = colorList.takeLast();

    foreach (QColor c, colorList) {
        color = composeColor(c, color);
    }

    return color;
}

QColor DTThemeData::composeColor(QColor foreColor, QColor backColor)
{
    qreal alphaF = foreColor.alphaF() * 1.0 +  1.0 *backColor.alphaF() * (1 - foreColor.alphaF());
    qreal red = (foreColor.red() * foreColor.alphaF() * 1.0+ backColor.red() * backColor.alphaF() *(1 - foreColor.alphaF()))/(alphaF * 1.0);
    qreal green = (foreColor.green() * foreColor.alphaF() * 1.0 + backColor.green() * backColor.alphaF() *(1 - foreColor.alphaF()))/(alphaF * 1.0);
    qreal blue = (foreColor.blue() * foreColor.alphaF() * 1.0 + backColor.blue() * backColor.alphaF() *(1 - foreColor.alphaF()))/(alphaF * 1.0);

    QColor color;
    color.setRed(red);
    color.setGreen(green);
    color.setBlue(blue);
    color.setAlphaF(alphaF);

    return color;
}

GlobalDTConfigPrivate::GlobalDTConfigPrivate()
{
    {
        windowTextActive         = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        windowTextInactive       = DTThemeData(QColor(115, 115, 115, 1 * 255), DTThemeData::DTDateType::Color);
        windowTextDisable        = DTThemeData(QColor(75, 175, 175, 1 * 255) , DTThemeData::DTDateType::Color);
        buttonActive             = DTThemeData(QColor(230, 230, 230, 1 * 255), DTThemeData::DTDateType::Color);
        buttonInactive           = DTThemeData(QColor(230, 230, 230, 1 * 255), DTThemeData::DTDateType::Color);
        buttonDisable            = DTThemeData(QColor(238, 238, 238, 1 * 255), DTThemeData::DTDateType::Color);
        lightActive              = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        lightInactive            = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        lightDisable             = DTThemeData(QColor(250, 250, 250, 1 * 255), DTThemeData::DTDateType::Color);
        midlightActive           = DTThemeData(QColor(220, 220, 220, 1 * 255), DTThemeData::DTDateType::Color);
        midlightInactive         = DTThemeData(QColor(220, 220, 220, 1 * 255), DTThemeData::DTDateType::Color);
        midlightDisable          = DTThemeData(QColor(230, 230, 230, 1 * 255), DTThemeData::DTDateType::Color);
        darkActive               = DTThemeData(QColor(0, 0, 0, 1 * 255)      , DTThemeData::DTDateType::Color);
        darkInactive             = DTThemeData(QColor(0, 0, 0, 1 * 255)      , DTThemeData::DTDateType::Color);
        darkDisable              = DTThemeData(QColor(64, 64, 64, 1 * 255)   , DTThemeData::DTDateType::Color);
        midActive                = DTThemeData(QColor(115, 115, 115, 1 * 255), DTThemeData::DTDateType::Color);
        midInactive              = DTThemeData(QColor(115, 115, 115, 1 * 255), DTThemeData::DTDateType::Color);
        midDisable               = DTThemeData(QColor(102, 102, 102, 1 * 255), DTThemeData::DTDateType::Color);
        textActive               = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        textInactive             = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        textDisable              = DTThemeData(QColor(175, 175, 175, 1 * 255), DTThemeData::DTDateType::Color);
        brightTextActive         = DTThemeData(QColor(0, 0, 0, 1 * 255)      , DTThemeData::DTDateType::Color);
        brightTextInactive       = DTThemeData(QColor(0, 0, 0, 1 * 255)      , DTThemeData::DTDateType::Color);
        brightTextDisable        = DTThemeData(QColor(0, 0, 0, 1 * 255)      , DTThemeData::DTDateType::Color);
        buttonTextActive         = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        buttonTextInactive       = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        buttonTextDisable        = DTThemeData(QColor(175, 175, 175, 1 * 255), DTThemeData::DTDateType::Color);
        baseActive               = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        baseInactive             = DTThemeData(QColor(250, 250, 250, 1 * 255), DTThemeData::DTDateType::Color);
        baseDisable              = DTThemeData(QColor(238, 238, 238, 1 * 255), DTThemeData::DTDateType::Color);
        windowActive             = DTThemeData(QColor(246, 246, 246, 1 * 255), DTThemeData::DTDateType::Color);
        windowInactive           = DTThemeData(QColor(242, 242, 242, 1 * 255), DTThemeData::DTDateType::Color);
        windowDisable            = DTThemeData(QColor(238, 238, 238, 1 * 255), DTThemeData::DTDateType::Color);
        shadowActive             = DTThemeData(QColor(0, 0, 0, 0.4 * 255)    , DTThemeData::DTDateType::Color);
        shadowInactive           = DTThemeData(QColor(0, 0, 0, 0.25 * 255)   , DTThemeData::DTDateType::Color);
        shadowDisable            = DTThemeData(QColor(0, 0, 0, 0.25 * 255)   , DTThemeData::DTDateType::Color);
        highLightActive          = DTThemeData(QColor(55, 144, 250, 1 * 255) , DTThemeData::DTDateType::Color);
        highLightInactive        = DTThemeData(QColor(55, 144, 250, 1 * 255) , DTThemeData::DTDateType::Color);
        highLightDisable         = DTThemeData(QColor(238, 238, 238, 1 * 255), DTThemeData::DTDateType::Color);
        highLightedTextActive    = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        highLightedTextInactive  = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        highLightedTextDisable   = DTThemeData(QColor(175, 175, 175, 1 * 255), DTThemeData::DTDateType::Color);
        linkActive               = DTThemeData(QColor(55, 144, 250, 1 * 255) , DTThemeData::DTDateType::Color);
        linkInactive             = DTThemeData(QColor(55, 144, 250, 1 * 255) , DTThemeData::DTDateType::Color);
        linkDisable              = DTThemeData(QColor(175, 175, 175, 1 * 255), DTThemeData::DTDateType::Color);
        linkVisitedActive        = DTThemeData(QColor()                      );
        linkVisitedInactive      = DTThemeData(QColor()                      );
        linkVisitedDisable       = DTThemeData(QColor(175, 175, 175, 1 * 255), DTThemeData::DTDateType::Color);
        alternateBaseActive      = DTThemeData(QColor(242, 242, 242, 1 * 255), DTThemeData::DTDateType::Color);
        alternateBaseInactive    = DTThemeData(QColor(242, 242, 242, 1 * 255), DTThemeData::DTDateType::Color);
        alternateBaseDisable     = DTThemeData(QColor(242, 242, 242, 1 * 255), DTThemeData::DTDateType::Color);
        noRoleActive             = DTThemeData(QColor(250, 250, 250, 1 * 255), DTThemeData::DTDateType::Color);
        noRoleInactive           = DTThemeData(QColor(250, 250, 250, 1 * 255), DTThemeData::DTDateType::Color);
        noRoleDisable            = DTThemeData(QColor(235, 235, 235, 1 * 255), DTThemeData::DTDateType::Color);
        toolTipBaseActive        = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        toolTipBaseInactive      = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        toolTipBaseDisable       = DTThemeData(QColor(255, 255, 255, 1 * 255), DTThemeData::DTDateType::Color);
        toolTipTextActive        = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        toolTipTextInactive      = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        toolTipTextDisable       = DTThemeData(QColor(38, 38, 38, 1 * 255)   , DTThemeData::DTDateType::Color);
        placeholderTextActive    = DTThemeData(QColor(165, 165, 165, 1 * 255), DTThemeData::DTDateType::Color);
        placeholderTextInactive  = DTThemeData(QColor(165, 165, 165, 1 * 255), DTThemeData::DTDateType::Color);
        placeholderTextDisable   = DTThemeData(QColor(200, 200, 200, 1 * 255), DTThemeData::DTDateType::Color);

        kComponentNormal                 = DTThemeData(QColor(230, 230, 230, 1 * 255)   , DTThemeData::DTDateType::Color);
        kLineBrandDisable                = DTThemeData(QColor(55, 144, 250, 0 * 255)    , DTThemeData::DTDateType::Color);
        kFontWhiteSecondary              = DTThemeData(QColor(255, 255, 255, 0.65 * 255), DTThemeData::DTDateType::Color);
        kGray12                          = DTThemeData(QColor(165, 165, 165, 1 * 255)   , DTThemeData::DTDateType::Color);
        kContainSecondaryAlpphaNormal    = DTThemeData(QColor(0, 0, 0, 0.03 * 255)      , DTThemeData::DTDateType::Color);
        kGray16                          = DTThemeData(QColor(64, 64, 64, 1 * 255)      , DTThemeData::DTDateType::Color);
        kBrand1                          = DTThemeData(QColor(55, 144, 250, 1 * 255)    , DTThemeData::DTDateType::Color);
        kLineComponentNormal             = DTThemeData(QColor(0, 0, 0, 0 * 255)         , DTThemeData::DTDateType::Color);
        kLineBrandClick                  = DTThemeData(QColor(55, 144, 250, 0 * 255)    , DTThemeData::DTDateType::Color);
        kGray3                           = DTThemeData(QColor(242, 242, 242, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray0                           = DTThemeData(QColor(255, 255, 255, 1 * 255)   , DTThemeData::DTDateType::Color);
        kLineDisable                     = DTThemeData(QColor(0, 0, 0, 0.05 * 255)      , DTThemeData::DTDateType::Color);
        kGrayAlpha2                      = DTThemeData(QColor(0, 0, 0, 0.08 * 255)      , DTThemeData::DTDateType::Color);
        kGrayAlpha0                      = DTThemeData(QColor(0, 0, 0, 0 * 255)         , DTThemeData::DTDateType::Color);
        kBrand2                          = DTThemeData(QColor(114, 46, 209, 1 * 255)    , DTThemeData::DTDateType::Color);
        kLineNormal                      = DTThemeData(QColor(0, 0, 0, 0.1 * 255)       , DTThemeData::DTDateType::Color);
        kGrayAlpha5                      = DTThemeData(QColor(0, 0, 0, 0.18 * 255)      , DTThemeData::DTDateType::Color);
        kGrayAlpha12                     = DTThemeData(QColor(0, 0, 0, 0.03 * 255)      , DTThemeData::DTDateType::Color);
        kErrorClick                      = DTThemeData(QColor());
        kLineWindowActive                = DTThemeData(QColor(0, 0, 0, 0.1 * 255)       , DTThemeData::DTDateType::Color);
        kComponentAlphaClick             = DTThemeData(QColor(0, 0, 0, 0.2 * 255)       , DTThemeData::DTDateType::Color);
        kBrand3                          = DTThemeData(QColor(235, 48, 150, 1 * 255)    , DTThemeData::DTDateType::Color);
        kSuccessNormal                   = DTThemeData(QColor(82, 196, 41, 1 * 255)     , DTThemeData::DTDateType::Color);
        kGrayAlpha1                      = DTThemeData(QColor(0, 0, 0, 0.05 * 255)      , DTThemeData::DTDateType::Color);
        kFontWhiteSecondaryDisable       = DTThemeData(QColor(255, 255, 255, 0.3 * 255) , DTThemeData::DTDateType::Color);
        kGray4                           = DTThemeData(QColor(238, 238, 238, 1 * 255)   , DTThemeData::DTDateType::Color);
        kContainClick                    = DTThemeData(QColor(238, 238, 238, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray8                           = DTThemeData(QColor(210, 210, 210, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGrayAlpha4                      = DTThemeData(QColor(0, 0, 0, 0.15 * 255)      , DTThemeData::DTDateType::Color);
        kModalmask                       = DTThemeData(QColor(0, 0, 0, 0.2 * 255)       , DTThemeData::DTDateType::Color);
        kLineBrandNormal                 = DTThemeData(QColor(55, 144, 250, 0 * 255)    , DTThemeData::DTDateType::Color);
        kBrand5                          = DTThemeData(QColor(0, 129, 31, 1 * 255)      , DTThemeData::DTDateType::Color);
        kGray10                          = DTThemeData(QColor(185, 185, 185, 1 * 255)   , DTThemeData::DTDateType::Color);
        kWarningNormal                   = DTThemeData(QColor(246, 140, 39, 1 * 255)    , DTThemeData::DTDateType::Color);
        kGray9                           = DTThemeData(QColor(200, 200, 200, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray1                           = DTThemeData(QColor(250, 250, 250, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray14                          = DTThemeData(QColor(102, 102, 102, 1 * 255)   , DTThemeData::DTDateType::Color);
        kLineComponentClick              = DTThemeData(QColor(0, 0, 0, 0 * 255)         , DTThemeData::DTDateType::Color);
        kWhite                           = DTThemeData(QColor(255, 255, 255, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray7                           = DTThemeData(QColor(220, 220, 220, 1 * 255)   , DTThemeData::DTDateType::Color);
        kBrandFocus                      = DTThemeData(QColor());
        kGrayAlpha9                      = DTThemeData(QColor(0, 0, 0, 0.3 * 255)       , DTThemeData::DTDateType::Color);
        kGrayAlpha10                     = DTThemeData(QColor(0, 0, 0, 0.35 * 255)      , DTThemeData::DTDateType::Color);
        kGrayAlpha6                      = DTThemeData(QColor(0, 0, 0, 0.2 * 255)       , DTThemeData::DTDateType::Color);
        kContainHover                    = DTThemeData(QColor(242, 242, 242, 1 * 255)   , DTThemeData::DTDateType::Color);
        kLineComponentDisable            = DTThemeData(QColor(0, 0, 0, 0 * 255)         , DTThemeData::DTDateType::Color);
        kFontWhite                       = DTThemeData(QColor(255, 255, 255, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGrayAlpha11                     = DTThemeData(QColor(0, 0, 0, 0.55 * 255)      , DTThemeData::DTDateType::Color);
        kGray6                           = DTThemeData(QColor(230, 230, 230, 1 * 255)   , DTThemeData::DTDateType::Color);
        kFontWhiteDisable                = DTThemeData(QColor(255, 255, 255, 0.35 * 255), DTThemeData::DTDateType::Color);
        kComponentDisable                = DTThemeData(QColor(238, 238, 238, 1 * 255)   , DTThemeData::DTDateType::Color);
        kBlack                           = DTThemeData(QColor(0, 0, 0, 1 * 255)         , DTThemeData::DTDateType::Color);
        kComponentAlphaDisable           = DTThemeData(QColor(0, 0, 0, 0.08 * 255)      , DTThemeData::DTDateType::Color);
        kFontSecondaryDisable            = DTThemeData(QColor(185, 185, 185, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGrayAlpha8                      = DTThemeData(QColor(0, 0, 0, 0.28 * 255)      , DTThemeData::DTDateType::Color);
        kFontPrimaryDisable              = DTThemeData(QColor(175, 175, 175, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray5                           = DTThemeData(QColor(235, 235, 235, 1 * 255)   , DTThemeData::DTDateType::Color);
        kFontSecondary                   = DTThemeData(QColor(115, 115, 115, 1 * 255)   , DTThemeData::DTDateType::Color);
        kErrorNormal                     = DTThemeData(QColor(243, 34, 45, 1 * 255)     , DTThemeData::DTDateType::Color);
        kComponentHover                  = DTThemeData(QColor(220, 220, 220, 1 * 255)   , DTThemeData::DTDateType::Color);
        kComponentAlphaHover             = DTThemeData(QColor(0, 0, 0, 0.15 * 255)      , DTThemeData::DTDateType::Color);
        kGrayAlpha3                      = DTThemeData(QColor(0, 0, 0, 0.1 * 255)       , DTThemeData::DTDateType::Color);
        kGray13                          = DTThemeData(QColor(115, 115, 115, 1 * 255)   , DTThemeData::DTDateType::Color);
        kGray11                          = DTThemeData(QColor(175, 175, 175, 1 * 255)   , DTThemeData::DTDateType::Color);
        kErrorHover                      = DTThemeData(QColor());
        kContainGeneralNormal            = DTThemeData(QColor(255, 255, 255, 1 * 255)   , DTThemeData::DTDateType::Color);
        kComponentAlphaNormal            = DTThemeData(QColor(0, 0, 0, 0.1 * 255)       , DTThemeData::DTDateType::Color);
        kGray15                          = DTThemeData(QColor(77, 77, 77, 1 * 255)      , DTThemeData::DTDateType::Color);
        kBrand7                          = DTThemeData(QColor(241, 133, 43, 1 * 255)    , DTThemeData::DTDateType::Color);
        kGrayAlpha7                      = DTThemeData(QColor(0, 0, 0, 0.25 * 255)      , DTThemeData::DTDateType::Color);
        kLineBrandHover                  = DTThemeData(QColor(55, 144, 250, 0 * 255)    , DTThemeData::DTDateType::Color);
        kGrayAlpha13                     = DTThemeData(QColor(0, 0, 0, 0.4 * 255)       , DTThemeData::DTDateType::Color);
        kLineComponentHover              = DTThemeData(QColor(0, 0, 0, 0 * 255)         , DTThemeData::DTDateType::Color);
        kBrand6                          = DTThemeData(QColor(223, 49, 55, 1 * 255)     , DTThemeData::DTDateType::Color);
        kGray17                          = DTThemeData(QColor(38, 38, 38, 1 * 255)      , DTThemeData::DTDateType::Color);
        kComponentClick                  = DTThemeData(QColor(185, 185, 185, 1 * 255)   , DTThemeData::DTDateType::Color);
        kBrandClick                      = DTThemeData(QColor());
        kFontStrong                      = DTThemeData(QColor(0, 0, 0, 1 * 255)         , DTThemeData::DTDateType::Color);
        kLineWindow                      = DTThemeData(QColor(0, 0, 0, 0.1 * 255)       , DTThemeData::DTDateType::Color);
        kGray2                           = DTThemeData(QColor(246, 246, 246, 1 * 255)   , DTThemeData::DTDateType::Color);
        kBrand4                          = DTThemeData(QColor(240, 188, 54, 1 * 255)    , DTThemeData::DTDateType::Color);
        kContainSecondaryNormal          = DTThemeData(QColor(246, 246, 246, 1 * 255)   , DTThemeData::DTDateType::Color);
        kBrandHover                      = DTThemeData(QColor());
        kFontPrimary                     = DTThemeData(QColor(38, 38, 38, 1 * 255)      , DTThemeData::DTDateType::Color);
        kBrandNormal                     = DTThemeData(QColor(55, 144, 250, 1 * 255)    , DTThemeData::DTDateType::Color);

        normalline      = DTThemeData(1,    DTThemeData::DTDateType::Integer);
        focusline       = DTThemeData(2,    DTThemeData::DTDateType::Integer);
        tokenGap4       = DTThemeData(4,    DTThemeData::DTDateType::Integer);
        tokenGap8       = DTThemeData(8,    DTThemeData::DTDateType::Integer);
        tokenGap16      = DTThemeData(16,   DTThemeData::DTDateType::Integer);
        tokenGap24      = DTThemeData(24,   DTThemeData::DTDateType::Integer);
        tokenGap40      = DTThemeData(40,   DTThemeData::DTDateType::Integer);

        kradiusMin      = DTThemeData(4,    DTThemeData::DTDateType::Integer);
        kradiusMenu     = DTThemeData(8,    DTThemeData::DTDateType::Integer);
        kradiusNormal   = DTThemeData(6,    DTThemeData::DTDateType::Integer);
        kradiusWindow   = DTThemeData(12,   DTThemeData::DTDateType::Integer);
    }

    initUKUIGlobalThemeParameters();
}

GlobalDTConfigPrivate::~GlobalDTConfigPrivate()
{

}


bool GlobalDTConfigPrivate::load()
{
    QString cfg;

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *settings = new QGSettings("org.ukui.style");

        QString styleName = settings->get("styleName").toString();
        if (styleName == "ukui-light" || styleName == "ukui-white") {
            m_lightMode = "light";
        } else if (styleName == "ukui-dark" || styleName == "ukui-black") {
            m_lightMode = "dark";
        }
        m_widgetThemeName = settings->get("widgetThemeName").toString();

        QString path = ThemeConfigPath;
        //dt config
        cfg = (QFile::exists(path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css")) ?
                    path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css" :
                    path + "k" + m_widgetThemeName + ".css";
    }

    qDebug() << "loadddd...." << cfg;
    if (cfg.isEmpty() || !QFile::exists(cfg)) {
        qWarning() << "TMPReadConfig load file not exists!" << cfg;
        return false;
    }

    QFile file(cfg);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "TMPReadConfig load file Error!" << cfg;
        return false;
    }

    QByteArray configData = file.readAll();
    file.close();
    m_cfgPath = cfg;
    m_dt_configData = configData;
    return true;
}

QColor GlobalDTConfigPrivate::stringToColor(QString s) const
{
    QColor c = Qt::NoBrush;

    QStringList list = s.split(",");
    if (list.length() >= 3) {
        c.setRed(list[0].toInt());
        c.setGreen(list[1].toInt());
        c.setBlue(list[2].toInt());
        if (list.length() == 4) {
            QStringList alphaList = QString(list[3]).split("*");
            if (alphaList.length() == 2) {
                if (alphaList[0].toFloat() == 255.0)
                    c.setAlphaF(alphaList[1].toFloat());
                else if (alphaList[1].toFloat() == 255.0)
                    c.setAlphaF(alphaList[0].toFloat());
                else
                    c.setAlphaF(alphaList[0].toFloat() * alphaList[1].toFloat() / 255.0);

            } else if (alphaList.length() == 1) {
                if(alphaList[0].toFloat() <= 1.0)
                    c.setAlphaF(alphaList[0].toFloat());
                else
                    c.setAlphaF(alphaList[0].toFloat()/255.0);
            } else
                c.setAlphaF(1.0);
        }
    } else if (list.length() == 1 && list[0].startsWith("#")) {
        c = list[0];
    }

    return c;
}

inline bool GlobalDTConfigPrivate::getColorValue(DTThemeData &dtColor, const QString key)
{
    if (m_dt_configData.isEmpty()) {
        qCDebug(DTReadConfig) << "get fills array isempty!";
        return false;
    }

    if (!m_dt_configData.contains("--" + key)) {
        qCDebug(DTReadConfig) << "Unvalid key!" << key;
        return false;
    }

    int start_index, end_index;
    QString colorStr;

    start_index = m_dt_configData.indexOf("--" + key);
    if (start_index == -1) {
        qCDebug(DTReadConfig) <<"Config file format error! Has no key" << key;
        return false;
    }

    start_index += QString("--" + key).length();
    end_index = m_dt_configData.indexOf(";", start_index);
    colorStr = m_dt_configData.mid(start_index, end_index - start_index + 1);

    if (colorStr.contains(": var(")) {
        //quote key
        start_index = colorStr.indexOf(": var(--");
        if (start_index == -1) {
            qWarning() << "Config file format error! type var" << key;
            return false;
        }
        start_index += QString(": var(--").length();
        end_index = colorStr.indexOf(")", start_index);
        getColorValue(dtColor, colorStr.mid(start_index, end_index - start_index).toLower());

    } else if (colorStr.contains(": rgba(")) {
        //solidcolor
        start_index = colorStr.indexOf(": rgba(");
        if (start_index == -1) {
            qWarning() << "Config file format error! type rgba" << key;
            return false;
        }
        start_index += QString(": rgba(").length();
        end_index = colorStr.indexOf(")", start_index);

        dtColor.setColor(stringToColor(colorStr.mid(start_index, end_index - start_index)));
        dtColor.setType(DTThemeData::DTDateType::Color);
    } else if (colorStr.contains(": linear-gradient(0deg, ")) {
        //mix solidcolor
        start_index = colorStr.indexOf(": linear-gradient(0deg, ");
        if (start_index == -1) {
            qWarning() << "Config file format error! type linear-gradient 0deg" << key;
            return false;
        }

        start_index += QString(": linear-gradient(0deg, ").length();
        end_index = colorStr.indexOf(";", start_index);
        colorStr = colorStr.mid(start_index, end_index - start_index);

        QStringList sList = colorStr.split("rgba");
        QList<QColor> listColor;
        QColor solidColor;
        QStringList colorList;
        for(int i = 0; i < sList.length(); i++) {
            QString s = sList[i];
            if(!s.isEmpty()){
                int sIndex = s.indexOf("(");
                int eIndex = s.indexOf(")");
                s = s.mid(sIndex + 1, eIndex - sIndex - 1);
                if(!colorList.contains(s))
                    colorList.append(s);
            }
        }

        for (int i = 0; i < colorList.length(); i++) {
            QString s = colorList[i];

            QColor color = stringToColor(s);
            if(!listColor.contains(color))
                listColor.append(color);
        }

        dtColor.setType(DTThemeData::DTDateType::Color);
        solidColor = dtColor.mixColor(listColor);
        dtColor.setColor(solidColor);
    } else if (colorStr.contains(": linear-gradient(180deg, ")) {
        //gradient
        start_index = colorStr.indexOf(": linear-gradient(180deg, ");
        if (start_index == -1) {
            qWarning() << "Config file format error! type linear-gradient 180deg" << key;
            return false;
        }

        start_index += QString(": linear-gradient(180deg, ").length();
        end_index = colorStr.indexOf(";", start_index);
        colorStr = colorStr.mid(start_index, end_index - start_index);

        QStringList sList = colorStr.split("rgba");
        QList<QColor> startListColor;
        QList<QColor> endListColor;

        QColor backColor;
        QColor startColor;
        QColor endColor;

        QString sbackColor = sList.takeLast();
        sbackColor = sbackColor.mid(sbackColor.indexOf("(") + 1, sbackColor.indexOf(")") - sbackColor.indexOf("(") - 1);
        backColor = stringToColor(sbackColor);
        qDebug()<<"backColor"<<backColor.red()<<backColor.green()<<backColor.blue()<<backColor.alpha();
        for(int i = 0; i < sList.length(); i++) {
            QString s = sList[i];
            if (s.isEmpty())
                continue;

            int sIndex = s.indexOf("(");
            int eIndex = s.indexOf(")");
            s = s.mid(sIndex + 1, eIndex - sIndex - 1);
            if (i <= sList.length() / 2)
                startListColor.append(stringToColor(s));
            else
                endListColor.append(stringToColor(s));
        }

        startListColor.append(backColor);
        endListColor.append(backColor);

        startColor = dtColor.mixColor(startListColor);
        endColor   = dtColor.mixColor(endListColor);


        dtColor.setType(DTThemeData::DTDateType::Gradient);
        dtColor.setStartColor(startColor);
        dtColor.setEndColor(endColor);
    } else {
        qCDebug(DTReadConfig) << "Config file format error! " << key << "colorStr" << colorStr;
        return false;
    }

    return true;
}


inline bool GlobalDTConfigPrivate::getValue(DTThemeData &value, const QString key)
{
    if (m_dt_configData.isEmpty()) {
        qCDebug(DTReadConfig) << "get fills array isempty!";
        return false;
    }

    if (!m_dt_configData.contains(key)) {
        qCDebug(DTReadConfig) << "Unvalid key!" << key;
        return false;
    }

    int start_index = m_dt_configData.indexOf("--" + key + ": ");
    if (start_index == -1) {
        qCDebug(DTReadConfig) << "Config file format error!" << key;
        return false;
    }
    start_index += QString("--" + key + ": ").length();
    int end_index = m_dt_configData.indexOf("px", start_index);

    value.setIntValue(m_dt_configData.mid(start_index, end_index - start_index).toInt());

    return true;
}

void GlobalDTConfigPrivate::initUKUIGlobalThemeParameters()
{
    m_ukui_dt_loadConfig = this->load();
    qDebug() << "initUKUIGlobalThemeParameters......" << m_ukui_dt_loadConfig;
    if (m_ukui_dt_loadConfig) {

        //default palette
        getColorValue(windowTextActive          , "windowtext-active");
        getColorValue(windowTextInactive        , "windowtext-inactive");
        getColorValue(windowTextDisable         , "windowtext-disable");
        getColorValue(buttonActive              , "button-active");
        getColorValue(buttonInactive            , "button-inactive");
        getColorValue(buttonDisable             , "button-disable");
        getColorValue(lightActive               , "light-active");
        getColorValue(lightInactive             , "light-inactive");
        getColorValue(lightDisable              , "light-disable");
        getColorValue(midlightActive            , "midlight-active");
        getColorValue(midlightInactive          , "midlight-inactive");
        getColorValue(midlightDisable           , "midlight-disable");
        getColorValue(darkActive                , "dark-active");
        getColorValue(darkInactive              , "dark-inactive");
        getColorValue(darkDisable               , "dark-disable");
        getColorValue(midActive                 , "mid-active");
        getColorValue(midInactive               , "mid-inactive");
        getColorValue(midDisable                , "mid-disable");
        getColorValue(textActive                , "text-active");
        getColorValue(textInactive              , "text-inactive");
        getColorValue(textDisable               , "text-disable");
        getColorValue(brightTextActive          , "brighttext-active");
        getColorValue(brightTextInactive        , "brighttext-inactive");
        getColorValue(brightTextDisable         , "brighttext-disable");
        getColorValue(buttonTextActive          , "buttontext-active");
        getColorValue(buttonTextInactive        , "buttontext-inactive");
        getColorValue(buttonTextDisable         , "buttontext-disable");
        getColorValue(baseActive                , "base-active");
        getColorValue(baseInactive              , "base-inactive");
        getColorValue(baseDisable               , "base-disable");
        getColorValue(windowActive              , "window-active");
        getColorValue(windowInactive            , "window-inactive");
        getColorValue(windowDisable             , "window-disable");
        getColorValue(shadowActive              , "shadow-active");
        getColorValue(shadowInactive            , "shadow-inactive");
        getColorValue(shadowDisable             , "shadow-disable");
        getColorValue(highLightActive           , "highlight-active");
        getColorValue(highLightInactive         , "highlight-inactive");
        getColorValue(highLightDisable          , "highlight-disable");
        getColorValue(highLightedTextActive     , "highlightedtext-active");
        getColorValue(highLightedTextInactive   , "highlightedtext-inactive");
        getColorValue(highLightedTextDisable    , "highlightedtext-disable");
        getColorValue(linkActive                , "link-active");
        getColorValue(linkInactive              , "link-inactive");
        getColorValue(linkDisable               , "link-disable");
        getColorValue(linkVisitedActive         , "linkvisited-active");
        getColorValue(linkVisitedInactive       , "linkvisited-inactive");
        getColorValue(linkVisitedDisable        , "linkvisited-disable");
        getColorValue(alternateBaseActive       , "alternatebase-active");
        getColorValue(alternateBaseInactive     , "alternatebase-inactive");
        getColorValue(alternateBaseDisable      , "alternatebase-disable");
        getColorValue(noRoleActive              , "norole-active");
        getColorValue(noRoleInactive            , "norole-inactive");
        getColorValue(noRoleDisable             , "norole-disable");
        getColorValue(toolTipBaseActive         , "tooltipbase-active");
        getColorValue(toolTipBaseInactive       , "tooltipbase-inactive");
        getColorValue(toolTipBaseDisable        , "tooltipbase-disable");
        getColorValue(toolTipTextActive         , "tooltiptext-active");
        getColorValue(toolTipTextInactive       , "tooltiptext-inactive");
        getColorValue(toolTipTextDisable        , "tooltiptext-disable");
        getColorValue(placeholderTextActive     , "placeholdertext-active");
        getColorValue(placeholderTextInactive   , "placeholdertext-inactive");
        getColorValue(placeholderTextDisable    , "placeholdertext-disable");

        //custom palette
        getColorValue(kBlack                        , "kblack");
        getColorValue(kBrand1                       , "kbrand1");
        getColorValue(kBrand2                       , "kbrand2");
        getColorValue(kBrand3                       , "kbrand3");
        getColorValue(kBrand4                       , "kbrand4");
        getColorValue(kBrand5                       , "kbrand5");
        getColorValue(kBrand6                       , "kbrand6");
        getColorValue(kBrand7                       , "kbrand7");
        getColorValue(kBrandClick                   , "kbrand-click");
        getColorValue(kBrandFocus                   , "kbrand-focus");
        getColorValue(kBrandHover                   , "kbrand-hover");
        getColorValue(kBrandNormal                  , "kbrand-normal");
        getColorValue(kComponentAlphaClick          , "kcomponent-alpha-click");
        getColorValue(kComponentAlphaDisable        , "kcomponent-alpha-disable");
        getColorValue(kComponentAlphaHover          , "kcomponent-alpha-hover");
        getColorValue(kComponentAlphaNormal         , "kcomponent-alpha-normal");
        getColorValue(kComponentClick               , "kcomponent-click");
        getColorValue(kComponentDisable             , "kcomponent-disable");
        getColorValue(kComponentHover               , "kcomponent-hover");
        getColorValue(kComponentNormal              , "kcomponent-normal");
        getColorValue(kContainClick                 , "kcontain-click");
        getColorValue(kContainHover                 , "kcontain-hover");
        getColorValue(kContainGeneralNormal         , "kcontain-general-normal");
        getColorValue(kContainSecondaryAlpphaNormal , "kcontain-secondary-alpha-normal");
        getColorValue(kContainSecondaryNormal       , "kcontain-secondary-normal");
        getColorValue(kErrorClick                   , "kerror-click");
        getColorValue(kErrorHover                   , "kerror-hover");
        getColorValue(kErrorNormal                  , "kerror-normal");
        getColorValue(kFontPrimary                  , "kfont-primary");
        getColorValue(kFontPrimaryDisable           , "kfont-primary-disable");
        getColorValue(kFontSecondary                , "kfont-secondary");
        getColorValue(kFontSecondaryDisable         , "kfont-secondary-disable");
        getColorValue(kFontStrong                   , "kfont-strong");
        getColorValue(kFontWhite                    , "kfont-white");
        getColorValue(kFontWhiteDisable             , "kfont-white-disable");
        getColorValue(kFontWhiteSecondary           , "kfont-white-secondary");
        getColorValue(kFontWhiteSecondaryDisable    , "kfont-white-secondary-disable");
        getColorValue(kGray0                        , "kgray-0");
        getColorValue(kGray1                        , "kgray-1");
        getColorValue(kGray2                        , "kgray-2");
        getColorValue(kGray3                        , "kgray-3");
        getColorValue(kGray4                        , "kgray-4");
        getColorValue(kGray5                        , "kgray-5");
        getColorValue(kGray6                        , "kgray-6");
        getColorValue(kGray7                        , "kgray-7");
        getColorValue(kGray8                        , "kgray-8");
        getColorValue(kGray9                        , "kgray-9");
        getColorValue(kGray10                       , "kgray-10");
        getColorValue(kGray11                       , "kgray-11");
        getColorValue(kGray12                       , "kgray-12");
        getColorValue(kGray13                       , "kgray-13");
        getColorValue(kGray14                       , "kgray-14");
        getColorValue(kGray15                       , "kgray-15");
        getColorValue(kGray16                       , "kgray-16");
        getColorValue(kGray17                       , "kgray-17");
        getColorValue(kGrayAlpha0                   , "kgray-alpha0");
        getColorValue(kGrayAlpha1                   , "kgray-alpha1");
        getColorValue(kGrayAlpha2                   , "kgray-alpha2");
        getColorValue(kGrayAlpha3                   , "kgray-alpha3");
        getColorValue(kGrayAlpha4                   , "kgray-alpha4");
        getColorValue(kGrayAlpha5                   , "kgray-alpha5");
        getColorValue(kGrayAlpha6                   , "kgray-alpha6");
        getColorValue(kGrayAlpha7                   , "kgray-alpha7");
        getColorValue(kGrayAlpha8                   , "kgray-alpha8");
        getColorValue(kGrayAlpha9                   , "kgray-alpha9");
        getColorValue(kGrayAlpha10                  , "kgray-alpha10");
        getColorValue(kGrayAlpha11                  , "kgray-alpha11");
        getColorValue(kGrayAlpha12                  , "kgray-alpha12");
        getColorValue(kGrayAlpha13                  , "kgray-alpha13");
        getColorValue(kLineBrandClick               , "kline-brand-click");
        getColorValue(kLineBrandDisable             , "kline-brand-disable");
        getColorValue(kLineBrandHover               , "kline-brand-hover");
        getColorValue(kLineBrandNormal              , "kline-brand-normal");
        getColorValue(kLineComponentClick           , "kline-component-click");
        getColorValue(kLineComponentDisable         , "kline-component-disable");
        getColorValue(kLineComponentHover           , "kline-component-hover");
        getColorValue(kLineComponentNormal          , "kline-component-normal");
        getColorValue(kLineDisable                  , "kline-disable");
        getColorValue(kLineNormal                   , "kline-normal");
        getColorValue(kLineWindow                   , "kline-window");
        getColorValue(kLineWindowActive             , "kline-window-active");
        getColorValue(kModalmask                    , "kmodalmask");
        getColorValue(kSuccessNormal                , "ksuccess-normal");
        getColorValue(kWarningNormal                , "kwarning-normal");
        getColorValue(kWhite                        , "kwhite");

        //line and margin
        getValue(normalline     , "normalline");
        getValue(focusline      , "focusline");
        getValue(tokenGap4      , "token-gap-4");
        getValue(tokenGap8      , "token-gap-8");
        getValue(tokenGap16     , "token-gap-16");
        getValue(tokenGap24     , "token-gap-24");
        getValue(tokenGap40     , "token-gap-40");

        //radius
        getValue(kradiusMin     , "kradius-min");
        getValue(kradiusMenu    , "kradius-menu");
        getValue(kradiusNormal  , "kradius-normal");
        getValue(kradiusWindow  , "kradius-window");
    }

    dtDataMap.clear();
    //default palette
    dtDataMap.insert("windowtext-active"        , windowTextActive);
    dtDataMap.insert("windowtext-inactive"      , windowTextInactive);
    dtDataMap.insert("windowtext-disable"       , windowTextDisable);
    dtDataMap.insert("button-active"            , buttonActive);
    dtDataMap.insert("button-inactive"          , buttonInactive);
    dtDataMap.insert("button-disable"           , buttonDisable);
    dtDataMap.insert("light-active"             , lightActive);
    dtDataMap.insert("light-inactive"           , lightInactive);
    dtDataMap.insert("light-disable"            , lightDisable);
    dtDataMap.insert("midlight-active"          , midlightActive);
    dtDataMap.insert("midlight-inactive"        , midlightInactive);
    dtDataMap.insert("midlight-disable"         , midlightDisable);
    dtDataMap.insert("dark-active"              , darkActive);
    dtDataMap.insert("dark-inactive"            , darkInactive);
    dtDataMap.insert("dark-disable"             , darkDisable);
    dtDataMap.insert("mid-active"               , midActive);
    dtDataMap.insert("mid-inactive"             , midInactive);
    dtDataMap.insert("mid-disable"              , midDisable);
    dtDataMap.insert("text-active"              , textActive);
    dtDataMap.insert("text-inactive"            , textInactive);
    dtDataMap.insert("text-disable"             , textDisable);
    dtDataMap.insert("brighttext-active"        , brightTextActive);
    dtDataMap.insert("brighttext-inactive"      , brightTextInactive);
    dtDataMap.insert("brighttext-disable"       , brightTextDisable);
    dtDataMap.insert("buttontext-active"        , buttonTextActive);
    dtDataMap.insert("buttontext-inactive"      , buttonTextInactive);
    dtDataMap.insert("buttontext-disable"       , buttonTextDisable);
    dtDataMap.insert("base-active"              , baseActive);
    dtDataMap.insert("base-inactive"            , baseInactive);
    dtDataMap.insert("base-disable"             , baseDisable);
    dtDataMap.insert("window-active"            , windowActive);
    dtDataMap.insert("window-inactive"          , windowInactive);
    dtDataMap.insert("window-disable"           , windowDisable);
    dtDataMap.insert("shadow-active"            , shadowActive);
    dtDataMap.insert("shadow-inactive"          , shadowInactive);
    dtDataMap.insert("shadow-disable"           , shadowDisable);
    dtDataMap.insert("highlight-active"         , highLightActive);
    dtDataMap.insert("highlight-inactive"       , highLightInactive);
    dtDataMap.insert("highlight-disable"        , highLightDisable);
    dtDataMap.insert("highlightedtext-active"   , highLightedTextActive);
    dtDataMap.insert("highlightedtext-inactive" , highLightedTextInactive);
    dtDataMap.insert("highlightedtext-disable"  , highLightedTextDisable);
    dtDataMap.insert("link-active"              , linkActive);
    dtDataMap.insert("link-inactive"            , linkInactive);
    dtDataMap.insert("link-disable"             , linkDisable);
    dtDataMap.insert("linkvisited-active"       , linkVisitedActive);
    dtDataMap.insert("linkvisited-inactive"     , linkVisitedInactive);
    dtDataMap.insert("linkvisited-disable"      , linkVisitedDisable);
    dtDataMap.insert("alternatebase-active"     , alternateBaseActive);
    dtDataMap.insert("alternatebase-inactive"   , alternateBaseInactive);
    dtDataMap.insert("alternatebase-disable"    , alternateBaseDisable);
    dtDataMap.insert("norole-active"            , noRoleActive);
    dtDataMap.insert("norole-inactive"          , noRoleInactive);
    dtDataMap.insert("norole-disable"           , noRoleDisable);
    dtDataMap.insert("tooltipbase-active"       , toolTipBaseActive);
    dtDataMap.insert("tooltipbase-inactive"     , toolTipBaseInactive);
    dtDataMap.insert("tooltipbase-disable"      , toolTipBaseDisable);
    dtDataMap.insert("tooltiptext-active"       , toolTipTextActive);
    dtDataMap.insert("tooltiptext-inactive"     , toolTipTextInactive);
    dtDataMap.insert("tooltiptext-disable"      , toolTipTextDisable);
    dtDataMap.insert("placeholdertext-active"   , placeholderTextActive);
    dtDataMap.insert("placeholdertext-inactive" , placeholderTextInactive);
    dtDataMap.insert("placeholdertext-disable"  , placeholderTextDisable);

    //custom palette
    dtDataMap.insert("kblack", kBlack);
    dtDataMap.insert("kbrand1", kBrand1);
    dtDataMap.insert("kbrand2", kBrand2);
    dtDataMap.insert("kbrand3", kBrand3);
    dtDataMap.insert("kbrand4", kBrand4);
    dtDataMap.insert("kbrand5", kBrand5);
    dtDataMap.insert("kbrand6", kBrand6);
    dtDataMap.insert("kbrand7", kBrand7);
    dtDataMap.insert("kbrand-click", kBrandClick);
    dtDataMap.insert("kbrand-focus", kBrandFocus);
    dtDataMap.insert("kbrand-hover", kBrandHover);
    dtDataMap.insert("kbrand-normal", kBrandNormal);
    dtDataMap.insert("kcomponent-alpha-click", kComponentAlphaClick);
    dtDataMap.insert("kcomponent-alpha-disable", kComponentAlphaDisable);
    dtDataMap.insert("kcomponent-alpha-hover", kComponentAlphaHover);
    dtDataMap.insert("kcomponent-alpha-normal", kComponentAlphaNormal);
    dtDataMap.insert("kcomponent-click", kComponentClick);
    dtDataMap.insert("kcomponent-disable", kComponentDisable);
    dtDataMap.insert("kcomponent-hover", kComponentHover);
    dtDataMap.insert("kcomponent-normal", kComponentNormal);
    dtDataMap.insert("kcontain-click", kContainClick);
    dtDataMap.insert("kcontain-hover", kContainHover);
    dtDataMap.insert("kcontain-general-normal", kContainGeneralNormal);
    dtDataMap.insert("kcontain-secondary-alpha-normal", kContainSecondaryAlpphaNormal);
    dtDataMap.insert("kcontain-secondary-normal", kContainSecondaryNormal);
    dtDataMap.insert("kerror-click", kErrorClick);
    dtDataMap.insert("kerror-hover", kErrorHover);
    dtDataMap.insert("kerror-normal", kErrorNormal);
    dtDataMap.insert("kfont-primary", kFontPrimary);
    dtDataMap.insert("kfont-primary-disable", kFontPrimaryDisable);
    dtDataMap.insert("kfont-secondary", kFontSecondary);
    dtDataMap.insert("kfont-secondary-disable", kFontSecondaryDisable);
    dtDataMap.insert("kfont-strong", kFontStrong);
    dtDataMap.insert("kfont-white", kFontWhite);
    dtDataMap.insert("kfont-white-disable", kFontWhiteDisable);
    dtDataMap.insert("kfont-white-secondary", kFontWhiteSecondary);
    dtDataMap.insert("kfont-white-secondary-disable", kFontWhiteSecondaryDisable);
    dtDataMap.insert("kgray-0", kGray0);
    dtDataMap.insert("kgray-1", kGray1);
    dtDataMap.insert("kgray-2", kGray2);
    dtDataMap.insert("kgray-3", kGray3);
    dtDataMap.insert("kgray-4", kGray4);
    dtDataMap.insert("kgray-5", kGray5);
    dtDataMap.insert("kgray-6", kGray6);
    dtDataMap.insert("kgray-7", kGray7);
    dtDataMap.insert("kgray-8", kGray8);
    dtDataMap.insert("kgray-9", kGray9);
    dtDataMap.insert("kgray-10", kGray10);
    dtDataMap.insert("kgray-11", kGray11);
    dtDataMap.insert("kgray-12", kGray12);
    dtDataMap.insert("kgray-13", kGray13);
    dtDataMap.insert("kgray-14", kGray14);
    dtDataMap.insert("kgray-15", kGray15);
    dtDataMap.insert("kgray-16", kGray16);
    dtDataMap.insert("kgray-17", kGray17);
    dtDataMap.insert("kgray-alpha0", kGrayAlpha0);
    dtDataMap.insert("kgray-alpha1", kGrayAlpha1);
    dtDataMap.insert("kgray-alpha2", kGrayAlpha2);
    dtDataMap.insert("kgray-alpha3", kGrayAlpha3);
    dtDataMap.insert("kgray-alpha4", kGrayAlpha4);
    dtDataMap.insert("kgray-alpha5", kGrayAlpha5);
    dtDataMap.insert("kgray-alpha6", kGrayAlpha6);
    dtDataMap.insert("kgray-alpha7", kGrayAlpha7);
    dtDataMap.insert("kgray-alpha8", kGrayAlpha8);
    dtDataMap.insert("kgray-alpha9", kGrayAlpha9);
    dtDataMap.insert("kgray-alpha10", kGrayAlpha10);
    dtDataMap.insert("kgray-alpha11", kGrayAlpha11);
    dtDataMap.insert("kgray-alpha12", kGrayAlpha12);
    dtDataMap.insert("kgray-alpha13", kGrayAlpha13);
    dtDataMap.insert("kline-brand-click", kLineBrandClick);
    dtDataMap.insert("kline-brand-disable", kLineBrandDisable);
    dtDataMap.insert("kline-brand-hover", kLineBrandHover);
    dtDataMap.insert("kline-brand-normal", kLineBrandNormal);
    dtDataMap.insert("kline-component-click", kLineComponentClick);
    dtDataMap.insert("kline-component-disable", kLineComponentDisable);
    dtDataMap.insert("kline-component-hover", kLineComponentHover);
    dtDataMap.insert("kline-component-normal", kLineComponentNormal);
    dtDataMap.insert("kline-disable", kLineDisable);
    dtDataMap.insert("kline-normal", kLineNormal);
    dtDataMap.insert("kline-window", kLineWindow);
    dtDataMap.insert("kline-window-active", kLineWindowActive);
    dtDataMap.insert("kmodalmask", kModalmask);
    dtDataMap.insert("ksuccess-normal", kSuccessNormal);
    dtDataMap.insert("kwarning-normal", kWarningNormal);
    dtDataMap.insert("kwhite", kWhite);

    //line and margin
    dtDataMap.insert("normalline", normalline);
    dtDataMap.insert("focusline", focusline);
    dtDataMap.insert("token-gap-4", tokenGap4);
    dtDataMap.insert("token-gap-8", tokenGap8);
    dtDataMap.insert("token-gap-16", tokenGap16);
    dtDataMap.insert("token-gap-24", tokenGap24);
    dtDataMap.insert("token-gap-40", tokenGap40);

    //radius
    dtDataMap.insert("kradius-min", kradiusMin);
    dtDataMap.insert("kradius-menu", kradiusMenu);
    dtDataMap.insert("kradius-normal", kradiusNormal);
    dtDataMap.insert("kradius-window", kradiusWindow);
}

GlobalDTConfig::GlobalDTConfig() : d(new GlobalDTConfigPrivate())
{
    //theme switch
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *settings = new QGSettings("org.ukui.style");
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor" ||
                    key == "style-name" || key == "widget-theme-name" || key == "theme-color") {

                d->initUKUIGlobalThemeParameters();
                if (d->m_ukui_dt_loadConfig)
                    emit tokenChanged();
            }
        });
    }
}

GlobalDTConfig::~GlobalDTConfig()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

DTThemeData GlobalDTConfig::windowTextActive() const
{
    return d->windowTextActive;
}

DTThemeData GlobalDTConfig::windowTextInactive() const
{
    return d->windowTextInactive;
}

DTThemeData GlobalDTConfig::windowTextDisable() const
{
    return d->windowTextDisable;
}

DTThemeData GlobalDTConfig::buttonActive() const
{
    return d->buttonActive;
}

DTThemeData GlobalDTConfig::buttonInactive() const
{
    return d->buttonInactive;
}

DTThemeData GlobalDTConfig::buttonDisable() const
{
    return d->buttonDisable;
}

DTThemeData GlobalDTConfig::lightActive() const
{
    return d->lightActive;
}

DTThemeData GlobalDTConfig::lightInactive() const
{
    return d->lightInactive;
}

DTThemeData GlobalDTConfig::lightDisable() const
{
    return d->lightDisable;
}

DTThemeData GlobalDTConfig::midlightActive() const
{
    return d->midlightActive;
}

DTThemeData GlobalDTConfig::midlightInactive() const
{
    return d->midlightInactive;
}

DTThemeData GlobalDTConfig::midlightDisable() const
{
    return d->midlightDisable;
}

DTThemeData GlobalDTConfig::darkActive() const
{
    return d->darkActive;
}

DTThemeData GlobalDTConfig::darkInactive() const
{
    return d->darkInactive;
}

DTThemeData GlobalDTConfig::darkDisable() const
{
    return d->darkDisable;
}

DTThemeData GlobalDTConfig::midActive() const
{
    return d->midActive;
}

DTThemeData GlobalDTConfig::midInactive() const
{
    return d->midInactive;
}

DTThemeData GlobalDTConfig::midDisable() const
{
    return d->midDisable;
}

DTThemeData GlobalDTConfig::textActive() const
{
    return d->textActive;
}

DTThemeData GlobalDTConfig::textInactive() const
{
    return d->textInactive;
}

DTThemeData GlobalDTConfig::textDisable() const
{
    return d->textDisable;
}

DTThemeData GlobalDTConfig::brightTextActive() const
{
    return d->brightTextActive;
}

DTThemeData GlobalDTConfig::brightTextInactive() const
{
    return d->brightTextInactive;
}

DTThemeData GlobalDTConfig::brightTextDisable() const
{
    return d->brightTextDisable;
}

DTThemeData GlobalDTConfig::buttonTextActive() const
{
    return d->buttonTextActive;
}

DTThemeData GlobalDTConfig::buttonTextInactive() const
{
    return d->buttonTextInactive;
}

DTThemeData GlobalDTConfig::buttonTextDisable() const
{
    return d->buttonTextDisable;
}

DTThemeData GlobalDTConfig::baseActive() const
{
    return d->baseActive;
}

DTThemeData GlobalDTConfig::baseInactive() const
{
    return d->baseInactive;
}

DTThemeData GlobalDTConfig::baseDisable() const
{
    return d->baseDisable;
}

DTThemeData GlobalDTConfig::windowActive() const
{
    return d->windowActive;
}

DTThemeData GlobalDTConfig::windowInactive() const
{
    return d->windowInactive;
}

DTThemeData GlobalDTConfig::windowDisable() const
{
    return d->windowDisable;
}

DTThemeData GlobalDTConfig::shadowActive() const
{
    return d->shadowActive;
}

DTThemeData GlobalDTConfig::shadowInactive() const
{
    return d->shadowInactive;
}

DTThemeData GlobalDTConfig::shadowDisable() const
{
    return d->shadowDisable;
}

DTThemeData GlobalDTConfig::highLightActive() const
{
    return d->highLightActive;
}

DTThemeData GlobalDTConfig::highLightInactive() const
{
    return d->highLightInactive;
}

DTThemeData GlobalDTConfig::highLightDisable() const
{
    return d->highLightDisable;
}

DTThemeData GlobalDTConfig::highLightedTextActive() const
{
    return d->highLightedTextActive;
}

DTThemeData GlobalDTConfig::highLightedTextInactive() const
{
    return d->highLightedTextInactive;
}

DTThemeData GlobalDTConfig::highLightedTextDisable() const
{
    return d->highLightedTextDisable;
}

DTThemeData GlobalDTConfig::linkActive() const
{
    return d->linkActive;
}

DTThemeData GlobalDTConfig::linkInactive() const
{
    return d->linkInactive;
}

DTThemeData GlobalDTConfig::linkDisable() const
{
    return d->linkDisable;
}

DTThemeData GlobalDTConfig::linkVisitedActive() const
{
    return d->linkVisitedActive;
}

DTThemeData GlobalDTConfig::linkVisitedInactive() const
{
    return d->linkVisitedInactive;
}

DTThemeData GlobalDTConfig::linkVisitedDisable() const
{
    return d->linkVisitedDisable;
}

DTThemeData GlobalDTConfig::alternateBaseActive() const
{
    return d->alternateBaseActive;
}

DTThemeData GlobalDTConfig::alternateBaseInactive() const
{
    return d->alternateBaseInactive;
}

DTThemeData GlobalDTConfig::alternateBaseDisable() const
{
    return d->alternateBaseDisable;
}

DTThemeData GlobalDTConfig::noRoleActive() const
{
    return d->noRoleActive;
}

DTThemeData GlobalDTConfig::noRoleInactive() const
{
    return d->noRoleInactive;
}

DTThemeData GlobalDTConfig::noRoleDisable() const
{
    return d->noRoleDisable;
}

DTThemeData GlobalDTConfig::toolTipBaseActive() const
{
    return d->toolTipBaseActive;
}

DTThemeData GlobalDTConfig::toolTipBaseInactive() const
{
    return d->toolTipBaseInactive;
}

DTThemeData GlobalDTConfig::toolTipBaseDisable() const
{
    return d->toolTipBaseDisable;
}

DTThemeData GlobalDTConfig::toolTipTextActive() const
{
    return d->toolTipTextActive;
}

DTThemeData GlobalDTConfig::toolTipTextInactive() const
{
    return d->toolTipTextInactive;
}

DTThemeData GlobalDTConfig::toolTipTextDisable() const
{
    return d->toolTipTextDisable;
}

DTThemeData GlobalDTConfig::placeholderTextActive() const
{
    return d->placeholderTextActive;
}

DTThemeData GlobalDTConfig::placeholderTextInactive() const
{
    return d->placeholderTextInactive;
}

DTThemeData GlobalDTConfig::placeholderTextDisable() const
{
    return d->placeholderTextDisable;
}

DTThemeData GlobalDTConfig::kComponentNormal() const
{
    return d->kComponentNormal;
}

DTThemeData GlobalDTConfig::kLineBrandDisable() const
{
    return d->kLineBrandDisable;
}

DTThemeData GlobalDTConfig::kFontWhiteSecondary() const
{
    return d->kFontWhiteSecondary;
}

DTThemeData GlobalDTConfig::kGray12() const
{
    return d->kGray12;
}

DTThemeData GlobalDTConfig::kContainSecondaryAlpphaNormal() const
{
    return d->kContainSecondaryAlpphaNormal;
}

DTThemeData GlobalDTConfig::kGray16() const
{
    return d->kGray16;
}

DTThemeData GlobalDTConfig::kBrand1() const
{
    return d->kBrand1;
}

DTThemeData GlobalDTConfig::kLineComponentNormal() const
{
    return d->kLineComponentNormal;
}

DTThemeData GlobalDTConfig::kLineBrandClick() const
{
    return d->kLineBrandClick;
}

DTThemeData GlobalDTConfig::kGray3() const
{
    return d->kGray3;
}

DTThemeData GlobalDTConfig::kGray0() const
{
    return d->kGray0;
}

DTThemeData GlobalDTConfig::kLineDisable() const
{
    return d->kLineDisable;
}

DTThemeData GlobalDTConfig::kGrayAlpha2() const
{
    return d->kGrayAlpha2;
}

DTThemeData GlobalDTConfig::kGrayAlpha0() const
{
    return d->kGrayAlpha0;
}

DTThemeData GlobalDTConfig::kBrand2() const
{
    return d->kBrand2;
}

DTThemeData GlobalDTConfig::kLineNormal() const
{
    return d->kLineNormal;
}

DTThemeData GlobalDTConfig::kGrayAlpha5() const
{
    return d->kGrayAlpha5;
}

DTThemeData GlobalDTConfig::kGrayAlpha12() const
{
    return d->kGrayAlpha12;
}

DTThemeData GlobalDTConfig::kErrorClick() const
{
    return d->kErrorClick;
}

DTThemeData GlobalDTConfig::kLineWindowActive() const
{
    return d->kLineWindowActive;
}

DTThemeData GlobalDTConfig::kComponentAlphaClick() const
{
    return d->kComponentAlphaClick;
}

DTThemeData GlobalDTConfig::kBrand3() const
{
    return d->kBrand3;
}

DTThemeData GlobalDTConfig::kSuccessNormal() const
{
    return d->kSuccessNormal;
}

DTThemeData GlobalDTConfig::kGrayAlpha1() const
{
    return d->kGrayAlpha1;
}

DTThemeData GlobalDTConfig::kFontWhiteSecondaryDisable() const
{
    return d->kFontWhiteSecondaryDisable;
}

DTThemeData GlobalDTConfig::kGray4() const
{
    return d->kGray4;
}

DTThemeData GlobalDTConfig::kContainClick() const
{
    return d->kContainClick;
}

DTThemeData GlobalDTConfig::kGray8() const
{
    return d->kGray8;
}

DTThemeData GlobalDTConfig::kGrayAlpha4() const
{
    return d->kGrayAlpha4;
}

DTThemeData GlobalDTConfig::kModalmask() const
{
    return d->kModalmask;
}

DTThemeData GlobalDTConfig::kLineBrandNormal() const
{
    return d->kLineBrandNormal;
}

DTThemeData GlobalDTConfig::kBrand5() const
{
    return d->kBrand5;
}

DTThemeData GlobalDTConfig::kGray10() const
{
    return d->kGray10;
}

DTThemeData GlobalDTConfig::kWarningNormal() const
{
    return d->kWarningNormal;
}

DTThemeData GlobalDTConfig::kGray9() const
{
    return d->kGray9;
}

DTThemeData GlobalDTConfig::kGray1() const
{
    return d->kGray1;
}

DTThemeData GlobalDTConfig::kGray14() const
{
    return d->kGray14;
}

DTThemeData GlobalDTConfig::kLineComponentClick() const
{
    return d->kLineComponentClick;
}

DTThemeData GlobalDTConfig::kWhite() const
{
    return d->kWhite;
}

DTThemeData GlobalDTConfig::kGray7() const
{
    return d->kGray7;
}

DTThemeData GlobalDTConfig::kBrandFocus() const
{
    return d->kBrandFocus;
}

DTThemeData GlobalDTConfig::kGrayAlpha9() const
{
    return d->kGrayAlpha9;
}

DTThemeData GlobalDTConfig::kGrayAlpha10() const
{
    return d->kGrayAlpha10;
}

DTThemeData GlobalDTConfig::kGrayAlpha6() const
{
    return d->kGrayAlpha6;
}

DTThemeData GlobalDTConfig::kContainHover() const
{
    return d->kContainHover;
}

DTThemeData GlobalDTConfig::kLineComponentDisable() const
{
    return d->kLineComponentDisable;
}

DTThemeData GlobalDTConfig::kFontWhite() const
{
    return d->kFontWhite;
}

DTThemeData GlobalDTConfig::kGrayAlpha11() const
{
    return d->kGrayAlpha11;
}

DTThemeData GlobalDTConfig::kGray6() const
{
    return d->kGray6;
}

DTThemeData GlobalDTConfig::kFontWhiteDisable() const
{
    return d->kFontWhiteDisable;
}

DTThemeData GlobalDTConfig::kComponentDisable() const
{
    return d->kComponentDisable;
}

DTThemeData GlobalDTConfig::kBlack() const
{
    return d->kBlack;
}

DTThemeData GlobalDTConfig::kComponentAlphaDisable() const
{
    return d->kComponentAlphaDisable;
}

DTThemeData GlobalDTConfig::kFontSecondaryDisable() const
{
    return d->kFontSecondaryDisable;
}

DTThemeData GlobalDTConfig::kGrayAlpha8() const
{
    return d->kGrayAlpha8;
}

DTThemeData GlobalDTConfig::kFontPrimaryDisable() const
{
    return d->kFontPrimaryDisable;
}

DTThemeData GlobalDTConfig::kGray5() const
{
    return d->kGray5;
}

DTThemeData GlobalDTConfig::kFontSecondary() const
{
    return d->kFontSecondary;
}

DTThemeData GlobalDTConfig::kErrorNormal() const
{
    return d->kErrorNormal;
}

DTThemeData GlobalDTConfig::kComponentHover() const
{
    return d->kComponentHover;
}

DTThemeData GlobalDTConfig::kComponentAlphaHover() const
{
    return d->kComponentAlphaHover;
}

DTThemeData GlobalDTConfig::kGrayAlpha3() const
{
    return d->kGrayAlpha3;
}

DTThemeData GlobalDTConfig::kGray13() const
{
    return d->kGray13;
}

DTThemeData GlobalDTConfig::kGray11() const
{
    return d->kGray11;
}

DTThemeData GlobalDTConfig::kErrorHover() const
{
    return d->kErrorHover;
}

DTThemeData GlobalDTConfig::kContainGeneralNormal() const
{
    return d->kContainGeneralNormal;
}

DTThemeData GlobalDTConfig::kComponentAlphaNormal() const
{
    return d->kComponentAlphaNormal;
}

DTThemeData GlobalDTConfig::kGray15() const
{
    return d->kGray15;
}

DTThemeData GlobalDTConfig::kBrand7() const
{
    return d->kBrand7;
}

DTThemeData GlobalDTConfig::kGrayAlpha7() const
{
    return d->kGrayAlpha7;
}

DTThemeData GlobalDTConfig::kLineBrandHover() const
{
    return d->kLineBrandHover;
}

DTThemeData GlobalDTConfig::kGrayAlpha13() const
{
    return d->kGrayAlpha13;
}

DTThemeData GlobalDTConfig::kLineComponentHover() const
{
    return d->kLineComponentHover;
}

DTThemeData GlobalDTConfig::kBrand6() const
{
    return d->kBrand6;
}

DTThemeData GlobalDTConfig::kGray17() const
{
    return d->kGray17;
}

DTThemeData GlobalDTConfig::kComponentClick() const
{
    return d->kComponentClick;
}

DTThemeData GlobalDTConfig::kBrandClick() const
{
    return d->kBrandClick;
}

DTThemeData GlobalDTConfig::kFontStrong() const
{
    return d->kFontStrong;
}

DTThemeData GlobalDTConfig::kLineWindow() const
{
    return d->kLineWindow;
}

DTThemeData GlobalDTConfig::kGray2() const
{
    return d->kGray2;
}

DTThemeData GlobalDTConfig::kBrand4() const
{
    return d->kBrand4;
}

DTThemeData GlobalDTConfig::kContainSecondaryNormal() const
{
    return d->kContainSecondaryNormal;
}

DTThemeData GlobalDTConfig::kBrandHover() const
{
    return d->kBrandHover;
}

DTThemeData GlobalDTConfig::kFontPrimary() const
{
    return d->kFontPrimary;
}

DTThemeData GlobalDTConfig::kBrandNormal() const
{
    return d->kBrandNormal;
}

DTThemeData GlobalDTConfig::normalline() const
{
    return d->normalline;
}

DTThemeData GlobalDTConfig::focusline() const
{
    return d->focusline;
}

DTThemeData GlobalDTConfig::tokenGap4() const
{
    return d->tokenGap4;
}

DTThemeData GlobalDTConfig::tokenGap8() const
{
    return d->tokenGap8;
}

DTThemeData GlobalDTConfig::tokenGap16() const
{
    return d->tokenGap16;
}

DTThemeData GlobalDTConfig::tokenGap24() const
{
    return d->tokenGap24;
}

DTThemeData GlobalDTConfig::tokenGap40() const
{
    return d->tokenGap40;
}

DTThemeData GlobalDTConfig::kradiusMin() const
{
    return d->kradiusMin;
}

DTThemeData GlobalDTConfig::kradiusMenu() const
{
    return d->kradiusMenu;
}

DTThemeData GlobalDTConfig::kradiusNormal() const
{
    return d->kradiusNormal;
}

DTThemeData GlobalDTConfig::kradiusWindow() const
{
    return d->kradiusWindow;
}

QMap<QString, DTThemeData> GlobalDTConfig::getDTDateMap() const
{
    return d->dtDataMap;
}

