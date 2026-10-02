#ifndef GSETTINGSWRAPPER_H
#define GSETTINGSWRAPPER_H

#include <QHash>
#include <QSize>
#include <QColor>
#include <QGSettings/qgsettings.h>
#include "../common/singleton.h"

class QWidget;

// 主题key
#define ORG_UKUI_STYLE            "org.ukui.style"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_VALUE_DEFAULT  "ukui-default"
#define STYLE_NAME_VALUE_DARK     "ukui-dark"
#define STYLE_NAME_VALUE_BLACK    "ukui-black"
#define STYLE_NAME_VALUE_LIGHT    "ukui-light"
#define STYLE_NAME_VALUE_WHITE    "ukui-white"
#define ICON_THEME_NAME           "iconThemeName"
#define TRANSPARENCY_NAME		  "transparency"
#define COLOR_QT_KEY              "themeColor"

// 颜色
// 浅灰
#define COLOR_WINDOW_LIGHT_LEFT    "#FAFAFA"
// 深灰
#define COLOR_WINDOW_DARK_LEFT     "#232426"
// #define COLOR_WINDOW_DARK_LEFT     "#242424"
// 白色
#define COLOR_WINDOW_LIGHT_RIGHT   "#FFFFFF"
// 黑色
#define COLOR_WINDOW_DARK_RIGHT    "#121212"

/**
 * @brief 由于当前仅用于主题变化时，备份还原工具的一些图标跟随变化，此处暂时先简单封装。后续根据使用情况另行优化。
 */
class GSettingsWrapper : public Singleton<GSettingsWrapper>
{
public:
    GSettingsWrapper(token);
    virtual ~GSettingsWrapper();

    /**
     * @brief 是否深色主题
     * @return true-深色主题；false-浅色主题
     */
    bool isDarkTheme() {
        return m_isDarkTheme;
    }

public:

    QGSettings *m_pGsettingThemeData;
    bool m_isDarkTheme;
};

#define g_GSettingWrapper GSettingsWrapper::instance()

#endif // GSETTINGSWRAPPER_H
