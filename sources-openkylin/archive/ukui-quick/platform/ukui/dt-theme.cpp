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

#include "dt-theme.h"

#include <QFile>
#include <QQmlComponent>
#include <QVariant>
#include <functional>
#include <mutex>
#include <QGSettings>
#include <QGuiApplication>
#include <QRegularExpression>

#include "ukui-theme-proxy.h"

static QHash<QQmlEngine *, UkuiQuick::DtTheme *> g_dtThemes;

namespace UkuiQuick {
#define COLOR_TOKEN_GETTER(token)                       \
    GradientColor* DtTheme::token() const { return &(d->m_##token); }
#define INT_TOKEN_GETTER(token)                         \
    int DtTheme::token() const { return d->m_##token; }
#define SHADOW_TOKEN_GETTER(token)                       \
    ShadowData* DtTheme::token() const { return &(d->m_##token); }
#define COLOR_TOKEN_SETTER(token)                       \
    void DtTheme::token##Set(QVariant value)            \
    {                                                   \
        GradientColor* gra = value.value<UkuiQuick::GradientColor*>();\
        if(gra == nullptr) {                            \
            QColor color = value.value<QColor>();       \
            if(d->m_##token != color) {                 \
                d->m_##token.setGradientDeg(0);         \
                d->m_##token.setGradientStart(QColor(0,0,0,0)); \
                d->m_##token.setGradientEnd(QColor(0,0,0,0)); \
                d->m_##token.setGradientBackground(QColor(0,0,0,0)); \
                d->m_##token.setPureColor(color);       \
                d->m_##token.setColorType(DtColorType::Pure);\
                Q_EMIT token##Changed();                \
            }                                           \
        } else {                                        \
            if(d->m_##token != gra) {                   \
                d->m_##token.setGradientDeg(gra->gradientDeg());\
                d->m_##token.setGradientStart(gra->gradientStart());\
                d->m_##token.setGradientEnd(gra->gradientEnd());\
                d->m_##token.setGradientBackground(gra->gradientBackground());\
                d->m_##token.setPureColor(QColor(0,0,0,0)); \
                d->m_##token.setColorType(DtColorType::Gradient);\
                Q_EMIT token##Changed();                \
            }                                           \
        }                                               \
    }
#define INT_TOKEN_SETTER(token)                         \
    void DtTheme::token##Set(QVariant value)            \
    {                                                   \
        if(d->m_##token != value.value<int>()) {        \
            d->m_##token = value.value<int>();          \
            Q_EMIT token##Changed();                    \
        }                                               \
    }

#define SHADOW_TOKEN_SETTER(token)                      \
void DtTheme::token##Set(QVariant value)                \
{                                                       \
    ShadowData* data = value.value<UkuiQuick::ShadowData*>();\
    if(d->m_##token != *data) {                          \
        d->m_##token.setXOffset(data->xOffset());       \
        d->m_##token.setYOffset(data->yOffset());       \
        d->m_##token.setBlurRadius(data->blurRadius()); \
        d->m_##token.setSpread(data->spread());         \
        d->m_##token.setIsMultiShadow(data->isMultiShadow());\
        d->m_##token.setShadowColor(data->shadowColor());\
        d->m_##token.setChildShadow(data->childShadow());\
        Q_EMIT token##Changed();                        \
    }                                                   \
}

#define UKUI_STYLE_SETTING          "org.ukui.style"
#define UKUI_STYLE_NAME_KEY         "widgetThemeName"
#define UKUI_STYLE_COLOR_KEY        "styleName"
#define UKUI_STYLE_FONT_SIZE_KEY    "systemFontSize"
#define UKUI_STYLE_FONT_KEY         "systemFont"
#define UKUI_STYLE_ICON_THEME_NAME_KEY "iconThemeName"
#define UKUI_STYLE_THEME_COLOR      "themeColor"
#define UKUI_STYLE_WINDOW_RADIUS    "windowRadius"
#define CONTROL_CENTER_SETTING          "org.ukui.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"

class ThemeGlobalConfigPrivate;
class ThemeGlobalConfigPrivate
{
public:
    QString m_themeName = "default";
    QString m_styleColor = "Light";
    qreal m_transparency = 1.0;
    qreal m_fontSize = 11.0;
    QString m_fontFamily = "Noto Sans CJK SC";
    Qt::LayoutDirection layoutDirection = QGuiApplication::layoutDirection();
    QString m_iconThemeName = "ukui-icon-theme-default";
    QString m_themeColor = "KBrand1";
    QString m_windowRadius = "bigRadius";
};
ThemeGlobalConfig* ThemeGlobalConfig::instance()
{
    static ThemeGlobalConfig* instance = nullptr;
    if(!instance) {
        instance = new ThemeGlobalConfig();
    }
    return instance;
}

ThemeGlobalConfig::~ThemeGlobalConfig()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

qreal ThemeGlobalConfig::transparency() const
{
    return d->m_transparency;
}

QString ThemeGlobalConfig::themeName() const
{
    return d->m_themeName;
}

QString ThemeGlobalConfig::styleColor() const
{
    return d->m_styleColor;
}

QString ThemeGlobalConfig::themeColor() const
{
    return d->m_themeColor;
}

QString ThemeGlobalConfig::windowRadius() const
{
    return d->m_windowRadius;
}

QString ThemeGlobalConfig::iconThemeName() const
{
    return d->m_iconThemeName;
}

Qt::LayoutDirection ThemeGlobalConfig::layoutDirection() const
{
    return d->layoutDirection;
}

bool ThemeGlobalConfig::isDarkTheme() const
{
    return d->m_styleColor == "Dark";
}

qreal ThemeGlobalConfig::fontSize() const
{
    return d->m_fontSize;
}

QString ThemeGlobalConfig::fontFamily() const
{
    return d->m_fontFamily;
}

ThemeGlobalConfig::ThemeGlobalConfig() : d(new ThemeGlobalConfigPrivate())
{
    QByteArray id(CONTROL_CENTER_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), this);

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            d->m_transparency = settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
        }
        connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY) {
                d->m_transparency = settings->get(key).toReal();
                Q_EMIT transparencyChanged();
            }
        });
    }

     id = UKUI_STYLE_SETTING;
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), this);

        QStringList keys = settings->keys();
        if (keys.contains(UKUI_STYLE_NAME_KEY)) {
            d->m_themeName = settings->get(UKUI_STYLE_NAME_KEY).toString();
        }
		if (keys.contains(UKUI_STYLE_COLOR_KEY)) {
            auto color = settings->get(UKUI_STYLE_COLOR_KEY).toString();
            if(color == "ukui-light" || color == "ukui-white") {
               d->m_styleColor = "Light";
            } else if (color == "ukui-dark" || color == "ukui-black") {
               d->m_styleColor = "Dark";
            }
        }
		if (keys.contains(UKUI_STYLE_FONT_SIZE_KEY)) {
            d->m_fontSize = settings->get(UKUI_STYLE_FONT_SIZE_KEY).toReal();
        }
		if (keys.contains(UKUI_STYLE_FONT_KEY)) {
            d->m_fontFamily = settings->get(UKUI_STYLE_FONT_KEY).toString();
        }
        if (keys.contains(UKUI_STYLE_THEME_COLOR)) {
            d->m_themeColor = settings->get(UKUI_STYLE_THEME_COLOR).toString();
        }
        if (keys.contains(UKUI_STYLE_WINDOW_RADIUS)) {
            d->m_windowRadius = settings->get(UKUI_STYLE_WINDOW_RADIUS).toString();
        }
        if (keys.contains(UKUI_STYLE_ICON_THEME_NAME_KEY)) {
            d->m_iconThemeName = settings->get(UKUI_STYLE_ICON_THEME_NAME_KEY).toString();
        }
        QObject::connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == UKUI_STYLE_NAME_KEY) {
                if(d->m_themeName != settings->get(key).toString()) {
                    d->m_themeName = settings->get(key).toString();
                    Q_EMIT themeNameChanged();
                }
            } else if (key == UKUI_STYLE_COLOR_KEY) {
                auto color = settings->get(UKUI_STYLE_COLOR_KEY).toString();
                if(color == "ukui-light" || color == "ukui-white") {
                   d->m_styleColor = "Light";
                } else if (color == "ukui-dark" || color == "ukui-black") {
                   d->m_styleColor = "Dark";
                }
                Q_EMIT styleColorChanged();
            } else if (key == UKUI_STYLE_FONT_SIZE_KEY) {
                d->m_fontSize = settings->get(key).toReal();
                Q_EMIT fontSizeChanged();
            } else if (key == UKUI_STYLE_FONT_KEY) {
                d->m_fontFamily = settings->get(key).toString();
                Q_EMIT fontFamilyChanged();
            } else if (key == UKUI_STYLE_ICON_THEME_NAME_KEY) {
                d->m_iconThemeName = settings->get(key).toString();
                Q_EMIT iconThemeNameChanged();
            } else if (key == UKUI_STYLE_THEME_COLOR) {
                d->m_themeColor = settings->get(UKUI_STYLE_THEME_COLOR).toString();
                Q_EMIT themeColorChanged();
            } else if (key == UKUI_STYLE_WINDOW_RADIUS) {
                d->m_windowRadius = settings->get(UKUI_STYLE_WINDOW_RADIUS).toString();
                Q_EMIT windowRadiusChanged();
            }
        });
    }

    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, [this] (const Qt::LayoutDirection &layoutDirection) {
        d->layoutDirection = layoutDirection;
        Q_EMIT layoutDirectionChanged();
    });
}
class DtThemePrivate {
public:
    DtThemePrivate(DtTheme *q) : q(q) {};
    DESIGN_TOKENS
    QQmlEngine* m_engine = nullptr;
    DtThemeDefinition *m_themeDefinition = nullptr;
    DtTheme *q = nullptr;
    QMap<QString, std::function<void(QVariant)>> m_valueSet;
};

/*!
    \qmltype GlobalTheme
    \inqmlmodule org.ukui.quick.platform
    \ingroup ukui-quick-platform
    \brief DT主题信息模块，提供DT设计语言的全局 token。

    GlobalTheme 是平台模块提供的主题单例。它将主题色板、描边、阴影、
    间距、圆角和字体信息统一暴露给 QML，便于组件按同一套 token 进行渲染。
    主题切换、强调色变更、透明度变更后，对应属性会自动更新。

    \section1 使用方式
    GlobalTheme 作为单例使用，不需要手动创建实例。

    \qml
    import QtQuick
    import org.ukui.quick.platform 1.0

    DtThemeBackground {
        width: 260
        height: 96
        radius: GlobalTheme.kRadiusWindow
        backgroundColor: GlobalTheme.windowActive
        border.width: GlobalTheme.normalLine
        borderColor: GlobalTheme.kLineComponentNormal
    }
    \endqml

    \section1 Token 组织
    \list
    \li 调色板语义色：如 \c textActive、\c windowActive、\c buttonActive
    \li 状态色：如 \c kBrandNormal、\c kSuccessNormal、\c kWarningNormal、\c kErrorNormal
    \li 组件容器与描边：如 \c kComponentNormal、\c kLineComponentNormal、\c kLineBrandNormal
    \li 尺寸参数：如 \c normalLine、\c kRadiusWindow、\c kMarginNormal、\c kPaddingNormalLeft
    \li 阴影参数：如 \c kShadowPrimaryActive、\c kShadowMenu
    \endlist

    颜色 token 类型为 \l GradientColor，阴影 token 类型为 \l ShadowData。
    若颜色 token 为纯色，可通过 \c pureColor 直接取值；若为渐变，可结合
    \l GradientColor 的渐变属性进行渲染。

    \sa GradientColor
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowActive
    窗口背景的激活态颜色 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::textActive
    常规文本的激活态颜色 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightActive
    强调色的激活态 token，会随系统主题强调色联动更新。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrandNormal
    主色 token，通常用于主按钮、选中态和强调元素。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineComponentNormal
    组件默认描边颜色 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentNormal
    组件默认容器背景颜色 token。
*/

/*!
    \qmlproperty int GlobalTheme::normalLine
    常规描边宽度，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::focusLine
    焦点描边宽度，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kRadiusWindow
    窗口圆角半径，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kRadiusMenu
    菜单圆角半径，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kMarginNormal
    常规布局外边距 token，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingNormalLeft
    常规布局左内边距 token，单位为像素。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowPrimaryActive
    主阴影（激活态）token，包含偏移、模糊半径和阴影色等参数。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowMenu
    菜单阴影 token。
*/

/*!
    \qmlproperty real GlobalTheme::transparency
    全局透明度系数，取值范围通常为 \c [0, 1]。
*/

/*!
    \qmlproperty real GlobalTheme::fontSize
    系统字体基准字号。
*/

/*!
    \qmlproperty string GlobalTheme::fontFamily
    系统字体族名称。
*/

/*!
    \qmlproperty enumeration GlobalTheme::layoutDirection
    当前界面布局方向。可用于在 QML 中处理 LTR/RTL 布局适配。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::lightActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::lightInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::lightDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::darkActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::darkInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::darkDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::baseActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::baseInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::baseDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midLightActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midLightInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midLightDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::midDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::textInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::textDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::brightTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::brightTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::brightTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::buttonTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::windowDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::shadowActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::shadowInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::shadowDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkVisitedDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::alternateBaseActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::alternateBaseInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::alternateBaseDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::noRoleActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::noRoleInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::noRoleDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipBaseActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipBaseInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipBaseDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::toolTipTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::placeholderTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::placeholderTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::placeholderTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kWhite
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBlack
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray0
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray2
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray3
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray4
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray5
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray6
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray7
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray8
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray9
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray10
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray11
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray12
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray13
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray14
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray15
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray16
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGray17
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha0
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha2
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha3
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha4
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha5
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha6
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha7
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha8
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha9
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha10
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha11
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha12
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGrayAlpha13
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontStrong
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontPrimary
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontPrimaryDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontSecondary
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontSecondaryDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhite
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhiteDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhiteSecondary
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhiteSecondaryDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSuccessNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kWarningNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kErrorNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand2
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand3
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand4
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand5
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand6
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand7
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainGeneralNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainSecondaryNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainSecondaryAlphaNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentAlphaNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentAlphaDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWindowActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineComponentHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineComponentClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineComponentDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineBrandNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineBrandHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineBrandClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineBrandDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kModalMask
    主题属性 token。
*/

/*!
    \qmlproperty int GlobalTheme::kRadiusMin
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kRadiusNormal
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kRadiusMax
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kMarginMin
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kMarginBig
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kMarginWindow
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kMarginComponent
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingMinLeft
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingMinRight
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingMinTop
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingMinBottom
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingNormalRight
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingNormalTop
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingNormalBottom
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPadding8Left
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPadding8Right
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPadding8Top
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPadding8Bottom
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingWindowLeft
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingWindowRight
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingWindowTop
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty int GlobalTheme::kPaddingWindowBottom
    主题尺寸参数，单位为像素。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkVisitedActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::linkVisitedInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrandHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrandClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrandFocus
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSuccessHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSuccessClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kWarningHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kWarningClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kErrorHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kErrorClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentAlphaHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentAlphaClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kErrorDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainGeneralAlphaNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSuccessDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWindowInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainAlphaClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kWarningDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kDivider
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainAlphaHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightedTextActive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightedTextInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::highlightedTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrandDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand8
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand9
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kBrand10
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSelectboxNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSelectboxHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSelectboxClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSelectboxSelected
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSelectboxDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhitePlaceholderTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainGeneralInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontPlaceholderTextDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontWhitePlaceholderText
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineInputDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineDock
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineMenu
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineTable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineInputNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineInputHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineInputClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kOrange1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kGreen1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kRed1
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kDividerWhite
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSuccessNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSuccessHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSuccessClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineSuccessDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineErrorNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineErrorHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineErrorClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWarningNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWarningHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWarningClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineWarningDisable
    主题属性 token。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowPrimaryInactive
    主题属性 token。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowSecondaryActive
    主题属性 token。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowSecondaryInactive
    主题属性 token。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowMin
    主题属性 token。
*/

/*!
    \qmlproperty ShadowData GlobalTheme::kShadowComponent
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedAlphaNormal
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedAlphaHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedAlphaClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentSelectedAlphaDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentInput
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kComponentInputAlpha
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSelectAlphaWhite
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSelectAlphaWhiteHover
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSelectAlphaWhiteClick
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kSelectAlphaWhiteDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kMenu
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kLineErrorDisable
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kContainSecondaryInactive
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kFontPlaceholderText
    主题属性 token。
*/

/*!
    \qmlproperty GradientColor GlobalTheme::kPanel
    主题属性 token。
*/

DtTheme::DtTheme(QQmlEngine* engine, QObject *parent) : QObject(parent), d(new DtThemePrivate(this))
{
    d->m_engine = engine;
    //加载默认主题文件
    addFuncPointer();
    //根据DT主题名称和颜色读取对应的qml文件
    setTheme();
    onWindowRadiusChanged();
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::transparencyChanged, this, &DtTheme::transparencyChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeNameChanged, this, &DtTheme::setTheme);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeColorChanged, this, &DtTheme::onThemeColorChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::styleColorChanged, this, &DtTheme::setTheme);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::windowRadiusChanged, this, &DtTheme::onWindowRadiusChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::fontSizeChanged, this, &DtTheme::fontSizeChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::fontFamilyChanged, this, &DtTheme::fontFamilyChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::iconThemeNameChanged, this, &DtTheme::iconThemeNameChanged);
    connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::layoutDirectionChanged, this, &DtTheme::layoutDirectionChanged);
}

DtTheme* DtTheme::self(QQmlEngine* engine)
{
    if(!engine) {
        return nullptr;
    }
    const QHash<QQmlEngine*, DtTheme*>::iterator it = g_dtThemes.find(engine);
    if(it == g_dtThemes.end()) {
        return g_dtThemes.insert(engine, new DtTheme(engine, engine)).value();
    }
    return it.value();
}

DtTheme::~DtTheme()
{
    g_dtThemes.remove(d->m_engine);
    if(d) {
        delete d;
        d = nullptr;
    }
}

void DtTheme::proportySet()
{
    if(!d->m_themeDefinition) {
        return;
    }
    const QMetaObject* metaObject = d->m_themeDefinition->metaObject();
    QStringList properties;
    for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
        properties << QString::fromLatin1(metaObject->property(i).name());
    for (auto token : properties) {
        if(QVariant value = d->m_themeDefinition->property(token.toLocal8Bit().data()); value.isValid()) {
            if(d->m_valueSet.contains(token)) {
                std::function<void(QVariant)> valueSet = d->m_valueSet[token];
                valueSet(value);
            }
        }
    }
}

void DtTheme::setTheme()
{
    //根据DT主题名称和颜色读取对应的qml文件
    QString path= QString("/usr/share/config/themeconfig/token/k%1%2.qml").arg(ThemeGlobalConfig::instance()->themeName()).arg(ThemeGlobalConfig::instance()->styleColor());
    if(!QFile::exists(path)) {
        qWarning() << "No Theme file found, using default theme";
        path = QStringLiteral("/usr/share/ukui/ukui-quick-platform/DtThemeDefault.qml");
    }
    if(QFile::exists(path)) {
        QQmlComponent component(d->m_engine);
        component.loadUrl(QUrl(path));
        if (!component.isError()) {
            auto result = component.create();
            if (auto themeDefinition = qobject_cast<DtThemeDefinition *>(result)) {
                d->m_themeDefinition = themeDefinition;
                //TODO token详细的分类,分组更新
                proportySet();
                if (d->m_themeDefinition) {
                    delete d->m_themeDefinition;
                    d->m_themeDefinition = nullptr;
                }
                onThemeColorChanged();
                return;
            }
        }
        const auto errors = component.errors();
        for (const auto&error: errors) {
            qWarning() << error.toString();
        }
        qWarning() << "Invalid Theme file, using default theme.";
    }
 }

void DtTheme::addFuncPointer()
{
    d->m_valueSet.clear();
    d->m_valueSet.insert("windowTextActive", std::bind(&DtTheme::windowTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("windowTextInactive", std::bind(&DtTheme::windowTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("windowTextDisable", std::bind(&DtTheme::windowTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonActive", std::bind(&DtTheme::buttonActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonInactive", std::bind(&DtTheme::buttonInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonDisable", std::bind(&DtTheme::buttonDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("lightActive", std::bind(&DtTheme::lightActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("lightInactive", std::bind(&DtTheme::lightInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("lightDisable", std::bind(&DtTheme::lightDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midLightActive", std::bind(&DtTheme::midLightActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midLightInactive", std::bind(&DtTheme::midLightInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midLightDisable", std::bind(&DtTheme::midLightDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("darkActive", std::bind(&DtTheme::darkActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("darkInactive", std::bind(&DtTheme::darkInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("darkDisable", std::bind(&DtTheme::darkDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midActive", std::bind(&DtTheme::midActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midInactive", std::bind(&DtTheme::midInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("midDisable", std::bind(&DtTheme::midDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("textActive", std::bind(&DtTheme::textActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("textInactive", std::bind(&DtTheme::textInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("textDisable", std::bind(&DtTheme::textDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("brightTextActive", std::bind(&DtTheme::brightTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("brightTextInactive", std::bind(&DtTheme::brightTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("brightTextDisable", std::bind(&DtTheme::brightTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonTextActive", std::bind(&DtTheme::buttonTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonTextInactive", std::bind(&DtTheme::buttonTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("buttonTextDisable", std::bind(&DtTheme::buttonTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("baseActive", std::bind(&DtTheme::baseActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("baseInactive", std::bind(&DtTheme::baseInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("baseDisable", std::bind(&DtTheme::baseDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("windowActive", std::bind(&DtTheme::windowActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("windowInactive", std::bind(&DtTheme::windowInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("windowDisable", std::bind(&DtTheme::windowDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("shadowActive", std::bind(&DtTheme::shadowActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("shadowInactive", std::bind(&DtTheme::shadowInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("shadowDisable", std::bind(&DtTheme::shadowDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("highlightedTextActive", std::bind(&DtTheme::highlightedTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("highlightedTextInactive", std::bind(&DtTheme::highlightedTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("highlightedTextDisable", std::bind(&DtTheme::highlightedTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkActive", std::bind(&DtTheme::linkActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkInactive", std::bind(&DtTheme::linkInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkDisable", std::bind(&DtTheme::linkDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkVisitedActive", std::bind(&DtTheme::linkVisitedActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkVisitedInactive", std::bind(&DtTheme::linkVisitedInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("linkVisitedDisable", std::bind(&DtTheme::linkVisitedDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("alternateBaseActive", std::bind(&DtTheme::alternateBaseActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("alternateBaseInactive", std::bind(&DtTheme::alternateBaseInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("alternateBaseDisable", std::bind(&DtTheme::alternateBaseDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("noRoleActive", std::bind(&DtTheme::noRoleActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("noRoleInactive", std::bind(&DtTheme::noRoleInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("noRoleDisable", std::bind(&DtTheme::noRoleDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipBaseActive", std::bind(&DtTheme::toolTipBaseActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipBaseInactive", std::bind(&DtTheme::toolTipBaseInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipBaseDisable", std::bind(&DtTheme::toolTipBaseDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipTextActive", std::bind(&DtTheme::toolTipTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipTextInactive", std::bind(&DtTheme::toolTipTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("toolTipTextDisable", std::bind(&DtTheme::toolTipTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("placeholderTextActive", std::bind(&DtTheme::placeholderTextActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("placeholderTextInactive", std::bind(&DtTheme::placeholderTextInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("placeholderTextDisable", std::bind(&DtTheme::placeholderTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kWhite", std::bind(&DtTheme::kWhiteSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kBlack", std::bind(&DtTheme::kBlackSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray0", std::bind(&DtTheme::kGray0Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray1", std::bind(&DtTheme::kGray1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray2", std::bind(&DtTheme::kGray2Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray3", std::bind(&DtTheme::kGray3Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray4", std::bind(&DtTheme::kGray4Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray5", std::bind(&DtTheme::kGray5Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray6", std::bind(&DtTheme::kGray6Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray7", std::bind(&DtTheme::kGray7Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray8", std::bind(&DtTheme::kGray8Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray9", std::bind(&DtTheme::kGray9Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray10", std::bind(&DtTheme::kGray10Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray11", std::bind(&DtTheme::kGray11Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray12", std::bind(&DtTheme::kGray12Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray13", std::bind(&DtTheme::kGray13Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray14", std::bind(&DtTheme::kGray14Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray15", std::bind(&DtTheme::kGray15Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray16", std::bind(&DtTheme::kGray16Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGray17", std::bind(&DtTheme::kGray17Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha0", std::bind(&DtTheme::kGrayAlpha0Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha1", std::bind(&DtTheme::kGrayAlpha1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha2", std::bind(&DtTheme::kGrayAlpha2Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha3", std::bind(&DtTheme::kGrayAlpha3Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha4", std::bind(&DtTheme::kGrayAlpha4Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha5", std::bind(&DtTheme::kGrayAlpha5Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha6", std::bind(&DtTheme::kGrayAlpha6Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha7", std::bind(&DtTheme::kGrayAlpha7Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha8", std::bind(&DtTheme::kGrayAlpha8Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha9", std::bind(&DtTheme::kGrayAlpha9Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha10", std::bind(&DtTheme::kGrayAlpha10Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha11", std::bind(&DtTheme::kGrayAlpha11Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha12", std::bind(&DtTheme::kGrayAlpha12Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGrayAlpha13", std::bind(&DtTheme::kGrayAlpha13Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontStrong", std::bind(&DtTheme::kFontStrongSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontPrimary", std::bind(&DtTheme::kFontPrimarySet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontPrimaryDisable", std::bind(&DtTheme::kFontPrimaryDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontSecondary", std::bind(&DtTheme::kFontSecondarySet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontSecondaryDisable", std::bind(&DtTheme::kFontSecondaryDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhite", std::bind(&DtTheme::kFontWhiteSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhiteDisable", std::bind(&DtTheme::kFontWhiteDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhiteSecondary", std::bind(&DtTheme::kFontWhiteSecondarySet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhiteSecondaryDisable", std::bind(&DtTheme::kFontWhiteSecondaryDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrandFocus", std::bind(&DtTheme::kBrandFocusSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSuccessNormal", std::bind(&DtTheme::kSuccessNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSuccessHover", std::bind(&DtTheme::kSuccessHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSuccessClick", std::bind(&DtTheme::kSuccessClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kWarningNormal", std::bind(&DtTheme::kWarningNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kWarningHover", std::bind(&DtTheme::kWarningHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kWarningClick", std::bind(&DtTheme::kWarningClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kErrorNormal", std::bind(&DtTheme::kErrorNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kErrorHover", std::bind(&DtTheme::kErrorHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kErrorClick", std::bind(&DtTheme::kErrorClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand1", std::bind(&DtTheme::kBrand1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand2", std::bind(&DtTheme::kBrand2Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand3", std::bind(&DtTheme::kBrand3Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand4", std::bind(&DtTheme::kBrand4Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand5", std::bind(&DtTheme::kBrand5Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand6", std::bind(&DtTheme::kBrand6Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand7", std::bind(&DtTheme::kBrand7Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainHover", std::bind(&DtTheme::kContainHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainClick", std::bind(&DtTheme::kContainClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainGeneralNormal", std::bind(&DtTheme::kContainGeneralNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainSecondaryNormal", std::bind(&DtTheme::kContainSecondaryNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainSecondaryAlphaNormal", std::bind(&DtTheme::kContainSecondaryAlphaNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentNormal", std::bind(&DtTheme::kComponentNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentHover", std::bind(&DtTheme::kComponentHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentClick", std::bind(&DtTheme::kComponentClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentDisable", std::bind(&DtTheme::kComponentDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentAlphaNormal", std::bind(&DtTheme::kComponentAlphaNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentAlphaHover", std::bind(&DtTheme::kComponentAlphaHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentAlphaClick", std::bind(&DtTheme::kComponentAlphaClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentAlphaDisable", std::bind(&DtTheme::kComponentAlphaDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWindowActive", std::bind(&DtTheme::kLineWindowActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineComponentNormal", std::bind(&DtTheme::kLineComponentNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineComponentHover", std::bind(&DtTheme::kLineComponentHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineComponentClick", std::bind(&DtTheme::kLineComponentClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineComponentDisable", std::bind(&DtTheme::kLineComponentDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kModalMask", std::bind(&DtTheme::kModalMaskSet, this, std::placeholders::_1));

    d->m_valueSet.insert("normalLine", std::bind(&DtTheme::normalLineSet, this, std::placeholders::_1));
    d->m_valueSet.insert("focusLine", std::bind(&DtTheme::focusLineSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kRadiusMin", std::bind(&DtTheme::kRadiusMinSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kRadiusNormal", std::bind(&DtTheme::kRadiusNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kRadiusMax", std::bind(&DtTheme::kRadiusMaxSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMarginMin", std::bind(&DtTheme::kMarginMinSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMarginNormal", std::bind(&DtTheme::kMarginNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMarginBig", std::bind(&DtTheme::kMarginBigSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMarginWindow", std::bind(&DtTheme::kMarginWindowSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMarginComponent", std::bind(&DtTheme::kMarginComponentSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingMinLeft", std::bind(&DtTheme::kPaddingMinLeftSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingMinTop", std::bind(&DtTheme::kPaddingMinTopSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingMinRight", std::bind(&DtTheme::kPaddingMinRightSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingMinBottom", std::bind(&DtTheme::kPaddingMinBottomSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingNormalLeft", std::bind(&DtTheme::kPaddingNormalLeftSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingNormalTop", std::bind(&DtTheme::kPaddingNormalTopSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingNormalRight", std::bind(&DtTheme::kPaddingNormalRightSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingNormalBottom", std::bind(&DtTheme::kPaddingNormalBottomSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingWindowLeft", std::bind(&DtTheme::kPaddingWindowLeftSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingWindowTop", std::bind(&DtTheme::kPaddingWindowTopSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingWindowRight", std::bind(&DtTheme::kPaddingWindowRightSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPaddingWindowBottom", std::bind(&DtTheme::kPaddingWindowBottomSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kErrorDisable", std::bind(&DtTheme::kErrorDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kWarningDisable", std::bind(&DtTheme::kWarningDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineNormal", std::bind(&DtTheme::kLineNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineNormal", std::bind(&DtTheme::kLineNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainGeneralAlphaNormal", std::bind(&DtTheme::kContainGeneralAlphaNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSuccessDisable", std::bind(&DtTheme::kSuccessDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWindowInactive", std::bind(&DtTheme::kLineWindowInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainAlphaClick", std::bind(&DtTheme::kContainAlphaClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainAlphaHover", std::bind(&DtTheme::kContainAlphaHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineDisable", std::bind(&DtTheme::kLineDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kDivider", std::bind(&DtTheme::kDividerSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand8", std::bind(&DtTheme::kBrand8Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand9", std::bind(&DtTheme::kBrand9Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSelectboxNormal", std::bind(&DtTheme::kLineSelectboxNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSelectboxHover", std::bind(&DtTheme::kLineSelectboxHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSelectboxClick", std::bind(&DtTheme::kLineSelectboxClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSelectboxSelected", std::bind(&DtTheme::kLineSelectboxSelectedSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSelectboxDisable", std::bind(&DtTheme::kLineSelectboxDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhitePlaceholderTextDisable", std::bind(&DtTheme::kFontWhitePlaceholderTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainGeneralInactive", std::bind(&DtTheme::kContainGeneralInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontPlaceholderTextDisable", std::bind(&DtTheme::kFontPlaceholderTextDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontWhitePlaceholderText", std::bind(&DtTheme::kFontWhitePlaceholderTextSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineInputDisable", std::bind(&DtTheme::kLineInputDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineDock", std::bind(&DtTheme::kLineDockSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineMenu", std::bind(&DtTheme::kLineMenuSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineTable", std::bind(&DtTheme::kLineTableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineInputNormal", std::bind(&DtTheme::kLineInputNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineInputHover", std::bind(&DtTheme::kLineInputHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineInputClick", std::bind(&DtTheme::kLineInputClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kOrange1", std::bind(&DtTheme::kOrange1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kGreen1", std::bind(&DtTheme::kGreen1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kRed1", std::bind(&DtTheme::kRed1Set, this, std::placeholders::_1));
    d->m_valueSet.insert("kDividerWhite", std::bind(&DtTheme::kDividerWhiteSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSuccessNormal", std::bind(&DtTheme::kLineSuccessNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSuccessHover", std::bind(&DtTheme::kLineSuccessHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSuccessClick", std::bind(&DtTheme::kLineSuccessClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineSuccessDisable", std::bind(&DtTheme::kLineSuccessDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineErrorNormal", std::bind(&DtTheme::kLineErrorNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineErrorHover", std::bind(&DtTheme::kLineErrorHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineErrorClick", std::bind(&DtTheme::kLineErrorClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWarningNormal", std::bind(&DtTheme::kLineWarningNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWarningHover", std::bind(&DtTheme::kLineWarningHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWarningClick", std::bind(&DtTheme::kLineWarningClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineWarningDisable", std::bind(&DtTheme::kLineWarningDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowPrimaryActive", std::bind(&DtTheme::kShadowPrimaryActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowPrimaryInactive", std::bind(&DtTheme::kShadowPrimaryInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowSecondaryActive", std::bind(&DtTheme::kShadowSecondaryActiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowSecondaryInactive", std::bind(&DtTheme::kShadowSecondaryInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowMin", std::bind(&DtTheme::kShadowMinSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowMenu", std::bind(&DtTheme::kShadowMenuSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kShadowComponent", std::bind(&DtTheme::kShadowComponentSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedNormal", std::bind(&DtTheme::kComponentSelectedNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedHover", std::bind(&DtTheme::kComponentSelectedHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedClick", std::bind(&DtTheme::kComponentSelectedClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedDisable", std::bind(&DtTheme::kComponentSelectedDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedAlphaNormal", std::bind(&DtTheme::kComponentSelectedAlphaNormalSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedAlphaHover", std::bind(&DtTheme::kComponentSelectedAlphaHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedAlphaClick", std::bind(&DtTheme::kComponentSelectedAlphaClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentSelectedAlphaDisable", std::bind(&DtTheme::kComponentSelectedAlphaDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentInput", std::bind(&DtTheme::kComponentInputSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kComponentInputAlpha", std::bind(&DtTheme::kComponentInputAlphaSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSelectAlphaWhiteDisable", std::bind(&DtTheme::kSelectAlphaWhiteDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSelectAlphaWhiteHover", std::bind(&DtTheme::kSelectAlphaWhiteHoverSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSelectAlphaWhiteClick", std::bind(&DtTheme::kSelectAlphaWhiteClickSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kMenu", std::bind(&DtTheme::kMenuSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kLineErrorDisable", std::bind(&DtTheme::kLineErrorDisableSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kContainSecondaryInactive", std::bind(&DtTheme::kContainSecondaryInactiveSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kSelectAlphaWhite", std::bind(&DtTheme::kSelectAlphaWhiteSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kFontPlaceholderText", std::bind(&DtTheme::kFontPlaceholderTextSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kPanel", std::bind(&DtTheme::kPanelSet, this, std::placeholders::_1));
    d->m_valueSet.insert("kBrand10", std::bind(&DtTheme::kBrand10Set, this, std::placeholders::_1));

}

qreal DtTheme::transparency() const
{
    return ThemeGlobalConfig::instance()->transparency();
}

qreal DtTheme::fontSize() const
{
    return ThemeGlobalConfig::instance()->fontSize();
}

QString DtTheme::fontFamily() const
{
    return ThemeGlobalConfig::instance()->fontFamily();
}

Qt::LayoutDirection DtTheme::layoutDirection() const
{
    return ThemeGlobalConfig::instance()->layoutDirection();
}

//强调色
void DtTheme::onThemeColorChanged()
{
    QString themeColor = ThemeGlobalConfig::instance()->themeColor();
    //兼容现有强调色切换逻辑
    if(themeColor == "daybreakBlue" || themeColor.contains("kBrand1")) {
        updateHighLightColor(d->m_kBrand1.pureColor());
    }else if (themeColor == "jamPurple" || themeColor.contains("kBrand2")) {
        updateHighLightColor(d->m_kBrand2.pureColor());
    } else if (themeColor == "magenta" || themeColor.contains("kBrand3")) {
        updateHighLightColor(d->m_kBrand3.pureColor());
    } else if (themeColor == "dustGold" || themeColor.contains("kBrand4")) {
        updateHighLightColor(d->m_kBrand4.pureColor());
    } else if (themeColor == "polarGreen" || themeColor.contains("kBrand5")) {
        updateHighLightColor(d->m_kBrand5.pureColor());
    } else if (themeColor == "sunRed" || themeColor.contains("kBrand6")) {
        updateHighLightColor(d->m_kBrand6.pureColor());
    } else if (themeColor == "sunsetOrange" || themeColor.contains("kBrand7")) {
        updateHighLightColor(d->m_kBrand7.pureColor());
    } else if (themeColor.contains("kBrand8")) {
        updateHighLightColor(d->m_kBrand8.pureColor());
    } else if (themeColor.contains("kBrand9")) {
        updateHighLightColor(d->m_kBrand9.pureColor());
    } else if (themeColor.contains("kBrand10")) {
        updateHighLightColor(d->m_kBrand10.pureColor());
    } else if(themeColor.startsWith("#")) {
        //兼容gsettings键值为#FFFFFF格式的颜色
        if(QColor::isValidColor(themeColor)) {
            updateHighLightColor(QColor(themeColor));
        }
    } else {
        //兼容rgba格式颜色
        QRegularExpression rgbaReg(R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d),\s*(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d),\s*(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d),\s*(0(\.\d+)?|1(\.0+)?|0|1))");
        QRegularExpression rgbReg(R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d),\s*(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d),\s*(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d))");
        QStringList rgbaList = rgbaReg.match(themeColor).hasMatch() ? rgbaReg.match(themeColor).capturedTexts()
                                                                    : rgbReg.match(themeColor).capturedTexts();
        if(rgbaList.size() >= 4) {
            QColor rgba;
            rgba.setRed(rgbaList.at(1).toInt());
            rgba.setGreen(rgbaList.at(2).toInt());
            rgba.setBlue(rgbaList.at(3).toInt());
            if(rgbaList.size() >= 5) {
                auto alpha = rgbaList.at(4).toDouble();
                rgba.setAlphaF(alpha > 1 ? alpha/255 : alpha);
            }
            updateHighLightColor(rgba);
        }
    }
}

void DtTheme::updateHighLightColor(QColor highlightColor)
{
    d->m_highlightActive.setPureColor(highlightColor);
    d->m_highlightInactive.setPureColor(highlightColor);
    d->m_kBrandNormal.setPureColor(highlightColor);
    d->m_linkActive.setPureColor(highlightColor);
    d->m_linkInactive.setPureColor(highlightColor);
    d->m_linkVisitedActive.setPureColor(highlightColor);
    d->m_linkVisitedInactive.setPureColor(highlightColor);

    //强调色禁用为当前高亮色*0.44透明度
    QColor color = highlightColor;
    color.setAlphaF(0.45);
    d->m_kBrandDisable.setPureColor(color);
    d->m_highlightDisable.setPureColor(color);
    d->m_linkDisable.setPureColor(color);
    d->m_linkVisitedDisable.setPureColor(color);

    //强调色悬浮需要在强调色基础上叠加一个0.1透明度黑色背景色（0，0，0，0.1）
    if(d->m_kBrandHover.colorType() == DtColorType::Gradient) {
        d->m_kBrandHover.setGradientBackground(highlightColor);
    } else {
        d->m_kBrandHover.setPureColor(d->m_kBrandHover.mixBackGroundColor(QColor(0,0,0,0.1*255),highlightColor, 1));
    }
    //强调色点击需要在强调色基础上叠加一个0.2透明度黑色背景色（0，0，0，0.2）
    if(d->m_kBrandClick.colorType() == DtColorType::Gradient) {
        d->m_kBrandClick.setGradientBackground(highlightColor);
    } else {
        d->m_kBrandClick.setPureColor(d->m_kBrandClick.mixBackGroundColor(QColor(0,0,0,0.2*255),highlightColor, 1));
    }
    //TODO 强调色描边需要根据主题不同，使用不同的公式计算悬停，点击，禁用
    //强调色描边需要在强调色的基础上乘以0.55透明度
    color.setAlphaF(0.55);
    d->m_kLineBrandNormal.setPureColor(color);
    //强调色描边悬浮在5.0主题下需要在强调色基础上叠加一个0.1透明度黑色背景色（0，0，0，0.1）
    if(d->m_kLineBrandHover.colorType() == DtColorType::Gradient) {
        d->m_kLineBrandHover.setGradientBackground(color);
    } else {
        d->m_kLineBrandHover.setPureColor(d->m_kLineBrandHover.mixBackGroundColor(QColor(0,0,0,0.1*255),color, 1));
    }
    //强调色描边点击在5.0主题下需要在强调色基础上叠加一个0.2透明度黑色背景色（0，0，0，0.2）
    if(d->m_kLineBrandClick.colorType() == DtColorType::Gradient) {
        d->m_kLineBrandClick.setGradientBackground(color);
    } else {
        d->m_kLineBrandClick.setPureColor(d->m_kLineBrandClick.mixBackGroundColor(QColor(0,0,0,0.2*255),color, 1));
    }
    //强调色描边不可用需要在强调色基础上再乘以0.45透明度
    color.setAlphaF(0.55*0.45);
    d->m_kLineBrandDisable.setPureColor(color);
}

void DtTheme::onWindowRadiusChanged()
{
    QString windowRadius = ThemeGlobalConfig::instance()->windowRadius();
    if(windowRadius.contains("bigRadius")) {
        kRadiusWindowSet(12);
        kRadiusMenuSet(8);
    } else if(windowRadius.contains("mediumRadius")) {
        kRadiusWindowSet(6);
        kRadiusMenuSet(4);
    } else if(windowRadius.contains("noRadius")) {
        kRadiusWindowSet(0);
        kRadiusMenuSet(0);
    } else {
        QRegularExpression radiusReg(R"(([0-9][0-9]|[0-9]),\s*([0-9][0-9]|[0-9]))");
        if(radiusReg.match(windowRadius).hasMatch()) {
            QStringList radiusList = radiusReg.match(windowRadius).capturedTexts();
            kRadiusWindowSet(radiusList.at(1).toInt());
            kRadiusMenuSet(radiusList.at(2).toInt());
        }
    }
}

bool DtTheme::isDarkTheme()
{
    return ThemeGlobalConfig::instance()->isDarkTheme();
}

COLOR_TOKEN_GETTER(windowTextActive)
COLOR_TOKEN_GETTER(windowTextInactive)
COLOR_TOKEN_GETTER(windowTextDisable)
COLOR_TOKEN_GETTER(buttonActive)
COLOR_TOKEN_GETTER(buttonInactive)
COLOR_TOKEN_GETTER(buttonDisable)
COLOR_TOKEN_GETTER(lightActive)
COLOR_TOKEN_GETTER(lightInactive)
COLOR_TOKEN_GETTER(lightDisable)
COLOR_TOKEN_GETTER(midLightActive)
COLOR_TOKEN_GETTER(midLightInactive)
COLOR_TOKEN_GETTER(midLightDisable)
COLOR_TOKEN_GETTER(darkActive)
COLOR_TOKEN_GETTER(darkInactive)
COLOR_TOKEN_GETTER(darkDisable)
COLOR_TOKEN_GETTER(midActive)
COLOR_TOKEN_GETTER(midInactive)
COLOR_TOKEN_GETTER(midDisable)
COLOR_TOKEN_GETTER(textActive)
COLOR_TOKEN_GETTER(textInactive)
COLOR_TOKEN_GETTER(textDisable)
COLOR_TOKEN_GETTER(brightTextActive)
COLOR_TOKEN_GETTER(brightTextInactive)
COLOR_TOKEN_GETTER(brightTextDisable)
COLOR_TOKEN_GETTER(buttonTextActive)
COLOR_TOKEN_GETTER(buttonTextInactive)
COLOR_TOKEN_GETTER(buttonTextDisable)
COLOR_TOKEN_GETTER(baseActive)
COLOR_TOKEN_GETTER(baseInactive)
COLOR_TOKEN_GETTER(baseDisable)
COLOR_TOKEN_GETTER(windowActive)
COLOR_TOKEN_GETTER(windowInactive)
COLOR_TOKEN_GETTER(windowDisable)
COLOR_TOKEN_GETTER(shadowActive)
COLOR_TOKEN_GETTER(shadowInactive)
COLOR_TOKEN_GETTER(shadowDisable)
COLOR_TOKEN_GETTER(highlightActive)
COLOR_TOKEN_GETTER(highlightInactive)
COLOR_TOKEN_GETTER(highlightDisable)
COLOR_TOKEN_GETTER(highlightedTextActive)
COLOR_TOKEN_GETTER(highlightedTextInactive)
COLOR_TOKEN_GETTER(highlightedTextDisable)
COLOR_TOKEN_GETTER(linkActive)
COLOR_TOKEN_GETTER(linkInactive)
COLOR_TOKEN_GETTER(linkDisable)

COLOR_TOKEN_GETTER(linkVisitedDisable)
COLOR_TOKEN_GETTER(alternateBaseActive)
COLOR_TOKEN_GETTER(alternateBaseInactive)
COLOR_TOKEN_GETTER(alternateBaseDisable)
COLOR_TOKEN_GETTER(noRoleActive)
COLOR_TOKEN_GETTER(noRoleInactive)
COLOR_TOKEN_GETTER(noRoleDisable)
COLOR_TOKEN_GETTER(toolTipBaseActive)
COLOR_TOKEN_GETTER(toolTipBaseInactive)
COLOR_TOKEN_GETTER(toolTipBaseDisable)
COLOR_TOKEN_GETTER(toolTipTextActive)
COLOR_TOKEN_GETTER(toolTipTextInactive)
COLOR_TOKEN_GETTER(toolTipTextDisable)
COLOR_TOKEN_GETTER(placeholderTextActive)
COLOR_TOKEN_GETTER(placeholderTextInactive)
COLOR_TOKEN_GETTER(placeholderTextDisable)

COLOR_TOKEN_GETTER(kWhite)
COLOR_TOKEN_GETTER(kBlack)
COLOR_TOKEN_GETTER(kGray0)
COLOR_TOKEN_GETTER(kGray1)
COLOR_TOKEN_GETTER(kGray2)
COLOR_TOKEN_GETTER(kGray3)
COLOR_TOKEN_GETTER(kGray4)
COLOR_TOKEN_GETTER(kGray5)
COLOR_TOKEN_GETTER(kGray6)
COLOR_TOKEN_GETTER(kGray7)
COLOR_TOKEN_GETTER(kGray8)
COLOR_TOKEN_GETTER(kGray9)
COLOR_TOKEN_GETTER(kGray10)
COLOR_TOKEN_GETTER(kGray11)
COLOR_TOKEN_GETTER(kGray12)
COLOR_TOKEN_GETTER(kGray13)
COLOR_TOKEN_GETTER(kGray14)
COLOR_TOKEN_GETTER(kGray15)
COLOR_TOKEN_GETTER(kGray16)
COLOR_TOKEN_GETTER(kGray17)
COLOR_TOKEN_GETTER(kGrayAlpha0)
COLOR_TOKEN_GETTER(kGrayAlpha1)
COLOR_TOKEN_GETTER(kGrayAlpha2)
COLOR_TOKEN_GETTER(kGrayAlpha3)
COLOR_TOKEN_GETTER(kGrayAlpha4)
COLOR_TOKEN_GETTER(kGrayAlpha5)
COLOR_TOKEN_GETTER(kGrayAlpha6)
COLOR_TOKEN_GETTER(kGrayAlpha7)
COLOR_TOKEN_GETTER(kGrayAlpha8)
COLOR_TOKEN_GETTER(kGrayAlpha9)
COLOR_TOKEN_GETTER(kGrayAlpha10)
COLOR_TOKEN_GETTER(kGrayAlpha11)
COLOR_TOKEN_GETTER(kGrayAlpha12)
COLOR_TOKEN_GETTER(kGrayAlpha13)

COLOR_TOKEN_GETTER(kFontStrong)
COLOR_TOKEN_GETTER(kFontPrimary)
COLOR_TOKEN_GETTER(kFontPrimaryDisable)
COLOR_TOKEN_GETTER(kFontSecondary)
COLOR_TOKEN_GETTER(kFontSecondaryDisable)
COLOR_TOKEN_GETTER(kFontWhite)
COLOR_TOKEN_GETTER(kFontWhiteDisable)
COLOR_TOKEN_GETTER(kFontWhiteSecondary)
COLOR_TOKEN_GETTER(kFontWhiteSecondaryDisable)

COLOR_TOKEN_GETTER(kBrandNormal)
COLOR_TOKEN_GETTER(kSuccessNormal)
COLOR_TOKEN_GETTER(kWarningNormal)
COLOR_TOKEN_GETTER(kErrorNormal)
COLOR_TOKEN_GETTER(kBrand1)
COLOR_TOKEN_GETTER(kBrand2)
COLOR_TOKEN_GETTER(kBrand3)
COLOR_TOKEN_GETTER(kBrand4)
COLOR_TOKEN_GETTER(kBrand5)
COLOR_TOKEN_GETTER(kBrand6)
COLOR_TOKEN_GETTER(kBrand7)

COLOR_TOKEN_GETTER(kContainGeneralNormal)
COLOR_TOKEN_GETTER(kContainSecondaryNormal)
COLOR_TOKEN_GETTER(kContainSecondaryAlphaNormal)
COLOR_TOKEN_GETTER(kComponentNormal)
COLOR_TOKEN_GETTER(kComponentDisable)
COLOR_TOKEN_GETTER(kComponentAlphaNormal)
COLOR_TOKEN_GETTER(kComponentAlphaDisable)
COLOR_TOKEN_GETTER(kLineWindowActive)
COLOR_TOKEN_GETTER(kLineComponentNormal)
COLOR_TOKEN_GETTER(kLineComponentHover)
COLOR_TOKEN_GETTER(kLineComponentClick)
COLOR_TOKEN_GETTER(kLineComponentDisable)
COLOR_TOKEN_GETTER(kLineBrandNormal)
COLOR_TOKEN_GETTER(kLineBrandHover)
COLOR_TOKEN_GETTER(kLineBrandClick)
COLOR_TOKEN_GETTER(kLineBrandDisable)
COLOR_TOKEN_GETTER(kModalMask)
COLOR_TOKEN_GETTER(linkVisitedActive)
COLOR_TOKEN_GETTER(linkVisitedInactive)
COLOR_TOKEN_GETTER(kBrandHover)
COLOR_TOKEN_GETTER(kBrandClick)
COLOR_TOKEN_GETTER(kBrandFocus)
COLOR_TOKEN_GETTER(kSuccessHover)
COLOR_TOKEN_GETTER(kSuccessClick)
COLOR_TOKEN_GETTER(kWarningHover)
COLOR_TOKEN_GETTER(kWarningClick)
COLOR_TOKEN_GETTER(kErrorHover)
COLOR_TOKEN_GETTER(kErrorClick)
COLOR_TOKEN_GETTER(kContainHover)
COLOR_TOKEN_GETTER(kContainClick)
COLOR_TOKEN_GETTER(kComponentHover)
COLOR_TOKEN_GETTER(kComponentClick)
COLOR_TOKEN_GETTER(kComponentAlphaHover)
COLOR_TOKEN_GETTER(kComponentAlphaClick)
COLOR_TOKEN_GETTER(kErrorDisable)
COLOR_TOKEN_GETTER(kWarningDisable)
COLOR_TOKEN_GETTER(kLineNormal)
COLOR_TOKEN_GETTER(kContainGeneralAlphaNormal)
COLOR_TOKEN_GETTER(kSuccessDisable)
COLOR_TOKEN_GETTER(kLineWindowInactive)
COLOR_TOKEN_GETTER(kContainAlphaClick)
COLOR_TOKEN_GETTER(kContainAlphaHover)
COLOR_TOKEN_GETTER(kBrandDisable)
COLOR_TOKEN_GETTER(kLineDisable)
COLOR_TOKEN_GETTER(kDivider)

COLOR_TOKEN_GETTER(kBrand8)
COLOR_TOKEN_GETTER(kBrand9)
COLOR_TOKEN_GETTER(kLineSelectboxNormal)
COLOR_TOKEN_GETTER(kLineSelectboxHover)
COLOR_TOKEN_GETTER(kLineSelectboxClick)
COLOR_TOKEN_GETTER(kLineSelectboxSelected)
COLOR_TOKEN_GETTER(kLineSelectboxDisable)
COLOR_TOKEN_GETTER(kFontWhitePlaceholderTextDisable)
COLOR_TOKEN_GETTER(kContainGeneralInactive)
COLOR_TOKEN_GETTER(kFontPlaceholderTextDisable)
COLOR_TOKEN_GETTER(kFontWhitePlaceholderText)
COLOR_TOKEN_GETTER(kLineInputDisable)
COLOR_TOKEN_GETTER(kLineDock)
COLOR_TOKEN_GETTER(kLineMenu)
COLOR_TOKEN_GETTER(kLineTable)
COLOR_TOKEN_GETTER(kLineInputNormal)
COLOR_TOKEN_GETTER(kLineInputHover)
COLOR_TOKEN_GETTER(kLineInputClick)
COLOR_TOKEN_GETTER(kOrange1)
COLOR_TOKEN_GETTER(kGreen1)
COLOR_TOKEN_GETTER(kRed1)
COLOR_TOKEN_GETTER(kDividerWhite)
COLOR_TOKEN_GETTER(kLineSuccessNormal)
COLOR_TOKEN_GETTER(kLineSuccessHover)
COLOR_TOKEN_GETTER(kLineSuccessClick)
COLOR_TOKEN_GETTER(kLineSuccessDisable)
COLOR_TOKEN_GETTER(kLineErrorNormal)
COLOR_TOKEN_GETTER(kLineErrorHover)
COLOR_TOKEN_GETTER(kLineErrorClick)
COLOR_TOKEN_GETTER(kLineWarningNormal)
COLOR_TOKEN_GETTER(kLineWarningHover)
COLOR_TOKEN_GETTER(kLineWarningClick)
COLOR_TOKEN_GETTER(kLineWarningDisable)
COLOR_TOKEN_GETTER(kComponentSelectedNormal)
COLOR_TOKEN_GETTER(kComponentSelectedHover)
COLOR_TOKEN_GETTER(kComponentSelectedClick)
COLOR_TOKEN_GETTER(kComponentSelectedDisable)
COLOR_TOKEN_GETTER(kComponentSelectedAlphaNormal)
COLOR_TOKEN_GETTER(kComponentSelectedAlphaHover)
COLOR_TOKEN_GETTER(kComponentSelectedAlphaClick)
COLOR_TOKEN_GETTER(kComponentSelectedAlphaDisable)
COLOR_TOKEN_GETTER(kComponentInput)
COLOR_TOKEN_GETTER(kComponentInputAlpha)
COLOR_TOKEN_GETTER(kSelectAlphaWhiteDisable)
COLOR_TOKEN_GETTER(kSelectAlphaWhiteHover)
COLOR_TOKEN_GETTER(kSelectAlphaWhiteClick)
COLOR_TOKEN_GETTER(kMenu)
COLOR_TOKEN_GETTER(kLineErrorDisable)
COLOR_TOKEN_GETTER(kContainSecondaryInactive)
COLOR_TOKEN_GETTER(kSelectAlphaWhite)
COLOR_TOKEN_GETTER(kFontPlaceholderText)
COLOR_TOKEN_GETTER(kPanel)
COLOR_TOKEN_GETTER(kBrand10)

INT_TOKEN_GETTER(normalLine)
INT_TOKEN_GETTER(focusLine)
INT_TOKEN_GETTER(kRadiusMin)
INT_TOKEN_GETTER(kRadiusNormal)
INT_TOKEN_GETTER(kRadiusMax)
INT_TOKEN_GETTER(kRadiusMenu)
INT_TOKEN_GETTER(kRadiusWindow)
INT_TOKEN_GETTER(kMarginMin)
INT_TOKEN_GETTER(kMarginNormal)
INT_TOKEN_GETTER(kMarginBig)
INT_TOKEN_GETTER(kMarginWindow)
INT_TOKEN_GETTER(kMarginComponent)
INT_TOKEN_GETTER(kPaddingMinLeft)
INT_TOKEN_GETTER(kPaddingMinTop)
INT_TOKEN_GETTER(kPaddingMinRight)
INT_TOKEN_GETTER(kPaddingMinBottom)
INT_TOKEN_GETTER(kPaddingNormalLeft)
INT_TOKEN_GETTER(kPaddingNormalTop)
INT_TOKEN_GETTER(kPaddingNormalRight)
INT_TOKEN_GETTER(kPaddingNormalBottom)
INT_TOKEN_GETTER(kPadding8Left)
INT_TOKEN_GETTER(kPadding8Top)
INT_TOKEN_GETTER(kPadding8Right)
INT_TOKEN_GETTER(kPadding8Bottom)
INT_TOKEN_GETTER(kPaddingWindowLeft)
INT_TOKEN_GETTER(kPaddingWindowTop)
INT_TOKEN_GETTER(kPaddingWindowRight)
INT_TOKEN_GETTER(kPaddingWindowBottom)

SHADOW_TOKEN_GETTER(kShadowPrimaryActive)
SHADOW_TOKEN_GETTER(kShadowPrimaryInactive)
SHADOW_TOKEN_GETTER(kShadowSecondaryActive)
SHADOW_TOKEN_GETTER(kShadowSecondaryInactive)
SHADOW_TOKEN_GETTER(kShadowMin)
SHADOW_TOKEN_GETTER(kShadowMenu)
SHADOW_TOKEN_GETTER(kShadowComponent)


COLOR_TOKEN_SETTER(windowTextActive)
COLOR_TOKEN_SETTER(windowTextInactive)
COLOR_TOKEN_SETTER(windowTextDisable)
COLOR_TOKEN_SETTER(buttonActive)
COLOR_TOKEN_SETTER(buttonInactive)
COLOR_TOKEN_SETTER(buttonDisable)
COLOR_TOKEN_SETTER(lightActive)
COLOR_TOKEN_SETTER(lightInactive)
COLOR_TOKEN_SETTER(lightDisable)
COLOR_TOKEN_SETTER(midLightActive)
COLOR_TOKEN_SETTER(midLightInactive)
COLOR_TOKEN_SETTER(midLightDisable)
COLOR_TOKEN_SETTER(darkActive)
COLOR_TOKEN_SETTER(darkInactive)
COLOR_TOKEN_SETTER(darkDisable)
COLOR_TOKEN_SETTER(midActive)
COLOR_TOKEN_SETTER(midInactive)
COLOR_TOKEN_SETTER(midDisable)
COLOR_TOKEN_SETTER(textActive)
COLOR_TOKEN_SETTER(textInactive)
COLOR_TOKEN_SETTER(textDisable)
COLOR_TOKEN_SETTER(brightTextActive)
COLOR_TOKEN_SETTER(brightTextInactive)
COLOR_TOKEN_SETTER(brightTextDisable)
COLOR_TOKEN_SETTER(buttonTextActive)
COLOR_TOKEN_SETTER(buttonTextInactive)
COLOR_TOKEN_SETTER(buttonTextDisable)
COLOR_TOKEN_SETTER(baseActive)
COLOR_TOKEN_SETTER(baseInactive)
COLOR_TOKEN_SETTER(baseDisable)
COLOR_TOKEN_SETTER(windowActive)
COLOR_TOKEN_SETTER(windowInactive)
COLOR_TOKEN_SETTER(windowDisable)
COLOR_TOKEN_SETTER(shadowActive)
COLOR_TOKEN_SETTER(shadowInactive)
COLOR_TOKEN_SETTER(shadowDisable)
COLOR_TOKEN_SETTER(highlightActive)
COLOR_TOKEN_SETTER(highlightInactive)
COLOR_TOKEN_SETTER(highlightDisable)
COLOR_TOKEN_SETTER(highlightedTextActive)
COLOR_TOKEN_SETTER(highlightedTextInactive)
COLOR_TOKEN_SETTER(highlightedTextDisable)
COLOR_TOKEN_SETTER(linkActive)
COLOR_TOKEN_SETTER(linkInactive)
COLOR_TOKEN_SETTER(linkDisable)
COLOR_TOKEN_SETTER(linkVisitedDisable)
COLOR_TOKEN_SETTER(alternateBaseActive)
COLOR_TOKEN_SETTER(alternateBaseInactive)
COLOR_TOKEN_SETTER(alternateBaseDisable)
COLOR_TOKEN_SETTER(noRoleActive)
COLOR_TOKEN_SETTER(noRoleInactive)
COLOR_TOKEN_SETTER(noRoleDisable)
COLOR_TOKEN_SETTER(toolTipBaseActive)
COLOR_TOKEN_SETTER(toolTipBaseInactive)
COLOR_TOKEN_SETTER(toolTipBaseDisable)
COLOR_TOKEN_SETTER(toolTipTextActive)
COLOR_TOKEN_SETTER(toolTipTextInactive)
COLOR_TOKEN_SETTER(toolTipTextDisable)
COLOR_TOKEN_SETTER(placeholderTextActive)
COLOR_TOKEN_SETTER(placeholderTextInactive)
COLOR_TOKEN_SETTER(placeholderTextDisable)

COLOR_TOKEN_SETTER(kWhite)
COLOR_TOKEN_SETTER(kBlack)
COLOR_TOKEN_SETTER(kGray0)
COLOR_TOKEN_SETTER(kGray1)
COLOR_TOKEN_SETTER(kGray2)
COLOR_TOKEN_SETTER(kGray3)
COLOR_TOKEN_SETTER(kGray4)
COLOR_TOKEN_SETTER(kGray5)
COLOR_TOKEN_SETTER(kGray6)
COLOR_TOKEN_SETTER(kGray7)
COLOR_TOKEN_SETTER(kGray8)
COLOR_TOKEN_SETTER(kGray9)
COLOR_TOKEN_SETTER(kGray10)
COLOR_TOKEN_SETTER(kGray11)
COLOR_TOKEN_SETTER(kGray12)
COLOR_TOKEN_SETTER(kGray13)
COLOR_TOKEN_SETTER(kGray14)
COLOR_TOKEN_SETTER(kGray15)
COLOR_TOKEN_SETTER(kGray16)
COLOR_TOKEN_SETTER(kGray17)
COLOR_TOKEN_SETTER(kGrayAlpha0)
COLOR_TOKEN_SETTER(kGrayAlpha1)
COLOR_TOKEN_SETTER(kGrayAlpha2)
COLOR_TOKEN_SETTER(kGrayAlpha3)
COLOR_TOKEN_SETTER(kGrayAlpha4)
COLOR_TOKEN_SETTER(kGrayAlpha5)
COLOR_TOKEN_SETTER(kGrayAlpha6)
COLOR_TOKEN_SETTER(kGrayAlpha7)
COLOR_TOKEN_SETTER(kGrayAlpha8)
COLOR_TOKEN_SETTER(kGrayAlpha9)
COLOR_TOKEN_SETTER(kGrayAlpha10)
COLOR_TOKEN_SETTER(kGrayAlpha11)
COLOR_TOKEN_SETTER(kGrayAlpha12)
COLOR_TOKEN_SETTER(kGrayAlpha13)

COLOR_TOKEN_SETTER(kFontStrong)
COLOR_TOKEN_SETTER(kFontPrimary)
COLOR_TOKEN_SETTER(kFontPrimaryDisable)
COLOR_TOKEN_SETTER(kFontSecondary)
COLOR_TOKEN_SETTER(kFontSecondaryDisable)
COLOR_TOKEN_SETTER(kFontWhite)
COLOR_TOKEN_SETTER(kFontWhiteDisable)
COLOR_TOKEN_SETTER(kFontWhiteSecondary)
COLOR_TOKEN_SETTER(kFontWhiteSecondaryDisable)

COLOR_TOKEN_SETTER(kBrandNormal)
COLOR_TOKEN_SETTER(kSuccessNormal)
COLOR_TOKEN_SETTER(kWarningNormal)
COLOR_TOKEN_SETTER(kErrorNormal)
COLOR_TOKEN_SETTER(kBrand1)
COLOR_TOKEN_SETTER(kBrand2)
COLOR_TOKEN_SETTER(kBrand3)
COLOR_TOKEN_SETTER(kBrand4)
COLOR_TOKEN_SETTER(kBrand5)
COLOR_TOKEN_SETTER(kBrand6)
COLOR_TOKEN_SETTER(kBrand7)

COLOR_TOKEN_SETTER(kContainGeneralNormal)
COLOR_TOKEN_SETTER(kContainSecondaryNormal)
COLOR_TOKEN_SETTER(kContainSecondaryAlphaNormal)
COLOR_TOKEN_SETTER(kComponentNormal)
COLOR_TOKEN_SETTER(kComponentDisable)
COLOR_TOKEN_SETTER(kComponentAlphaNormal)
COLOR_TOKEN_SETTER(kComponentAlphaDisable)
COLOR_TOKEN_SETTER(kLineWindowActive)
COLOR_TOKEN_SETTER(kLineComponentNormal)
COLOR_TOKEN_SETTER(kLineComponentHover)
COLOR_TOKEN_SETTER(kLineComponentClick)
COLOR_TOKEN_SETTER(kLineComponentDisable)
COLOR_TOKEN_SETTER(kLineBrandNormal)
COLOR_TOKEN_SETTER(kLineBrandHover)
COLOR_TOKEN_SETTER(kLineBrandClick)
COLOR_TOKEN_SETTER(kLineBrandDisable)
COLOR_TOKEN_SETTER(kModalMask)
COLOR_TOKEN_SETTER(kComponentAlphaClick)
COLOR_TOKEN_SETTER(kComponentAlphaHover)
COLOR_TOKEN_SETTER(kComponentHover)
COLOR_TOKEN_SETTER(kComponentClick)
COLOR_TOKEN_SETTER(kContainHover)
COLOR_TOKEN_SETTER(kContainClick)
COLOR_TOKEN_SETTER(kBrandHover)
COLOR_TOKEN_SETTER(kBrandClick)
COLOR_TOKEN_SETTER(kBrandFocus)
COLOR_TOKEN_SETTER(kSuccessHover)
COLOR_TOKEN_SETTER(kSuccessClick)
COLOR_TOKEN_SETTER(kErrorHover)
COLOR_TOKEN_SETTER(kErrorClick)
COLOR_TOKEN_SETTER(kWarningHover)
COLOR_TOKEN_SETTER(kWarningClick)
COLOR_TOKEN_SETTER(linkVisitedActive)
COLOR_TOKEN_SETTER(linkVisitedInactive)
COLOR_TOKEN_SETTER(kErrorDisable)
COLOR_TOKEN_SETTER(kWarningDisable)
COLOR_TOKEN_SETTER(kLineNormal)
COLOR_TOKEN_SETTER(kContainGeneralAlphaNormal)
COLOR_TOKEN_SETTER(kSuccessDisable)
COLOR_TOKEN_SETTER(kLineWindowInactive)
COLOR_TOKEN_SETTER(kContainAlphaClick)
COLOR_TOKEN_SETTER(kContainAlphaHover)
COLOR_TOKEN_SETTER(kBrandDisable)
COLOR_TOKEN_SETTER(kLineDisable)
COLOR_TOKEN_SETTER(kDivider)

COLOR_TOKEN_SETTER(kBrand8)
COLOR_TOKEN_SETTER(kBrand9)
COLOR_TOKEN_SETTER(kLineSelectboxNormal)
COLOR_TOKEN_SETTER(kLineSelectboxHover)
COLOR_TOKEN_SETTER(kLineSelectboxClick)
COLOR_TOKEN_SETTER(kLineSelectboxSelected)
COLOR_TOKEN_SETTER(kLineSelectboxDisable)
COLOR_TOKEN_SETTER(kFontWhitePlaceholderTextDisable)
COLOR_TOKEN_SETTER(kContainGeneralInactive)
COLOR_TOKEN_SETTER(kFontPlaceholderTextDisable)
COLOR_TOKEN_SETTER(kFontWhitePlaceholderText)
COLOR_TOKEN_SETTER(kLineInputDisable)
COLOR_TOKEN_SETTER(kLineDock)
COLOR_TOKEN_SETTER(kLineMenu)
COLOR_TOKEN_SETTER(kLineTable)
COLOR_TOKEN_SETTER(kLineInputNormal)
COLOR_TOKEN_SETTER(kLineInputHover)
COLOR_TOKEN_SETTER(kLineInputClick)
COLOR_TOKEN_SETTER(kOrange1)
COLOR_TOKEN_SETTER(kGreen1)
COLOR_TOKEN_SETTER(kRed1)
COLOR_TOKEN_SETTER(kDividerWhite)
COLOR_TOKEN_SETTER(kLineSuccessNormal)
COLOR_TOKEN_SETTER(kLineSuccessHover)
COLOR_TOKEN_SETTER(kLineSuccessClick)
COLOR_TOKEN_SETTER(kLineSuccessDisable)
COLOR_TOKEN_SETTER(kLineErrorNormal)
COLOR_TOKEN_SETTER(kLineErrorHover)
COLOR_TOKEN_SETTER(kLineErrorClick)
COLOR_TOKEN_SETTER(kLineWarningNormal)
COLOR_TOKEN_SETTER(kLineWarningHover)
COLOR_TOKEN_SETTER(kLineWarningClick)
COLOR_TOKEN_SETTER(kLineWarningDisable)
COLOR_TOKEN_SETTER(kComponentSelectedNormal)
COLOR_TOKEN_SETTER(kComponentSelectedHover)
COLOR_TOKEN_SETTER(kComponentSelectedClick)
COLOR_TOKEN_SETTER(kComponentSelectedDisable)
COLOR_TOKEN_SETTER(kComponentSelectedAlphaNormal)
COLOR_TOKEN_SETTER(kComponentSelectedAlphaHover)
COLOR_TOKEN_SETTER(kComponentSelectedAlphaClick)
COLOR_TOKEN_SETTER(kComponentSelectedAlphaDisable)
COLOR_TOKEN_SETTER(kComponentInput)
COLOR_TOKEN_SETTER(kComponentInputAlpha)
COLOR_TOKEN_SETTER(kSelectAlphaWhiteDisable)
COLOR_TOKEN_SETTER(kSelectAlphaWhiteHover)
COLOR_TOKEN_SETTER(kSelectAlphaWhiteClick)
COLOR_TOKEN_SETTER(kMenu)
COLOR_TOKEN_SETTER(kLineErrorDisable)
COLOR_TOKEN_SETTER(kContainSecondaryInactive)
COLOR_TOKEN_SETTER(kSelectAlphaWhite)
COLOR_TOKEN_SETTER(kFontPlaceholderText)
COLOR_TOKEN_SETTER(kPanel)
COLOR_TOKEN_SETTER(kBrand10)

INT_TOKEN_SETTER(normalLine)
INT_TOKEN_SETTER(focusLine)
INT_TOKEN_SETTER(kRadiusMin)
INT_TOKEN_SETTER(kRadiusNormal)
INT_TOKEN_SETTER(kRadiusMax)
INT_TOKEN_SETTER(kRadiusMenu)
INT_TOKEN_SETTER(kRadiusWindow)
INT_TOKEN_SETTER(kMarginMin)
INT_TOKEN_SETTER(kMarginNormal)
INT_TOKEN_SETTER(kMarginBig)
INT_TOKEN_SETTER(kMarginWindow)
INT_TOKEN_SETTER(kMarginComponent)
INT_TOKEN_SETTER(kPaddingMinLeft)
INT_TOKEN_SETTER(kPaddingMinTop)
INT_TOKEN_SETTER(kPaddingMinRight)
INT_TOKEN_SETTER(kPaddingMinBottom)
INT_TOKEN_SETTER(kPaddingNormalLeft)
INT_TOKEN_SETTER(kPaddingNormalTop)
INT_TOKEN_SETTER(kPaddingNormalRight)
INT_TOKEN_SETTER(kPaddingNormalBottom)
INT_TOKEN_SETTER(kPadding8Left)
INT_TOKEN_SETTER(kPadding8Top)
INT_TOKEN_SETTER(kPadding8Right)
INT_TOKEN_SETTER(kPadding8Bottom)
INT_TOKEN_SETTER(kPaddingWindowLeft)
INT_TOKEN_SETTER(kPaddingWindowTop)
INT_TOKEN_SETTER(kPaddingWindowRight)
INT_TOKEN_SETTER(kPaddingWindowBottom)

SHADOW_TOKEN_SETTER(kShadowPrimaryActive)
SHADOW_TOKEN_SETTER(kShadowPrimaryInactive)
SHADOW_TOKEN_SETTER(kShadowSecondaryActive)
SHADOW_TOKEN_SETTER(kShadowSecondaryInactive)
SHADOW_TOKEN_SETTER(kShadowMin)
SHADOW_TOKEN_SETTER(kShadowMenu)
SHADOW_TOKEN_SETTER(kShadowComponent)
} // UkuiQuick
