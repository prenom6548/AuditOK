/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef MAIN_FRAME_UTILS_H_
#define MAIN_FRAME_UTILS_H_

#include <QMap>
#include <QString>
#include <QSysInfo>
#include <QJsonObject>

typedef struct
{
    bool isLoad;
    bool isRoot;
    int sort;
} pluginconfig_t;

/* 目前插件跟百宝箱需要的配置字段并没有完全确定，此处分别定义各自类型，方便之后扩展 */
typedef struct
{
    bool isLoad;
    bool isRoot;
    int sort;
    QString name_zh_CN;
    QString name_en_AU;
    QString name_bo_CN;
    QString name_mn_MN;
    QString comment_zh_CN;
    QString comment_en_AU;
    QString comment_bo_CN;
    QString comment_mn_MN;
    QString icon;
    QString defaultIcon;
    QString exec;
    QString architecture;
} boxconfig_t;

enum Theme { Dark = 0, Light, Default };

class Utils
{
public:
    Utils();
    ~Utils();

    /**
     * @brief 获取插件的配置信息
     *
     * @param 无
     *
     * @return 插件配置信息
     */
    static QMap<QString, pluginconfig_t> getPluginConfig(void);

    /**
     * @brief 获取百宝箱工具的配置信息
     *
     * @param 无
     *
     * @return 百宝箱工具配置信息
     */
    static QMap<QString, boxconfig_t> getBoxConfig(void);

    /**
     * @brief 获取需要加载的插件
     *
     * @param data 插件配置信息
     *
     * @return 需要加载的插件(升序排列)
     */
    static QVector<QString> getLoadPlugin(const QMap<QString, pluginconfig_t> &data);

    /**
     * @brief 获取需要加载的百宝箱工具
     *
     * @param data 百宝箱工具配置信息
     *
     * @return 需要加载的百宝箱工具(升序排列)
     */
    static QVector<QString> getLoadBox(const QMap<QString, boxconfig_t> &data);

    /**
     * @brief 获取系统主题
     *
     * @param 无
     *
     * @return 系统主题
     */
    static Theme getTheme(void);

    /**
     * @brief 获取系统字号
     *
     * @param 无
     *
     * @return 系统字号
     */
    static double getFontSize(void);

    /**
     * @brief 获取自适应字符串
     *
     * @param w 需要设置字符串的控件指针
     * @param text 需要设置的文本
     *
     * @return 自适应的文本
     */
    static QString getSelfAdaptionText(QWidget *w, QString text);

private:
    static void analysisPluginConfig(const QJsonObject &obj, pluginconfig_t &result);
    static void printPluginConfig(QMap<QString, pluginconfig_t> &data);

    static void analysisBoxConfig(const QJsonObject &obj, boxconfig_t &result);
    static void printBoxConfig(QMap<QString, boxconfig_t> &data);
    static void boxConfigClear(boxconfig_t &config);
};

#endif
