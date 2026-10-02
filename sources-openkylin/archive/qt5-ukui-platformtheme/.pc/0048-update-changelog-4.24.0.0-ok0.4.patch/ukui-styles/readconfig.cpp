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

#include "readconfig.h"
#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QGSettings>
#include <QMetaEnum>
#include <QCoreApplication>
#include <QUrl>
#include <QBrush>
#include <QPainter>
#include "platformthemedebug.h"
#include "settings/black-list.h"

#define ThemeConfigPath "/usr/share/config/themeconfig/token/"
#define DefaultConfigName "kdefault-light.css"

using namespace UKUIGlobalDTConfig;

namespace {
QString designTokenPathToLocal(const QString &cfg)
{
    if (cfg.startsWith(QLatin1String("file:"), Qt::CaseInsensitive))
        return QUrl(cfg).toLocalFile();
    return cfg;
}
}

GlobalDTConfigPrivate::GlobalDTConfigPrivate()
{

    dtMap.insert("alternatebase-active"           , QVariant::fromValue(&alternateBaseActive));
    dtMap.insert("alternatebase-disable"          , QVariant::fromValue(&alternateBaseDisable));
    dtMap.insert("alternatebase-inactive"         , QVariant::fromValue(&alternateBaseInactive));

    dtMap.insert("base-active"                    , QVariant::fromValue(&baseActive));
    dtMap.insert("base-disable"                   , QVariant::fromValue(&baseDisable));
    dtMap.insert("base-inactive"                  , QVariant::fromValue(&baseInactive));

    dtMap.insert("brighttext-active"              , QVariant::fromValue(&brightTextActive));
    dtMap.insert("brighttext-disable"             , QVariant::fromValue(&brightTextDisable));
    dtMap.insert("brighttext-inactive"            , QVariant::fromValue(&brightTextInactive));

    dtMap.insert("button-active"                  , QVariant::fromValue(&buttonActive));
    dtMap.insert("button-disable"                 , QVariant::fromValue(&buttonDisable));
    dtMap.insert("button-inactive"                , QVariant::fromValue(&buttonInactive));
    dtMap.insert("buttontext-active"              , QVariant::fromValue(&buttonTextActive));
    dtMap.insert("buttontext-disable"             , QVariant::fromValue(&buttonTextDisable));
    dtMap.insert("buttontext-inactive"            , QVariant::fromValue(&buttonTextInactive));

    dtMap.insert("dark-active"                    , QVariant::fromValue(&darkActive));
    dtMap.insert("dark-disable"                   , QVariant::fromValue(&darkDisable));
    dtMap.insert("dark-inactive"                  , QVariant::fromValue(&darkInactive));

    dtMap.insert("focusline"                      , QVariant::fromValue(&focusline));

    dtMap.insert("highlight-active"               , QVariant::fromValue(&highLightActive));
    dtMap.insert("highlight-disable"              , QVariant::fromValue(&highLightDisable));
    dtMap.insert("highlightedtext-active"         , QVariant::fromValue(&highLightedTextActive));
    dtMap.insert("highlightedtext-disable"        , QVariant::fromValue(&highLightedTextDisable));
    dtMap.insert("highlightedtext-inactive"       , QVariant::fromValue(&highLightedTextInactive));
    dtMap.insert("highlight-inactive"             , QVariant::fromValue(&highLightInactive));

    dtMap.insert("kblack"                         , QVariant::fromValue(&kBlack));

    dtMap.insert("kbrand1"                        , QVariant::fromValue(&kBrand1));
    dtMap.insert("kbrand2"                        , QVariant::fromValue(&kBrand2));
    dtMap.insert("kbrand3"                        , QVariant::fromValue(&kBrand3));
    dtMap.insert("kbrand4"                        , QVariant::fromValue(&kBrand4));
    dtMap.insert("kbrand5"                        , QVariant::fromValue(&kBrand5));
    dtMap.insert("kbrand6"                        , QVariant::fromValue(&kBrand6));
    dtMap.insert("kbrand7"                        , QVariant::fromValue(&kBrand7));
    dtMap.insert("kbrand8"                        , QVariant::fromValue(&kBrand8));
    dtMap.insert("kbrand9"                        , QVariant::fromValue(&kBrand9));
    dtMap.insert("kbrand-click"                   , QVariant::fromValue(&kBrandClick  ));
    dtMap.insert("kbrand-disable"                 , QVariant::fromValue(&kBrandDisable));
    dtMap.insert("kbrand-focus"                   , QVariant::fromValue(&kBrandFocus  ));
    dtMap.insert("kbrand-hover"                   , QVariant::fromValue(&kBrandHover  ));
    dtMap.insert("kbrand-normal"                  , QVariant::fromValue(&kBrandNormal ));

    dtMap.insert("kcomponent-alpha-click"         , QVariant::fromValue(&kComponentAlphaClick  ));
    dtMap.insert("kcomponent-alpha-disable"       , QVariant::fromValue(&kComponentAlphaDisable));
    dtMap.insert("kcomponent-alpha-hover"         , QVariant::fromValue(&kComponentAlphaHover  ));
    dtMap.insert("kcomponent-alpha-normal"        , QVariant::fromValue(&kComponentAlphaNormal ));

    dtMap.insert("kcomponent-click"               , QVariant::fromValue(&kComponentClick));
    dtMap.insert("kcomponent-disable"             , QVariant::fromValue(&kComponentDisable));
    dtMap.insert("kcomponent-hover"               , QVariant::fromValue(&kComponentHover));
    dtMap.insert("kcomponent-normal"              , QVariant::fromValue(&kComponentNormal));

    dtMap.insert("kcomponent-selected-alpha-click"      ,QVariant::fromValue(&kComponentSelectedAlphaClick));
    dtMap.insert("kcomponent-selected-alpha-disable"    ,QVariant::fromValue(&kComponentSelectedAlphaDisable));
    dtMap.insert("kcomponent-selected-alpha-hover"      ,QVariant::fromValue(&kComponentSelectedAlphaHover));
    dtMap.insert("kcomponent-selected-alpha-normal"     ,QVariant::fromValue(&kComponentSelectedAlphaNormal));

    dtMap.insert("kcomponent-selected-click"        ,QVariant::fromValue(&kComponentSelectedClick));
    dtMap.insert("kcomponent-selected-disable"      ,QVariant::fromValue(&kComponentSelectedDisable));
    dtMap.insert("kcomponent-selected-hover"        ,QVariant::fromValue(&kComponentSelectedHover));
    dtMap.insert("kcomponent-selected-normal"       ,QVariant::fromValue(&kComponentSelectedNormal));

    dtMap.insert("kcontain-alpha-click"           , QVariant::fromValue(&kContainAlphaClick));
    dtMap.insert("kcontain-alpha-hover"           , QVariant::fromValue(&kContainAlphaHover));
    dtMap.insert("kcontain-click"                 , QVariant::fromValue(&kContainClick));
    dtMap.insert("kcontain-general-alpha-normal"  , QVariant::fromValue(&kContainGeneralAlphaNormal));
    dtMap.insert("kcontain-general-inactive"      , QVariant::fromValue(&kContainGeneralInactive));
    dtMap.insert("kcontain-general-normal"        , QVariant::fromValue(&kContainGeneralNormal));
    dtMap.insert("kcontain-hover"                 , QVariant::fromValue(&kContainHover));
    dtMap.insert("kcontain-secondary-alpha-normal", QVariant::fromValue(&kContainSecondaryAlphaNormal));
    dtMap.insert("kcontain-secondary-inactive"    , QVariant::fromValue(&kContainSecondaryInactive));
    dtMap.insert("kcontain-secondary-normal"      , QVariant::fromValue(&kContainSecondaryNormal));

    dtMap.insert("kdivider"                       , QVariant::fromValue(&kDivider));
    dtMap.insert("kdivider-white"                 , QVariant::fromValue(&kDividerWhite));

    dtMap.insert("kerror-click"                   , QVariant::fromValue(&kErrorClick  ));
    dtMap.insert("kerror-disable"                 , QVariant::fromValue(&kErrorDisable));
    dtMap.insert("kerror-hover"                   , QVariant::fromValue(&kErrorHover  ));
    dtMap.insert("kerror-normal"                  , QVariant::fromValue(&kErrorNormal  ));

    dtMap.insert("kfont-placeholdertext-disable"  , QVariant::fromValue(&kFontPlaceholdertextDisable ));
    dtMap.insert("kfont-placeholdertext"          , QVariant::fromValue(&kFontPlaceholdertext));
    dtMap.insert("kfont-primary-disable"          , QVariant::fromValue(&kFontPrimaryDisable));
    dtMap.insert("kfont-primary"                  , QVariant::fromValue(&kFontPrimary));
    dtMap.insert("kfont-secondary-disable"        , QVariant::fromValue(&kFontSecondaryDisable    ));
    dtMap.insert("kfont-secondary"                , QVariant::fromValue(&kFontSecondary           ));
    dtMap.insert("kfont-strong"                   , QVariant::fromValue(&kFontStrong              ));
    dtMap.insert("kfont-white-disable"            , QVariant::fromValue(&kFontWhiteDisable        ));
    dtMap.insert("kfont-white-placeholdertext-disable"    , QVariant::fromValue(&kFontWhitePlaceholdertextDisable));
    dtMap.insert("kfont-white-placeholdertext"            , QVariant::fromValue(&kFontWhiteplaceholdertext));
    dtMap.insert("kfont-white"                            , QVariant::fromValue(&kFontWhite));
    dtMap.insert("kfont-white-secondary-disable"          , QVariant::fromValue(&kFontWhiteSecondaryDisable));
    dtMap.insert("kfont-white-secondary"                  , QVariant::fromValue(&kFontWhiteSecondary));

    dtMap.insert("kgray-0"                        , QVariant::fromValue(&kGray0 ));
    dtMap.insert("kgray-10"                       , QVariant::fromValue(&kGray10 ));
    dtMap.insert("kgray-11"                       , QVariant::fromValue(&kGray11 ));
    dtMap.insert("kgray-12"                       , QVariant::fromValue(&kGray12 ));
    dtMap.insert("kgray-13"                       , QVariant::fromValue(&kGray13 ));
    dtMap.insert("kgray-14"                       , QVariant::fromValue(&kGray14 ));
    dtMap.insert("kgray-15"                       , QVariant::fromValue(&kGray15 ));
    dtMap.insert("kgray-16"                       , QVariant::fromValue(&kGray16 ));
    dtMap.insert("kgray-17"                       , QVariant::fromValue(&kGray17 ));
    dtMap.insert("kgray-1"                        , QVariant::fromValue(&kGray1 ));
    dtMap.insert("kgray-2"                        , QVariant::fromValue(&kGray2 ));
    dtMap.insert("kgray-3"                        , QVariant::fromValue(&kGray3 ));
    dtMap.insert("kgray-4"                        , QVariant::fromValue(&kGray4 ));
    dtMap.insert("kgray-5"                        , QVariant::fromValue(&kGray5 ));
    dtMap.insert("kgray-6"                        , QVariant::fromValue(&kGray6 ));
    dtMap.insert("kgray-7"                        , QVariant::fromValue(&kGray7 ));
    dtMap.insert("kgray-8"                        , QVariant::fromValue(&kGray8 ));
    dtMap.insert("kgray-9"                        , QVariant::fromValue(&kGray9 ));
    dtMap.insert("kgray-alpha0"                   , QVariant::fromValue(&kGrayAlpha0 ));
    dtMap.insert("kgray-alpha10"                  , QVariant::fromValue(&kGrayAlpha10));
    dtMap.insert("kgray-alpha11"                  , QVariant::fromValue(&kGrayAlpha11));
    dtMap.insert("kgray-alpha12"                  , QVariant::fromValue(&kGrayAlpha12));
    dtMap.insert("kgray-alpha13"                  , QVariant::fromValue(&kGrayAlpha13));
    dtMap.insert("kgray-alpha1"                   , QVariant::fromValue(&kGrayAlpha1 ));
    dtMap.insert("kgray-alpha2"                   , QVariant::fromValue(&kGrayAlpha2 ));
    dtMap.insert("kgray-alpha3"                   , QVariant::fromValue(&kGrayAlpha3 ));
    dtMap.insert("kgray-alpha4"                   , QVariant::fromValue(&kGrayAlpha4 ));
    dtMap.insert("kgray-alpha5"                   , QVariant::fromValue(&kGrayAlpha5 ));
    dtMap.insert("kgray-alpha6"                   , QVariant::fromValue(&kGrayAlpha6 ));
    dtMap.insert("kgray-alpha7"                   , QVariant::fromValue(&kGrayAlpha7 ));
    dtMap.insert("kgray-alpha8"                   , QVariant::fromValue(&kGrayAlpha8 ));
    dtMap.insert("kgray-alpha9"                   , QVariant::fromValue(&kGrayAlpha9 ));

    dtMap.insert("kgreen1"                        , QVariant::fromValue(&kGreen1));

    dtMap.insert("kline-brand-click"              , QVariant::fromValue(&kLineBrandClick      ));
    dtMap.insert("kline-brand-disable"            , QVariant::fromValue(&kLineBrandDisable    ));
    dtMap.insert("kline-brand-hover"              , QVariant::fromValue(&kLineBrandHover      ));
    dtMap.insert("kline-brand-normal"             , QVariant::fromValue(&kLineBrandNormal     ));

    dtMap.insert("kline-component-click"          , QVariant::fromValue(&kLineComponentClick  ));
    dtMap.insert("kline-component-disable"        , QVariant::fromValue(&kLineComponentDisable));
    dtMap.insert("kline-component-hover"          , QVariant::fromValue(&kLineComponentHover  ));
    dtMap.insert("kline-component-normal"         , QVariant::fromValue(&kLineComponentNormal ));

    dtMap.insert("kline-disable"                  , QVariant::fromValue(&kLineDisable));
    dtMap.insert("kline-dock"                     , QVariant::fromValue(&kLineDock));

    dtMap.insert("kline-error-click"              , QVariant::fromValue(&kLineErrorClick  ));
    dtMap.insert("kline-error-disable"            , QVariant::fromValue(&kLineErrorDisable));
    dtMap.insert("kline-error-hover"              , QVariant::fromValue(&kLineErrorHover  ));
    dtMap.insert("kline-error-normal"             , QVariant::fromValue(&kLineErrorNormal ));

    dtMap.insert("kline-input-click"              , QVariant::fromValue(&kLineInputClick));
    dtMap.insert("kline-input-disable"            , QVariant::fromValue(&kLineInputDisable     ));
    dtMap.insert("kline-input-hover"              , QVariant::fromValue(&kLineInputHover   ));
    dtMap.insert("kline-input-normal"             , QVariant::fromValue(&kLineInputNormal ));

    dtMap.insert("kline-menu"                     , QVariant::fromValue(&kLineMenu   ));
    dtMap.insert("kline-normal"                   , QVariant::fromValue(&kLineNormal           ));

    dtMap.insert("kline-selectbox-click"          , QVariant::fromValue(&kLineSelectboxClick             ));
    dtMap.insert("kline-selectbox-disable"        , QVariant::fromValue(&kLineSelectboxDisable      ));
    dtMap.insert("kline-selectbox-hover"          , QVariant::fromValue(&kLineSelectboxHover       ));
    dtMap.insert("kline-selectbox-normal"         , QVariant::fromValue(&kLineSelectboxNormal  ));

    dtMap.insert("kline-selectbox-selected"       , QVariant::fromValue(&kLineSelectboxSelected       ));

    dtMap.insert("kline-success-click"            , QVariant::fromValue(&kLineSuccessClick       ));
    dtMap.insert("kline-success-disable"          , QVariant::fromValue(&kLineSuccessDisable     ));
    dtMap.insert("kline-success-hover"            , QVariant::fromValue(&kLineSuccessHover       ));
    dtMap.insert("kline-success-normal"           , QVariant::fromValue(&kLineSuccessNormal      ));

    dtMap.insert("kline-table"                    , QVariant::fromValue(&kLineTable      ));

    dtMap.insert("kline-warning-click"            , QVariant::fromValue(&kLineWarningClick  ));
    dtMap.insert("kline-warning-disable"          , QVariant::fromValue(&kLineWarningDisable));
    dtMap.insert("kline-warning-hover"            , QVariant::fromValue(&kLineWarningHover  ));
    dtMap.insert("kline-warning-normal"           , QVariant::fromValue(&kLineWarningNormal ));

    dtMap.insert("kline-window-active"            , QVariant::fromValue(&kLineWindowActive  ));
    dtMap.insert("kline-window-inactive"          , QVariant::fromValue(&kLineWindowInactive));

    dtMap.insert("kmenu"                          , QVariant::fromValue(&kMenu              ));
    dtMap.insert("kmodalmask"                     , QVariant::fromValue(&kModalmask         ));
    dtMap.insert("korange1"                       , QVariant::fromValue(&kOrange1           ));

    dtMap.insert("kradius-max"                    , QVariant::fromValue(&kradiusMax   ));
    dtMap.insert("kradius-menu"                   , QVariant::fromValue(&kradiusMenu  ));
    dtMap.insert("kradius-min"                    , QVariant::fromValue(&kradiusMin   ));
    dtMap.insert("kradius-normal"                 , QVariant::fromValue(&kradiusNormal));
    dtMap.insert("kradius-window"                 , QVariant::fromValue(&kradiusWindow));

    dtMap.insert("kred1"                          , QVariant::fromValue(&kRed1));

    dtMap.insert("kselect-alpha-white-click"      , QVariant::fromValue(&kSelectAlphaWhiteClick  ));
    dtMap.insert("kselect-alpha-white-disable"    , QVariant::fromValue(&kSelectAlphaWhiteDisable));
    dtMap.insert("kselect-alpha-white-hover"      , QVariant::fromValue(&kSelectAlphaWhiteHover  ));
    dtMap.insert("kselect-alpha-white"            , QVariant::fromValue(&kSelectAlphaWhite       ));

    dtMap.insert("kshadow-component"              , QVariant::fromValue(&kshadowComponent));
    dtMap.insert("kshadow-menu"                   , QVariant::fromValue(&kshadowMenu          ));
    dtMap.insert("kshadow-min"                    , QVariant::fromValue(&kshadowMin           ));
    dtMap.insert("kshadow-primary-active"         , QVariant::fromValue(&kshadowPrimaryActive         ));
    dtMap.insert("kshadow-primary-inactive"       , QVariant::fromValue(&kshadowPrimaryInactive           ));
    dtMap.insert("kshadow-secondary-active"       , QVariant::fromValue(&kshadowSecondaryActive));
    dtMap.insert("kshadow-secondary-inactive"     , QVariant::fromValue(&kshadowSecondaryInactive  ));

    dtMap.insert("ksuccess-click"                 , QVariant::fromValue(&kSuccessClick  ));
    dtMap.insert("ksuccess-disable"               , QVariant::fromValue(&kSuccessDisable    ));
    dtMap.insert("ksuccess-hover"                 , QVariant::fromValue(&kSuccessHover              ));
    dtMap.insert("ksuccess-normal"                , QVariant::fromValue(&kSuccessNormal             ));

    dtMap.insert("kwarning-click"                 , QVariant::fromValue(&kWarningClick  ));
    dtMap.insert("kwarning-disable"               , QVariant::fromValue(&kWarningDisable));
    dtMap.insert("kwarning-hover"                 , QVariant::fromValue(&kWarningHover  ));
    dtMap.insert("kwarning-normal"                , QVariant::fromValue(&kWarningNormal ));

    dtMap.insert("kwhite"                         , QVariant::fromValue(&kWhite));

    dtMap.insert("light-active"                   , QVariant::fromValue(&lightActive));
    dtMap.insert("light-disable"                  , QVariant::fromValue(&lightDisable ));
    dtMap.insert("light-inactive"                 , QVariant::fromValue(&lightInactive  ));

    dtMap.insert("link-active"                    , QVariant::fromValue(&linkActive       ));
    dtMap.insert("link-disable"                   , QVariant::fromValue(&linkDisable        ));
    dtMap.insert("link-inactive"                  , QVariant::fromValue(&linkInactive         ));
    dtMap.insert("linkvisited-active"             , QVariant::fromValue(&linkVisitedActive      ));
    dtMap.insert("linkvisited-disable"            , QVariant::fromValue(&linkVisitedDisable       ));
    dtMap.insert("linkvisited-inactive"           , QVariant::fromValue(&linkVisitedInactive        ));

    dtMap.insert("mid-active"                     , QVariant::fromValue(&midActive      ));
    dtMap.insert("mid-disable"                    , QVariant::fromValue(&midDisable));
    dtMap.insert("mid-inactive"                   , QVariant::fromValue(&midInactive ));
    dtMap.insert("midlight-active"                , QVariant::fromValue(&midlightActive  ));
    dtMap.insert("midlight-disable"               , QVariant::fromValue(&midlightDisable     ));
    dtMap.insert("midlight-inactive"              , QVariant::fromValue(&midlightInactive      ));

    dtMap.insert("normalline"                     , QVariant::fromValue(&normalline       ));
    dtMap.insert("norole-active"                  , QVariant::fromValue(&noRoleActive           ));
    dtMap.insert("norole-disable"                 , QVariant::fromValue(&noRoleDisable          ));
    dtMap.insert("norole-inactive"                , QVariant::fromValue(&noRoleInactive         ));

    dtMap.insert("placeholdertext-active"         , QVariant::fromValue(&placeholderTextActive  ));
    dtMap.insert("placeholdertext-disable"        , QVariant::fromValue(&placeholderTextDisable ));
    dtMap.insert("placeholdertext-inactive"       , QVariant::fromValue(&placeholderTextInactive));

    dtMap.insert("shadow"                         , QVariant::fromValue(&shadow        ));
    dtMap.insert("shadow-active"                  , QVariant::fromValue(&shadowActive  ));
    dtMap.insert("shadow-disable"                 , QVariant::fromValue(&shadowDisable ));
    dtMap.insert("shadow-inactive"                , QVariant::fromValue(&shadowInactive));

    dtMap.insert("text-active"                    , QVariant::fromValue(&textActive  ));
    dtMap.insert("text-disable"                   , QVariant::fromValue(&textDisable ));
    dtMap.insert("text-inactive"                  , QVariant::fromValue(&textInactive));

    dtMap.insert("tooltipbase-active"             , QVariant::fromValue(&toolTipBaseActive  ));
    dtMap.insert("tooltipbase-disable"            , QVariant::fromValue(&toolTipBaseDisable ));
    dtMap.insert("tooltipbase-inactive"           , QVariant::fromValue(&toolTipBaseInactive));
    dtMap.insert("tooltiptext-active"             , QVariant::fromValue(&toolTipTextActive  ));
    dtMap.insert("tooltiptext-disable"            , QVariant::fromValue(&toolTipTextDisable ));
    dtMap.insert("tooltiptext-inactive"           , QVariant::fromValue(&toolTipTextInactive));

    dtMap.insert("window-active"                  , QVariant::fromValue(&windowActive      ));
    dtMap.insert("window-disable"                 , QVariant::fromValue(&windowDisable     ));
    dtMap.insert("window-inactive"                , QVariant::fromValue(&windowInactive    ));
    dtMap.insert("windowtext-active"              , QVariant::fromValue(&windowTextActive  ));
    dtMap.insert("windowtext-disable"             , QVariant::fromValue(&windowTextDisable ));
    dtMap.insert("windowtext-inactive"            , QVariant::fromValue(&windowTextInactive));

    dtMap.insert("kcomponent-input-alpha"         , QVariant::fromValue(&kComponentInputAlpha));
    dtMap.insert("kcomponent-input"               , QVariant::fromValue(&kComponentInput));


    init();
}

GlobalDTConfigPrivate::~GlobalDTConfigPrivate()
{

}

void GlobalDTConfigPrivate::init()
{
    QString radiusString;
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings settings("org.ukui.style");

        QString styleName = settings.get("styleName").toString();
        if (styleName == "ukui-light" || styleName == "ukui-white") {
            m_lightMode = "light";
        } else if (styleName == "ukui-dark" || styleName == "ukui-black") {
            m_lightMode = "dark";
        }
        if (useLightPaletteList().contains(qAppName())) {
            m_lightMode = "light";
        } else if (useDarkPaletteList().contains(qAppName())) {
            m_lightMode = "dark";
        }
        if(qApp && qApp->property("useLightTheme").isValid()&&qApp->property("useLightTheme").canConvert<bool>()){
            m_lightMode = "light";
        } else if (qApp && qApp->property("useDarkTheme").isValid()&&qApp->property("useDarkTheme").canConvert<bool>()) {
            m_lightMode = "dark";
        }
        radiusString = settings.get("window-radius").toString();
        m_widgetThemeName = settings.get("widgetThemeName").toString();
    }
    QString path;

    QByteArray envValue = qgetenv("XDG_DATA_DIRS");

        if (envValue.isEmpty()) {
            qWarning() << "XDG_DATA_DIRS is null!";
        } else {
            QString value = QString::fromUtf8(envValue);
            QStringList pathList = value.split(":");

            for(int i = 0; i < pathList.length(); i ++){
                path = pathList.at(i) + "/config/themeconfig/token/";
                if(QFile::exists(path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css")){
                    m_ukui_dt_path = path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css";
                    break;
                }
                if(QFile::exists(path + "k" + m_widgetThemeName + ".css")){
                    m_ukui_dt_path = path + "k" + m_widgetThemeName + ".css";
                    break;
                }
            }
        }
        qDebug() << "read DT file = " << m_ukui_dt_path;

//    //dt config
//        if(m_ukui_dt_path.isEmpty()){
//            path = ThemeConfigPath;
//            m_ukui_dt_path = (QFile::exists(path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css")) ?
//                        path + "k" + m_widgetThemeName + "-" + m_lightMode + ".css" :
//                        path + "k" + m_widgetThemeName + ".css";
//        }
    //qDebug() << "=========> m_ukui_dt_path = " << m_ukui_dt_path;
    m_ukui_dt_loadConfig = this->load(m_ukui_dt_path);

    initUKUIGlobalThemeParameters();

    kradiusWindow = radiusString.split(",").value(0).toInt();
    kradiusMenu = radiusString.split(",").value(1).toInt();
}


QColor GlobalDTConfigPrivate::composeColor(QColor foreColor, QColor backColor)
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

QColor GlobalDTConfigPrivate::mixColor(QList<QColor> colorList)
{
    //bottom color is the last one on the list.
    QColor color = colorList.takeLast();

    foreach (QColor c, colorList) {
        color = composeColor(c, color);
    }

    return color;
}

bool GlobalDTConfigPrivate::load(const QString &cfg)
{
    const QString localPath = designTokenPathToLocal(cfg);
    if (!QFile::exists(localPath)) {
        qWarning() << "TMPReadConfig load file not exists!" << cfg << "resolved:" << localPath;
        return false;
    }

    QFile file(localPath);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "TMPReadConfig load file Error!" << localPath;
        return false;
    }

    QByteArray configData = file.readAll();
    file.close();
    m_cfgPath = localPath;
    m_dt_configData = configData;
    return true;
}

QColor GlobalDTConfigPrivate::stringToColor(QString s) const
{
    QColor c = Qt::NoBrush;

    QStringList list = s.mid(s.indexOf("(") + 1, s.indexOf(")") - s.indexOf("(") - 1).split(",");
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

inline bool GlobalDTConfigPrivate::getColorValue(QBrush &dtBrush, const QString key)
{
    if (m_dt_configData.isEmpty()) {
        cDebug << "get fills array isempty!";
        return false;
    }

    if (!m_dt_configData.contains("--" + key + ":")) {
        cDebug << "Unvalid key!" << key;
        return false;
    }

    int start_index, end_index;
    QString colorStr;

    start_index = m_dt_configData.indexOf("--" + key + ":");
    if (start_index == -1) {
        cDebug <<"Config file format error! Has no key" << key;
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
        getColorValue(dtBrush, colorStr.mid(start_index, end_index - start_index).toLower());
    } else if (colorStr.contains(": rgba(")) {
        //solidcolor
        start_index = colorStr.indexOf(": rgba(");
        if (start_index == -1) {
            qWarning() << "Config file format error! type rgba" << key;
            return false;
        }
        start_index += QString(": rgba(").length();
        end_index = colorStr.indexOf(")", start_index);

        dtBrush = QBrush(stringToColor(colorStr.mid(start_index, end_index - start_index)));

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

        solidColor = mixColor(listColor);
        dtBrush = QBrush(solidColor);
    } else if (colorStr.contains(": linear-gradient(rgba")) {
        //mix solidcolor
        start_index = colorStr.indexOf(": linear-gradient(");
        if (start_index == -1) {
            qWarning() << "Config file format error! type linear-gradient rgb" << key;
            return false;
        }

        start_index += QString(": linear-gradient(").length();
        end_index = colorStr.indexOf(";", start_index);
        colorStr = colorStr.mid(start_index, end_index - start_index);
        colorStr = colorStr.replace(")),", "),");
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

        solidColor = mixColor(listColor);
        dtBrush = QBrush(solidColor);
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
        QLinearGradient gradient;

        QString sbackColor = sList.takeLast();
        sbackColor = sbackColor.mid(sbackColor.indexOf("(") + 1, sbackColor.indexOf(")") - sbackColor.indexOf("(") - 1);
        backColor = stringToColor(sbackColor);
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

        startColor = mixColor(startListColor);
        endColor   = mixColor(endListColor);

        gradient.setColorAt(0, startColor);
        gradient.setColorAt(1, endColor);
        dtBrush = QBrush(gradient);
    } else {
        cDebug << "Config file format error! " << key << "colorStr" << colorStr;
        return false;
    }

    return true;
}


inline bool GlobalDTConfigPrivate::getValue(int &value, const QString key) const
{
    if (m_dt_configData.isEmpty()) {
        cDebug << "get fills array isempty!";
        return false;
    }

    if (!m_dt_configData.contains(key)) {
        cDebug << "Unvalid key!";
        return false;
    }

    int start_index = m_dt_configData.indexOf("--" + key + ": ");
    if (start_index == -1) {
        cDebug << "Config file format error!";
        return false;
    }
    start_index += QString("--" + key + ": ").length();
    int end_index = m_dt_configData.indexOf("px", start_index);

    value = m_dt_configData.mid(start_index, end_index - start_index).toInt();

    return true;
}

void GlobalDTConfigPrivate::initUKUIGlobalThemeParameters()
{
    if (m_ukui_dt_loadConfig) {
        for(auto item = dtMap.begin(); item != dtMap.end(); item++){
            QVariant v = item.value();
            if(qApp && v.canConvert<QBrush*>()){
                QBrush* brushPtr = v.value<QBrush*>();
                getColorValue(*brushPtr, item.key());
                qApp->setProperty(item.key().toLocal8Bit(), *brushPtr );
            }
            else if(qApp && v.canConvert<int*>()){
                int* i = v.value<int*>();
                getValue(*i, item.key());
                qApp->setProperty(item.key().toLocal8Bit(), *i );
            }
        }

    }

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings settings("org.ukui.style");
//        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
//                if (key == "themeColor" ||key == "theme-color") {
//                // 获取当前的主题颜色
//                QString themeColor = settings->get("themeColor").value<QString>();
//                QColor color;
//                if (themeColor == "default") {

//                } else if (themeColor == "daybreakBlue") {
//                    color = QColor(55, 144, 250);
//                } else if (themeColor == "jamPurple") {
//                    color = QColor(120, 115, 245);
//                } else if (themeColor == "magenta") {
//                    color = QColor(235, 48, 150);
//                } else if (themeColor == "sunRed") {
//                    color = QColor(243, 34, 45);
//                } else if (themeColor == "sunsetOrange") {
//                   color = QColor(246, 140, 39);
//                } else if (themeColor == "dustGold") {
//                    color = QColor(249, 197, 61);
//                } else if (themeColor == "polarGreen") {
//                    color = QColor(82, 196, 41);
//                } else if (!themeColor.isEmpty()) {
//                    color = stringToColor(themeColor);
//                }
//                // 更新颜色
//                updateColorsBasedOnAccentColor(color);
//            }
//    });


    // 初始获取主题颜色
    QString initialThemeColorString = settings.get("themeColor").value<QString>();
    QColor initialThemeColor;
    if (initialThemeColorString == "default") {

    } else if (initialThemeColorString == "daybreakBlue") {
        initialThemeColor = QColor(55, 144, 250);
    } else if (initialThemeColorString == "jamPurple") {
        initialThemeColor = QColor(120, 115, 245);
    } else if (initialThemeColorString == "magenta") {
        initialThemeColor = QColor(235, 48, 150);
    } else if (initialThemeColorString == "sunRed") {
        initialThemeColor = QColor(243, 34, 45);
    } else if (initialThemeColorString == "sunsetOrange") {
       initialThemeColor = QColor(246, 140, 39);
    } else if (initialThemeColorString == "dustGold") {
        initialThemeColor = QColor(249, 197, 61);
    } else if (initialThemeColorString == "polarGreen") {
        initialThemeColor = QColor(82, 196, 41);
    } else if (!initialThemeColorString.isEmpty()) {
        initialThemeColor = stringToColor(initialThemeColorString);
    }
    updateColorsBasedOnAccentColor(initialThemeColor);
    }
}

/**
 * @brief 根据传入的强调色更新全局颜色配置
 *
 * 此函数用于基于给定的强调色（accentColor）更新一系列的颜色属性。它会将强调色应用于多个颜色相关的属性，
 * 同时处理包含特定颜色的 CSS 字符串，将其中的默认颜色替换为强调色，并将最终的颜色值设置为应用程序的属性。
 *
 * @param accentColor 作为基础的强调色，用于更新其他颜色属性
 */
void GlobalDTConfigPrivate::updateColorsBasedOnAccentColor(const QColor &accentColor)
{
    QBrush accentColorBrush = QBrush(QColor(accentColor));

    kBrandNormal = accentColorBrush;// 根据系统强调色，覆盖设置kbrandnormal色板颜色
    QColor disableColor = accentColor;
    disableColor.setAlphaF(0.45);
    kBrandDisable = QBrush(QColor(disableColor));
    linkInactive = accentColorBrush;
    highLightActive = accentColorBrush;
    linkActive = accentColorBrush;
    highLightInactive = accentColorBrush;

    kLineBrandNormal = accentColorBrush;
    kLineBrandHover = accentColorBrush;
//    kLineBrandDisable = accentColorBrush;
    kLineBrandClick = accentColorBrush;
    kBrand1 = accentColorBrush;
//    kBrandFocus = accentColorBrush;

//    QString cssStringhover = "linear-gradient(0deg, rgba(0, 0, 0, 0.05) 0%, rgba(0, 0, 0, 0.05) 100%), rgba(55, 144, 250, 1)";
//    QColor resultColor = processCSSColor(cssStringhover, accentColor);
//    kBrandHover = resultColor;

//    QString cssStringclick = "linear-gradient(0deg, rgba(0, 0, 0, 0.2) 0%, rgba(0, 0, 0, 0.2) 100%), rgba(55, 144, 250, 1)";
//    QColor resultColor1 = processCSSColor(cssStringclick, accentColor);
//    kBrandClick = resultColor1;

    adjustHighlightColor(highLightActive, getKeyValue("kbrand-hover"), getKeyValue("kbrand-click"));

    if(qApp){
        qApp->setProperty("kbrand-normal",                    kBrandNormal);
        qApp->setProperty("kbrand-hover",                     kBrandHover);
        qApp->setProperty("kbrand-click",                     kBrandClick);
        qApp->setProperty("kbrand-disable",                   kBrandDisable);


        qApp->setProperty("link-inactive",              linkInactive);
        qApp->setProperty("highlight-active",           highLightActive);
        qApp->setProperty("link-active",                linkActive);
        qApp->setProperty("highlight-inactive",         highLightInactive);

        qApp->setProperty("kline-brand-normal",               kLineBrandNormal);
        qApp->setProperty("kline-brand-hover",                kLineBrandHover);
        qApp->setProperty("kline-brand-disable",              kLineBrandDisable);
        qApp->setProperty("kline-brand-click",                kLineBrandClick);
        qApp->setProperty("kbrand1",                          kBrand1);
    }
//    qApp->setProperty("kbrand-focus",                     kBrandFocus);

}

/**
 * @brief 计算highlight  hover  click的颜色
 * @param highlightColor 高亮色
 */
void GlobalDTConfigPrivate::adjustHighlightColor(QBrush highlightColor, QString hoverStr, QString clickStr)
{
    bool isDark = true;
    if(m_lightMode == "light")
        isDark = false;
    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;
    QLinearGradient linearGradient1;

    if (hoverStr.contains("linear-gradient(180deg")) {
        if (isDark) {
            startColor = configMixColor(highlightColor.color(), QColor(Qt::white), 0.1);
            endColor = configMixColor(highlightColor.color(), QColor(Qt::black), 0.1);
        } else {
            startColor = configMixColor(highlightColor.color(), QColor(Qt::white), 0.1);
            endColor = configMixColor(highlightColor.color(), QColor(Qt::black), 0.1);
        }
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        kBrandHover = QBrush(linearGradient);
    } else {
        if (isDark) {
            kBrandHover = QBrush(configMixColor(highlightColor.color(), QColor(Qt::white), 0.05));
        } else {
            kBrandHover = QBrush(configMixColor(highlightColor.color(), QColor(Qt::black), 0.05));
        }
    }

    if (clickStr.contains("linear-gradient(180deg")) {
        if (isDark) {
            startColor = configMixColor(highlightColor.color(), QColor(Qt::white), 0.2);
            endColor = configMixColor(highlightColor.color(), QColor(Qt::black), 0.2);
        } else {
            startColor = configMixColor(highlightColor.color(), QColor(Qt::white), 0.2);
            endColor = configMixColor(highlightColor.color(), QColor(Qt::black), 0.2);
        }
        linearGradient1.setColorAt(0, startColor);
        linearGradient1.setColorAt(1, endColor);
        kBrandClick = QBrush(linearGradient1);
    } else {
        if (isDark) {
            kBrandClick = QBrush(configMixColor(highlightColor.color(), QColor(Qt::white), 0.2));
        } else {
            kBrandClick = QBrush(configMixColor(highlightColor.color(), QColor(Qt::black), 0.2));
        }
    }
}

qreal GlobalDTConfigPrivate::configMixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

QColor GlobalDTConfigPrivate::configMixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = configMixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = configMixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = configMixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = configMixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

QString GlobalDTConfigPrivate::getKeyValue(const QString key)
{
    if (m_dt_configData.isEmpty()) {
        cDebug << "get fills array isempty!";
        return "";
    }

    if (!m_dt_configData.contains("--" + key + ":")) {
        cDebug << "Unvalid key!" << key;
        return "";
    }

    int start_index, end_index;
    QString colorStr;

    start_index = m_dt_configData.indexOf("--" + key + ":");
    if (start_index == -1) {
        cDebug <<"Config file format error! Has no key" << key;
        return "";
    }

    start_index += QString("--" + key + ":").length();
    end_index = m_dt_configData.indexOf(";", start_index);
    colorStr = m_dt_configData.mid(start_index, end_index - start_index + 1);
    return colorStr;
}

/**
 * @brief processCSSColor 函数用于处理 CSS 字符串中的颜色信息，将特定的 rgba 颜色替换为指定颜色，
 *        提取所有的 rgba 颜色字符串并转换为 QColor 对象，最后将这些颜色混合得到一个最终颜色。
 *
 * @param cssString 输入的 CSS 字符串，其中可能包含一个或多个 rgba 颜色值。
 * @param replacementColor 用于替换特定 rgba 颜色（rgba(55, 144, 250, 1)）的 QColor 对象。
 * @return QColor 返回混合所有提取到的有效 rgba 颜色后的最终 QColor 对象。
 */
QColor GlobalDTConfigPrivate::processCSSColor(const QString &cssString, const QColor &replacementColor)
{
    // 构建新的 rgba 字符串
    QString newRGBA = QString("rgba(%1, %2, %3, %4)").arg(replacementColor.red()).arg(replacementColor.green()).arg(replacementColor.blue()).arg(replacementColor.alpha() / 255.0);

    // 查找原字符串中 rgba(55, 144, 250, 1) 并替换
    QString newCSSString = cssString;
    newCSSString.replace("rgba(55, 144, 250, 1)", newRGBA);

    // 提取所有 rgba 颜色字符串
    QStringList rgbaStrings;
    int index = 0;
    while ((index = newCSSString.indexOf("rgba", index)) != -1) {
        int start = newCSSString.indexOf("(", index);
        int end = newCSSString.indexOf(")", start);
        if (start != -1 && end != -1) {
            rgbaStrings.append(newCSSString.mid(index, end - index + 1));
            index = end + 1;
        } else {
            break;
        }
    }

    // 将 rgba 字符串转换为 QColor 对象
    QList<QColor> colors;
    for (const QString& rgba : rgbaStrings) {
        QColor color = stringToColor(rgba);
        if (color.isValid()) {
            colors.append(color);
        }
    }

    // 混合所有颜色
    return mixColor(colors);
}

GlobalDTConfig::GlobalDTConfig() : d(new GlobalDTConfigPrivate())
{
//    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
//        auto *settings = new QGSettings("org.ukui.style");
//        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
//            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor" ||
//                    key == "style-name" || key == "widget-theme-name" || key == "theme-color" ||
//                    key == "window-radius" || key == "windowRadius") {
//                d->init();
//                emit tokenChanged();
//            }
//        });
//    }
}

GlobalDTConfig::~GlobalDTConfig()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QBrush GlobalDTConfig::windowTextActive() const
{
    return d->windowTextActive;
}

QBrush GlobalDTConfig::windowTextInactive() const
{
    return d->windowTextInactive;
}

QBrush GlobalDTConfig::windowTextDisable() const
{
    return d->windowTextDisable;
}

QBrush GlobalDTConfig::buttonActive() const
{
    return d->buttonActive;
}

QBrush GlobalDTConfig::buttonInactive() const
{
    return d->buttonInactive;
}

QBrush GlobalDTConfig::buttonDisable() const
{
    return d->buttonDisable;
}

QBrush GlobalDTConfig::lightActive() const
{
    return d->lightActive;
}

QBrush GlobalDTConfig::lightInactive() const
{
    return d->lightInactive;
}

QBrush GlobalDTConfig::lightDisable() const
{
    return d->lightDisable;
}

QBrush GlobalDTConfig::midlightActive() const
{
    return d->midlightActive;
}

QBrush GlobalDTConfig::midlightInactive() const
{
    return d->midlightInactive;
}

QBrush GlobalDTConfig::midlightDisable() const
{
    return d->midlightDisable;
}

QBrush GlobalDTConfig::darkActive() const
{
    return d->darkActive;
}

QBrush GlobalDTConfig::darkInactive() const
{
    return d->darkInactive;
}

QBrush GlobalDTConfig::darkDisable() const
{
    return d->darkDisable;
}

QBrush GlobalDTConfig::midActive() const
{
    return d->midActive;
}

QBrush GlobalDTConfig::midInactive() const
{
    return d->midInactive;
}

QBrush GlobalDTConfig::midDisable() const
{
    return d->midDisable;
}

QBrush GlobalDTConfig::textActive() const
{
    return d->textActive;
}

QBrush GlobalDTConfig::textInactive() const
{
    return d->textInactive;
}

QBrush GlobalDTConfig::textDisable() const
{
    return d->textDisable;
}

QBrush GlobalDTConfig::brightTextActive() const
{
    return d->brightTextActive;
}

QBrush GlobalDTConfig::brightTextInactive() const
{
    return d->brightTextInactive;
}

QBrush GlobalDTConfig::brightTextDisable() const
{
    return d->brightTextDisable;
}

QBrush GlobalDTConfig::buttonTextActive() const
{
    return d->buttonTextActive;
}

QBrush GlobalDTConfig::buttonTextInactive() const
{
    return d->buttonTextInactive;
}

QBrush GlobalDTConfig::buttonTextDisable() const
{
    return d->buttonTextDisable;
}

QBrush GlobalDTConfig::baseActive() const
{
    return d->baseActive;
}

QBrush GlobalDTConfig::baseInactive() const
{
    return d->baseInactive;
}

QBrush GlobalDTConfig::baseDisable() const
{
    return d->baseDisable;
}

QBrush GlobalDTConfig::windowActive() const
{
    return d->windowActive;
}

QBrush GlobalDTConfig::windowInactive() const
{
    return d->windowInactive;
}

QBrush GlobalDTConfig::windowDisable() const
{
    return d->windowDisable;
}

QBrush GlobalDTConfig::shadowActive() const
{
    return d->shadowActive;
}

QBrush GlobalDTConfig::shadowInactive() const
{
    return d->shadowInactive;
}

QBrush GlobalDTConfig::shadowDisable() const
{
    return d->shadowDisable;
}

QBrush GlobalDTConfig::highLightActive() const
{
    return d->highLightActive;
}

QBrush GlobalDTConfig::highLightInactive() const
{
    return d->highLightInactive;
}

QBrush GlobalDTConfig::highLightDisable() const
{
    return d->highLightDisable;
}

QBrush GlobalDTConfig::highLightedTextActive() const
{
    return d->highLightedTextActive;
}

QBrush GlobalDTConfig::highLightedTextInactive() const
{
    return d->highLightedTextInactive;
}

QBrush GlobalDTConfig::highLightedTextDisable() const
{
    return d->highLightedTextDisable;
}

QBrush GlobalDTConfig::linkActive() const
{
    return d->linkActive;
}

QBrush GlobalDTConfig::linkInactive() const
{
    return d->linkInactive;
}

QBrush GlobalDTConfig::linkDisable() const
{
    return d->linkDisable;
}

QBrush GlobalDTConfig::linkVisitedActive() const
{
    return d->linkVisitedActive;
}

QBrush GlobalDTConfig::linkVisitedInactive() const
{
    return d->linkVisitedInactive;
}

QBrush GlobalDTConfig::linkVisitedDisable() const
{
    return d->linkVisitedDisable;
}

QBrush GlobalDTConfig::alternateBaseActive() const
{
    return d->alternateBaseActive;
}

QBrush GlobalDTConfig::alternateBaseInactive() const
{
    return d->alternateBaseInactive;
}

QBrush GlobalDTConfig::alternateBaseDisable() const
{
    return d->alternateBaseDisable;
}

QBrush GlobalDTConfig::noRoleActive() const
{
    return d->noRoleActive;
}

QBrush GlobalDTConfig::noRoleInactive() const
{
    return d->noRoleInactive;
}

QBrush GlobalDTConfig::noRoleDisable() const
{
    return d->noRoleDisable;
}

QBrush GlobalDTConfig::toolTipBaseActive() const
{
    return d->toolTipBaseActive;
}

QBrush GlobalDTConfig::toolTipBaseInactive() const
{
    return d->toolTipBaseInactive;
}

QBrush GlobalDTConfig::toolTipBaseDisable() const
{
    return d->toolTipBaseDisable;
}

QBrush GlobalDTConfig::toolTipTextActive() const
{
    return d->toolTipTextActive;
}

QBrush GlobalDTConfig::toolTipTextInactive() const
{
    return d->toolTipTextInactive;
}

QBrush GlobalDTConfig::toolTipTextDisable() const
{
    return d->toolTipTextDisable;
}

QBrush GlobalDTConfig::placeholderTextActive() const
{
    return d->placeholderTextActive;
}

QBrush GlobalDTConfig::placeholderTextInactive() const
{
    return d->placeholderTextInactive;
}

QBrush GlobalDTConfig::placeholderTextDisable() const
{
    return d->placeholderTextDisable;
}

QBrush GlobalDTConfig::kComponentNormal() const
{
    return d->kComponentNormal;
}
QBrush GlobalDTConfig::kComponentAlphaNormal() const
{
    return d->kComponentAlphaNormal;
}
QBrush GlobalDTConfig::kComponentInput() const
{
    return d->kComponentInput;
}
QBrush GlobalDTConfig::kComponentInputAlpha() const
{
    return d->kComponentInputAlpha;
}

QBrush GlobalDTConfig::kComponentSelectedAlphaDisable() const
{
    return d->kComponentSelectedAlphaDisable;
}
QBrush GlobalDTConfig::kComponentSelectedAlphaNormal() const
{
    return d->kComponentSelectedAlphaNormal;
}
QBrush GlobalDTConfig::kComponentSelectedAlphaHover() const
{
    return d->kComponentSelectedAlphaHover;
}
QBrush GlobalDTConfig::kComponentSelectedAlphaClick() const
{
    return d->kComponentSelectedAlphaClick;
}

QBrush GlobalDTConfig::kComponentSelectedClick() const
{
    return d->kComponentSelectedClick;
}
QBrush GlobalDTConfig::kComponentSelectedHover() const
{
    return d->kComponentSelectedHover;
}
QBrush GlobalDTConfig::kComponentSelectedDisable() const
{
    return d->kComponentSelectedDisable;
}
QBrush GlobalDTConfig::kComponentSelectedNormal() const
{
    return d->kComponentSelectedNormal;
}


QBrush GlobalDTConfig::kLineBrandDisable() const
{
    return d->kLineBrandDisable;
}

QBrush GlobalDTConfig::kFontWhiteSecondary() const
{
    return d->kFontWhiteSecondary;
}

QBrush GlobalDTConfig::kGray12() const
{
    return d->kGray12;
}

QBrush GlobalDTConfig::kGray16() const
{
    return d->kGray16;
}

QBrush GlobalDTConfig::kBrand1() const
{
    return d->kBrand1;
}

QBrush GlobalDTConfig::kLineComponentNormal() const
{
    return d->kLineComponentNormal;
}

QBrush GlobalDTConfig::kLineBrandClick() const
{
    return d->kLineBrandClick;
}

QBrush GlobalDTConfig::kGray3() const
{
    return d->kGray3;
}

QBrush GlobalDTConfig::kGray0() const
{
    return d->kGray0;
}

QBrush GlobalDTConfig::kLineDisable() const
{
    return d->kLineDisable;
}

QBrush GlobalDTConfig::kGrayAlpha2() const
{
    return d->kGrayAlpha2;
}

QBrush GlobalDTConfig::kGrayAlpha0() const
{
    return d->kGrayAlpha0;
}

QBrush GlobalDTConfig::kBrand2() const
{
    return d->kBrand2;
}

QBrush GlobalDTConfig::kLineNormal() const
{
    return d->kLineNormal;
}

QBrush GlobalDTConfig::kGrayAlpha5() const
{
    return d->kGrayAlpha5;
}

QBrush GlobalDTConfig::kGrayAlpha12() const
{
    return d->kGrayAlpha12;
}

QBrush GlobalDTConfig::kErrorClick() const
{
    return d->kErrorClick;
}

QBrush GlobalDTConfig::kLineWindowActive() const
{
    return d->kLineWindowActive;
}

QBrush GlobalDTConfig::kComponentAlphaClick() const
{
    return d->kComponentAlphaClick;
}

QBrush GlobalDTConfig::kBrand3() const
{
    return d->kBrand3;
}

QBrush GlobalDTConfig::kSuccessNormal() const
{
    return d->kSuccessNormal;
}

QBrush GlobalDTConfig::kGrayAlpha1() const
{
    return d->kGrayAlpha1;
}

QBrush GlobalDTConfig::kFontWhiteSecondaryDisable() const
{
    return d->kFontWhiteSecondaryDisable;
}

QBrush GlobalDTConfig::kGray4() const
{
    return d->kGray4;
}

QBrush GlobalDTConfig::kContainClick() const
{
    return d->kContainClick;
}

QBrush GlobalDTConfig::kGray8() const
{
    return d->kGray8;
}

QBrush GlobalDTConfig::kGrayAlpha4() const
{
    return d->kGrayAlpha4;
}

QBrush GlobalDTConfig::kModalmask() const
{
    return d->kModalmask;
}

QBrush GlobalDTConfig::kLineBrandNormal() const
{
    return d->kLineBrandNormal;
}

QBrush GlobalDTConfig::kBrand5() const
{
    return d->kBrand5;
}

QBrush GlobalDTConfig::kGray10() const
{
    return d->kGray10;
}

QBrush GlobalDTConfig::kWarningNormal() const
{
    return d->kWarningNormal;
}

QBrush GlobalDTConfig::kGray9() const
{
    return d->kGray9;
}

QBrush GlobalDTConfig::kGray1() const
{
    return d->kGray1;
}

QBrush GlobalDTConfig::kGray14() const
{
    return d->kGray14;
}

QBrush GlobalDTConfig::kLineComponentClick() const
{
    return d->kLineComponentClick;
}

QBrush GlobalDTConfig::kWhite() const
{
    return d->kWhite;
}

QBrush GlobalDTConfig::kGray7() const
{
    return d->kGray7;
}

QBrush GlobalDTConfig::kBrandFocus() const
{
    return d->kBrandFocus;
}

QBrush GlobalDTConfig::kGrayAlpha9() const
{
    return d->kGrayAlpha9;
}

QBrush GlobalDTConfig::kGrayAlpha10() const
{
    return d->kGrayAlpha10;
}

QBrush GlobalDTConfig::kGrayAlpha6() const
{
    return d->kGrayAlpha6;
}

QBrush GlobalDTConfig::kContainHover() const
{
    return d->kContainHover;
}

QBrush GlobalDTConfig::kLineComponentDisable() const
{
    return d->kLineComponentDisable;
}

QBrush GlobalDTConfig::kFontWhite() const
{
    return d->kFontWhite;
}

QBrush GlobalDTConfig::kGrayAlpha11() const
{
    return d->kGrayAlpha11;
}

QBrush GlobalDTConfig::kGray6() const
{
    return d->kGray6;
}

QBrush GlobalDTConfig::kFontWhiteDisable() const
{
    return d->kFontWhiteDisable;
}

QBrush GlobalDTConfig::kComponentDisable() const
{
    return d->kComponentDisable;
}

QBrush GlobalDTConfig::kBlack() const
{
    return d->kBlack;
}

QBrush GlobalDTConfig::kComponentAlphaDisable() const
{
    return d->kComponentAlphaDisable;
}

QBrush GlobalDTConfig::kFontSecondaryDisable() const
{
    return d->kFontSecondaryDisable;
}

QBrush GlobalDTConfig::kGrayAlpha8() const
{
    return d->kGrayAlpha8;
}

QBrush GlobalDTConfig::kFontPrimaryDisable() const
{
    return d->kFontPrimaryDisable;
}

QBrush GlobalDTConfig::kGray5() const
{
    return d->kGray5;
}

QBrush GlobalDTConfig::kFontSecondary() const
{
    return d->kFontSecondary;
}

QBrush GlobalDTConfig::kErrorNormal() const
{
    return d->kErrorNormal;
}

QBrush GlobalDTConfig::kComponentHover() const
{
    return d->kComponentHover;
}

QBrush GlobalDTConfig::kComponentAlphaHover() const
{
    return d->kComponentAlphaHover;
}

QBrush GlobalDTConfig::kGrayAlpha3() const
{
    return d->kGrayAlpha3;
}

QBrush GlobalDTConfig::kGray13() const
{
    return d->kGray13;
}

QBrush GlobalDTConfig::kGray11() const
{
    return d->kGray11;
}

QBrush GlobalDTConfig::kErrorHover() const
{
    return d->kErrorHover;
}

QBrush GlobalDTConfig::kContainGeneralNormal() const
{
    return d->kContainGeneralNormal;
}


QBrush GlobalDTConfig::kGray15() const
{
    return d->kGray15;
}

QBrush GlobalDTConfig::kBrand7() const
{
    return d->kBrand7;
}

QBrush GlobalDTConfig::kGrayAlpha7() const
{
    return d->kGrayAlpha7;
}

QBrush GlobalDTConfig::kLineBrandHover() const
{
    return d->kLineBrandHover;
}

QBrush GlobalDTConfig::kGrayAlpha13() const
{
    return d->kGrayAlpha13;
}

QBrush GlobalDTConfig::kLineComponentHover() const
{
    return d->kLineComponentHover;
}

QBrush GlobalDTConfig::kBrand6() const
{
    return d->kBrand6;
}

QBrush GlobalDTConfig::kGray17() const
{
    return d->kGray17;
}

QBrush GlobalDTConfig::kComponentClick() const
{
    return d->kComponentClick;
}

QBrush GlobalDTConfig::kBrandClick() const
{
    return d->kBrandClick;
}

QBrush GlobalDTConfig::kFontStrong() const
{
    return d->kFontStrong;
}

QBrush GlobalDTConfig::kGray2() const
{
    return d->kGray2;
}

QBrush GlobalDTConfig::kBrand4() const
{
    return d->kBrand4;
}

QBrush GlobalDTConfig::kContainSecondaryNormal() const
{
    return d->kContainSecondaryNormal;
}

QBrush GlobalDTConfig::kBrandHover() const
{
    return d->kBrandHover;
}

QBrush GlobalDTConfig::kFontPrimary() const
{
    return d->kFontPrimary;
}

QBrush GlobalDTConfig::kBrandNormal() const
{
    return d->kBrandNormal;
}

int GlobalDTConfig::normalline() const
{
    return d->normalline;
}

int GlobalDTConfig::focusline() const
{
    return d->focusline;
}

int GlobalDTConfig::kradiusMin() const
{
    return d->kradiusMin;
}

int GlobalDTConfig::kradiusMenu() const
{
    return d->kradiusMenu;
}

int GlobalDTConfig::kradiusNormal() const
{
    return d->kradiusNormal;
}

int GlobalDTConfig::kradiusWindow() const
{
    return d->kradiusWindow;
}



QBrush GlobalDTConfig::kBrand8() const
{
    return d->kBrand8;
}

QBrush GlobalDTConfig::kBrand9() const
{
    return d->kBrand9;
}

QBrush GlobalDTConfig::kBrandDisable() const
{
    return d->kBrandDisable;
}

QBrush GlobalDTConfig::kContainAlphaClick() const
{
    return d->kContainAlphaClick;
}

QBrush GlobalDTConfig::kContainAlphaHover() const
{
    return d->kContainAlphaHover;
}

QBrush GlobalDTConfig::kContainGeneralAlphaNormal() const
{
    return d->kContainGeneralAlphaNormal;
}

QBrush GlobalDTConfig::kContainGeneralInactive() const
{
    return d->kContainGeneralInactive;
}

QBrush GlobalDTConfig::kContainSecondaryAlphaNormal() const
{
    return d->kContainSecondaryAlphaNormal;
}

QBrush GlobalDTConfig::kContainSecondaryInactive() const
{
    return d->kContainSecondaryInactive;
}

QBrush GlobalDTConfig::kDivider() const
{
    return d->kDivider;
}

QBrush GlobalDTConfig::kDividerWhite() const
{
    return d->kDividerWhite;
}

QBrush GlobalDTConfig::kErrorDisable() const
{
    return d->kErrorDisable;
}

QBrush GlobalDTConfig::kFontPlaceholdertextDisable() const
{
    return d->kFontPlaceholdertextDisable;
}

QBrush GlobalDTConfig::kFontPlaceholdertext() const
{
    return d->kFontPlaceholdertext;
}

QBrush GlobalDTConfig::kFontWhitePlaceholdertextDisable() const
{
    return d->kFontWhitePlaceholdertextDisable;
}

QBrush GlobalDTConfig::kFontWhiteplaceholdertext() const
{
    return d->kFontWhiteplaceholdertext;
}

QBrush GlobalDTConfig::kGreen1() const
{
    return d->kGreen1;
}

QBrush GlobalDTConfig::kLineDock() const
{
    return d->kLineDock;
}

QBrush GlobalDTConfig::kLineErrorClick() const
{
    return d->kLineErrorClick;
}

QBrush GlobalDTConfig::kLineErrorDisable() const
{
    return d->kLineErrorDisable;
}

QBrush GlobalDTConfig::kLineErrorHover() const
{
    return d->kLineErrorHover;
}

QBrush GlobalDTConfig::kLineErrorNormal() const
{
    return d->kLineErrorNormal;
}

QBrush GlobalDTConfig::kLineInputClick() const
{
    return d->kLineInputClick;
}

QBrush GlobalDTConfig::kLineInputDisable() const
{
    return d->kLineInputDisable;
}

QBrush GlobalDTConfig::kLineInputHover() const
{
    return d->kLineInputHover;
}

QBrush GlobalDTConfig::kLineInputNormal() const
{
    return d->kLineInputNormal;
}

QBrush GlobalDTConfig::kLineMenu() const
{
    return d->kLineMenu;
}

QBrush GlobalDTConfig::kLineSelectboxClick() const
{
    return d->kLineSelectboxClick;
}

QBrush GlobalDTConfig::kLineSelectboxDisable() const
{
    return d->kLineSelectboxDisable;
}

QBrush GlobalDTConfig::kLineSelectboxHover() const
{
    return d->kLineSelectboxHover;
}

QBrush GlobalDTConfig::kLineSelectboxNormal() const
{
    return d->kLineSelectboxNormal;
}

QBrush GlobalDTConfig::kLineSelectboxSelected() const
{
    return d->kLineSelectboxSelected;
}

QBrush GlobalDTConfig::kLineSuccessClick() const
{
    return d->kLineSuccessClick;
}

QBrush GlobalDTConfig::kLineSuccessDisable() const
{
    return d->kLineSuccessDisable;
}

QBrush GlobalDTConfig::kLineSuccessHover() const
{
    return d->kLineSuccessHover;
}

QBrush GlobalDTConfig::kLineSuccessNormal() const
{
    return d->kLineSuccessNormal;
}

QBrush GlobalDTConfig::kLineTable() const
{
    return d->kLineTable;
}

QBrush GlobalDTConfig::kLineWarningClick() const
{
    return d->kLineWarningClick;
}

QBrush GlobalDTConfig::kLineWarningDisable() const
{
    return d->kLineWarningDisable;
}

QBrush GlobalDTConfig::kLineWarningHover() const
{
    return d->kLineWarningHover;
}

QBrush GlobalDTConfig::kLineWarningNormal() const
{
    return d->kLineWarningNormal;
}

QBrush GlobalDTConfig::kLineWindowInactive() const
{
    return d->kLineWindowInactive;
}

QBrush GlobalDTConfig::kMenu() const
{
    return d->kMenu;
}

QBrush GlobalDTConfig::kOrange1() const
{
    return d->kOrange1;
}

int GlobalDTConfig::kradiusMax() const
{
    return d->kradiusMax;
}

QBrush GlobalDTConfig::kRed1() const
{
    return d->kRed1;
}

QBrush GlobalDTConfig::kSelectAlphaWhiteClick() const
{
    return d->kSelectAlphaWhiteClick;
}

QBrush GlobalDTConfig::kSelectAlphaWhiteDisable() const
{
    return d->kSelectAlphaWhiteDisable;
}

QBrush GlobalDTConfig::kSelectAlphaWhiteHover() const
{
    return d->kSelectAlphaWhiteHover;
}

QBrush GlobalDTConfig::kSelectAlphaWhite() const
{
    return d->kSelectAlphaWhite;
}

QBrush GlobalDTConfig::kshadowComponent() const
{
    return d->kshadowComponent;
}

QBrush GlobalDTConfig::kshadowMenu() const
{
    return d->kshadowMenu;
}

QBrush GlobalDTConfig::kshadowMin() const
{
    return d->kshadowMin;
}

QBrush GlobalDTConfig::kshadowPrimaryActive() const
{
    return d->kshadowPrimaryActive;
}

QBrush GlobalDTConfig::kshadowPrimaryInactive() const
{
    return d->kshadowPrimaryInactive;
}

QBrush GlobalDTConfig::kshadowSecondaryActive() const
{
    return d->kshadowSecondaryActive;
}

QBrush GlobalDTConfig::kshadowSecondaryInactive() const
{
    return d->kshadowSecondaryInactive;
}

QBrush GlobalDTConfig::kSuccessClick() const
{
    return d->kSuccessClick;
}

QBrush GlobalDTConfig::kSuccessDisable() const
{
    return d->kSuccessDisable;
}

QBrush GlobalDTConfig::kSuccessHover() const
{
    return d->kSuccessHover;
}

QBrush GlobalDTConfig::kWarningClick() const
{
    return d->kWarningClick;
}

QBrush GlobalDTConfig::kWarningDisable() const
{
    return d->kWarningDisable;
}

QBrush GlobalDTConfig::kWarningHover() const
{
    return d->kWarningHover;
}

QBrush GlobalDTConfig::shadow() const
{
    return d->shadow;
}

QPalette GlobalDTConfig::appPalette()
{
    QPalette defaultPalette;
    defaultPalette.setColor(QPalette::Active, QPalette::WindowText, windowTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::WindowText, windowTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::WindowText, windowTextDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Button, buttonActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Button, buttonInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Button, buttonDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Light, lightActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Light, lightInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Light, lightDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Midlight, midlightActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Midlight, midlightInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Midlight, midlightDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Dark, darkActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Dark, darkInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Dark, darkDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Mid, midActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Mid, midInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Mid, midDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Text, textActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Text, textInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Text, textDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::BrightText, brightTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::BrightText, brightTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::BrightText, brightTextDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::ButtonText, buttonTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonTextDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Base, baseActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Base, baseInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Base, baseDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Window, windowActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Window, windowInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Window, windowDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Shadow, shadowActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Shadow, shadowInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Shadow, shadowDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Highlight, highLightActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Highlight, highLightInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Highlight, highLightDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::HighlightedText, highLightedTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightedTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightedTextDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::Link, linkActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::Link, linkInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::Link, linkDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisitedActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisitedInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisitedDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBaseActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBaseInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBaseDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::NoRole, noRoleActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::NoRole, noRoleInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::NoRole, noRoleDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBaseActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBaseInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBaseDisable().color());

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipTextDisable().color());

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    defaultPalette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderTextActive().color());
    defaultPalette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderTextInactive().color());
    defaultPalette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderTextDisable().color());
#endif

    return defaultPalette;
}

void GlobalDTConfig::init()
{
    d->init();
    emit tokenChanged();
}

bool GlobalDTConfig::loadDesignTokenFile(const QString &cfgPathOrUri)
{
    if (!d->load(cfgPathOrUri))
        return false;
    d->m_ukui_dt_path = designTokenPathToLocal(cfgPathOrUri);
    d->m_ukui_dt_loadConfig = true;
    d->initUKUIGlobalThemeParameters();
    emit tokenChanged();
    return true;
}
