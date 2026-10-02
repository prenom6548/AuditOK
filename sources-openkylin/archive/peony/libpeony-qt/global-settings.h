/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QDBusInterface>
#include <QJsonObject>

#include "peony-core_global.h"
#include <gio/gio.h>

//文件管理器版本
#define PEONY_VERSION               "peonyVersion"

//顶部菜单 - Top menu
#define RESIDENT_IN_BACKEND         "resident"
#define SHOW_DESKTOP_HIDDEN_PREFERENCE      "showDesktopHiddenFile"
#define SHOW_HIDDEN_PREFERENCE      "showHiddenFile"
#define ALLOW_FILE_OP_PARALLEL      "allowFileOpParallel"
#define FORBID_THUMBNAIL_IN_VIEW    "doNotThumbnail"
#define SHOW_IN_NEW_WINDOW          "showInNewWindow"
#define DISABLED_EXTENSIONS         "disabledExtensions"
#define SHOW_SHARE_PROPERTIES       "showShareProperties"
#define APP_WHITE_LIST              "appWhiteList"
#define APP_BLACK_LIST              "appBlackList"

//视图 - View
#define DEFAULT_VIEW_ID             "defaultViewId"
#define DEFAULT_VIEW_ZOOM_LEVEL     "defaultViewZoomLevel"
#define DEFAULT_DETAIL              "detail"
#define LABLE_ALIGNMENT             "labelAlignment"
#define LISTVIEW_COLUMN_WIDTHS      "listviewColumnWidths"
#define USE_GLOBAL_DEFAULT_VIEW     "useGlobalDefaultView"

//排序类型 - Sort type
// fix 112308, sort type option does no effect issue.
#define SORT_COLUMN                 "sortType"
#define SORT_TYPE                   "sortType"

//排序顺序 - Sort order
#define SORT_ORDER                  "sortOrder"

//排序更新图标
#define SORT_UPDATE                 "sortUpdate"

//排序偏好 - Sort preference
#define SORT_FOLDER_FIRST           "folderFirst"
#define SORT_CHINESE_FIRST          "chineseFirst"
#define USE_GLOBAL_DEFAULT_SORTING "useGlobalDefaultSorting"
#define SHOW_CREATE_TIME            "showCreateTime"

//窗口设置 - Window setting
#define DEFAULT_WINDOW_WIDTH        "defaultWindowWidth"
#define DEFAULT_WINDOW_HEIGHT       "defaultWindowHeight"
#define DEFAULT_WINDOW_SIZE         "default-window-size" //deprecated

//侧边栏 - Sidebar
#define DEFAULT_SIDEBAR_WIDTH       "defaultSidebarWidth"

//删除提示框 - tooltip
#define SHOW_TRASH_DIALOG           "showTrashDialog"

//状态栏 - Zoom Slider
#define ZOOM_SLIDER_VISIBLE         "zoomSliderVisible"

//多选模式 - Multiple selection
#define MULTI_SELECT                "multiSelectCheckboxEnabled"
#define SHOW_MULTISELECT_CHECKBOX   "showMultiselectCheckbox"

//桌面配置 - Desktop setting
#define LAST_DESKTOP_SORT_ORDER     "lastDesktopSortOrder"
#define DESKTOP_SORT_ORDER          "desktop-sort-order"
#define TEMPLATES_DIR               "templatesDir"
#define DEFAULT_DESKTOP_ZOOM_LEVEL  "defaultDesktopZoomLevel"
#define DEFAULT_GRID_SIZE           "default-grid-size"

//收藏的服务器IP - favorite ip
#define REMOTE_SERVER_REMOTE_IP     "remote-server/favorite-ip"

//gsettings
#define SIDEBAR_BG_OPACITY          "sidebar-bg-opacity" // deprecated
#define TABLET_MODE                 "tablet-mode"

#define FONT_SETTINGS               "org.ukui.style"

//difference between Community version and Commercial version
#define COMMERCIAL_VERSION          true

//其它
#define INIT_FOR_FIRST_TIME         "initForFirstTime"  /* org.ukui.peony.settings 首次初始化标志 */
#define SHOW_FILE_EXTENSION         "showFileExtension" /* 显示文件扩展名 */
#define SEND_URIS_OF_COPY_DSPS      "sendUrisOfCopyDsps"  /* send src and dest uris of copy dsps files to wps,默认值为false */
#define DOC_IS_OCCUPIED_BY_WPS      "docIsOccupiedByWps" /* 是否开启wps占用文件监控功能 */

#define UKUI_SEARCH_SCHEMAS          "org.ukui.search.settings"
#define SEARCH_METHOD_KEY            "fileIndexEnable"

//Control the display of desktop standard icons
#define DISPLAY_STANDARD_ICONS       "displayStandardIcons"
#define HOME_ICON_VISIBLE            "homeIconVisible"
#define TRASH_ICON_VISIBLE           "trashIconVisible"
#define COMPUTER_ICON_VISIBLE        "computerIconVisible"

//control the mobile device trash file issue, if be true can trash mobile files
#define TRASH_MOBILE_FILES            "trashMobileFiles"

//control the display of right menu terminal
#define SHOW_OPEN_TERMINAL           "showOpenTerminal"

//Control start peony
#define ENABLE_START_PEONY           "enableStartPeony"

//Control double click desktop file
#define ENABLE_DOUBLE_CLICK_DESKTOP  "enableDoubleClickDesktop"

//Control file operation of shortcut keys, such as Ctrl+C、Ctrl+V、Ctrl+A、Delete、Shift+Delete、Ctrl+X
#define ENABLE_SHORTCUT_KEYS         "enableShortcutKeys"

#define ENABLE_SMB_SHARE             "enableSmbShare"

// control center
#define UKUI_CONTROL_CENTER_PANEL_PLUGIN            "org.ukui.control-center.panel.plugins"                 // schema
#define UKUI_CONTROL_CENTER_PANEL_PLUGIN_TIME       "org.ukui.control-center.panel.plugins.time"            // time format key, value is '12' or '24'
#define UKUI_CONTROL_CENTER_PANEL_PLUGIN_DATE       "org.ukui.control-center.panel.plugins.date"            // date format key, value is cn or en

// desktop
#define DESKTOP_USE_AUTO_LAYOUT      "desktopUseAutoLayout"
#define DESKTOP_USE_ORIGANIZATION      "desktopUseAutoOriganization"
// guestos machine
#define IS_GUESTOS_MACHINE           "isGuestOSMachine"

//gsettings
#define PERSONAL_EFFECT_SCHEMA       "org.ukui.control-center.personalise"
#define PERSONAL_EFFECT_ENABLE       "effect"
#define PERSONAL_EFFECT_TRANSPARENCY "transparency"

#define SHOW_NETWORK                "showNetwork"

#define SHOW_RELATIVE_DATE          "showRelativeDate"

//intel 个性化设置，透明度...
//schema
#define UKUI_CONTROL_CENTER_PERSONALISE             "org.ukui.control-center.personalise"
//keys
#define PERSONALISE_EFFECT                          "effect"
#define PERSONALISE_TRANSPARENCY                    "transparency"

#define SETTINGS_DAEMON_SCHEMA_XRANDR        "org.ukui.SettingsDaemon.plugins.xrandr"
#define DUAL_SCREEN_MODE                     "xrandrMirrorMode"
#define DUAL_SCREEN_EXPAND_MODE              "expand"
#define DUAL_SCREEN_MIRROR_MODE              "mirror"

//dbus
#define DBUS_STATUS_MANAGER_IF      "com.kylin.statusmanager.interface"

//Control mouse click and right-click menu operations
#define ENABLE_MOUSE_CLICK_FILE                "enableMouseClickFile"             // 鼠标左键单击文件
#define ENABLE_MOUSE_DOUBLE_CLICK_FILE         "enableMouseDoubleClickFile"       // 鼠标左键双击文件
#define ENABLE_MOUSE_CLICK_FOLDER              "enableMouseClickFolder"           // 鼠标左键单击文件夹
#define ENABLE_MOUSE_DOUBLE_CLICK_FOLDER       "enableMouseDoubleClickFolder"     // 鼠标左键双击文件夹
#define ENABLE_MOUSE_CLICK_APP_START           "enableMouseClickAppStart"         // 鼠标左键单击桌面图标
#define ENABLE_MOUSE_DOUBLE_CLICK_APP_START    "enableMouseDoubleClickAppStart"   // 鼠标左键双击桌面图标
#define ENABLE_DESKTOP_FILE_MENU               "enableDesktopFileMenu"            // 鼠标桌面右键菜单
#define ENABLE_DESKTOP_BACKGROUND_MENU         "enableDesktopBackgroundMenu"      // 桌面空白处鼠标右键菜单
#define ENABLE_PEONY_RIGHT_MENU                "enablePeonyRightMenu"             // 桌面文件夹鼠标右键菜单、文管右键菜单
#define ENABLE_DESKTOP_APP_START_MENU          "enableDesktopAppStartMenu"        // 桌面应用鼠标右键菜单

// 功能管控宏定义
#define ENABLE_FORMAT_REMOVABLE_DEVICE         "enableFormatRemovableDevice"      // 可移动设备格式化功能
#define ENABLE_FILE_PREVIEW_PANE               "enableFilePreviewPane"            // 开启文件详情预览窗格
#define ENABLE_OPTICAL_DRIVE                   "enableOpticalDrive"               // 开启光盘功能
#define ENABLE_FILE_SEARCH                     "enableFileSearch"                 // 开启文件搜索功能
#define ENABLE_LOCAL_DISK_UNMOUNT              "enableLocalDiskUnmount"           // 本地磁盘卸载选项
#define ENABLE_PERMANENT_DELETE                "enablePermanentDelete"            // 永久删除功能

#ifdef KY_SDK_DATE
#define SDK_DATE_SERVER_PATH "/com/kylin/kysdk/Date"
#define SDK_DATE_SERVER_SERVICE "com.kylin.kysdk.DateServer"
#define SDK_DATE_SERVER_INTERFACE "com.kylin.kysdk.DateInterface"
#endif

// Configuration schema
#define PEONY_CONFIG_NAME         "org.ukui.peony.settings"
#define PEONY_SETTINGS_KEY        "org.ukui.peony.settings"

// Power manager config
#define POWER_MANAGER_CONFIG    "org.ukui.power-manager"
#define POWER_POLICY            "powerPolicyCurrent"

//Control delevier desktop file config
//not a gsettings value, used to save delever file info
#define CONTROL_DELIVER_DESKTOP_FILE           "controlDeliverDesktopFile"

class QGSettings;

namespace Peony {

/*!
 * \brief The GlobalSettings class
 * \details
 * this class provide a global settings for peony-qt.
 * it most be used to save and load preferences of peony-qt, such as default view,
 * sort type, etc.
 *
 * you can also save another kind of datas using by extensions. such as enable properties.
 * this class instance is shared in both peony-qt and its plugins.
 */
class PEONYCORESHARED_EXPORT GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings *getInstance();
    const QVariant getValue(const QString &key);
    bool isExist(const QString &key);
    bool initDateFormatDBus();
    QString getProjectName();
    bool isDesktopStartUp() const;
    void setDesktopStartUp(bool startUp);/* 桌面启动和结束时使用，谨慎调用 */
    void sendNotifyMessage(const QString &msg);

    bool getShowCreateTime() const;
    bool isJsonCacheKey(const QString &key);
Q_SIGNALS:
    void valueChanged(const QString &key);
    void updateHiddenFile(const QString &fileName);
    void signal_updateRemoteServer(const QString& server, bool add);
    void updateShortDataFormat(const QString &fileName);
    void updateLongDataFormat(const QString &fileName);

public Q_SLOTS:
    void setValue(const QString &key, const QVariant &value);
    void reset(const QString &key);
    void resetAll();
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void sendShortDataFormat(const QString &format);
    void sendLongDataFormat(const QString &format);
    QString getSystemTimeFormat();
    QString transToSystemTimeFormat(guint64 mtime, bool longFormat=false);

    /*!
     * \brief 通过GSetting保存设置
     * \param key
     * \param value
     */
    void setGSettingValue(const QString &key, const QVariant &value);

    /*!
     * \brief forceSync
     * \param key
     * \details
     * use force sync to ensure the settings key-value and cached map's data
     * keep same. this may be used in out progress, such as peony-qt-desktop.
     */
    void forceSync(const QString &key = nullptr);
    void slot_updateRemoteServer(const QString& server, bool add);

    bool isGuestOSMachine();

    /**
     * @brief Handle configuration change signal from DBus
     * @param userName The username
     * @param appConfigList List of changed configuration files
     */
    void onConfigChanged(const QString &userName, const QStringList &appConfigList);

    /**
     * @brief Handle security exit signal from DBus
     */
    void onSecurityExit();

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    ~GlobalSettings();

    void getUkuiStyle();
    void getMachineMode();
    void getDualScreenMode();
    /**
     * @brief Initialize safe manage control configuration sources
     */
    void initManageControl();

    /**
     * @brief Load and parse JSON configuration file
     *
     * This function attempts to load and parse a JSON configuration file from the specified path.
     * If the file cannot be opened, parsed, or contains invalid data, the JSON cache will be cleared.
     *
     * The JSON structure should follow the format:
     * {
     *     "org.ukui.peony.settings": {
     *         "setting-key1": "value1",
     *         "setting-key2": "value2"
     *     }
     * }
     *
     * @param configPath The full path to the JSON configuration file
     * @return bool Returns true if the configuration was successfully loaded and parsed,
     *              false if any error occurred during the process
     *
     * @note On any failure (file not found, parse error, empty content),
     *       the internal JSON cache (m_jsonCache) will be cleared
     *
     * @see updateConfigCache()
     */
    bool loadJsonConfig(const QString &configPath);

    /**
     * @brief Convert JSON configuration key to internal key format
     * @param jsonKey The key in JSON format
     * @return QString The converted internal key
     */
    QString convertJsonKeyToInternalKey(const QString &jsonKey);

    /**
     * @brief Update configuration cache and emit signals for changed values
     * @param newConfig The new configuration object
     */
    void updateConfigCache(const QJsonObject &peonySettings);

    /**
     * @brief Get configuration path from DBus service
     * @param userName The username
     * @param fileName The configuration file name
     * @return QString The configuration file path
     */
    QString getConfigPathFromDBus(const QString &userName, const QString &fileName);

    void initControlDefaults();

    template<typename Key, typename T>
    bool containsKey(const QMap<Key, T>& map, const Key& key) {
        return map.contains(key);
    }

    QMap<QString, QVariant>     m_jsonCache;       ///< Cache for JSON configuration values
    QString                     m_currentConfigPath;

    QSettings*                  m_settings;
    QMap<QString, QVariant>     m_cache;

    QGSettings*                 m_gsettings = nullptr;
    QGSettings*                 m_control_center_plugin = nullptr;
    QGSettings*                 m_peony_gsettings  = nullptr;
    QGSettings*                 m_peonyGSettings  = nullptr; //fixme: 代码冲突遗留，后续仅保留一个peony gsettings实例
    QGSettings *m_gsettings_tablet_mode = nullptr;
    QGSettings *m_gsettings_dual_screen_mode = nullptr;
    QMutex                      m_mutex;

    QString                     m_date_format = "";
    QString                     m_time_format = "";
    QString                     m_system_time_format  = "";

    QDBusInterface*             mDbusDateServer = nullptr;
    bool m_showCreateTime = false;
    bool m_showRelativeTime = false;
    bool                        m_isDesktopStartUp = false; /* 桌面进程启动中 */
    QDBusInterface*             mDbusSettingsServer = nullptr;
    QString                     m_currentUser;
};

}

#endif // GLOBALSETTINGS_H
