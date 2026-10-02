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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QList>
#include <QDebug>
#include <QVector>
#include <QFontMetrics>
#include <QWidget>
#include <gsettingmonitor.h>
#include "utils.h"

static constexpr char pluginConfigPath[] = "/etc/kylin-os-manager/kylin-os-manager-plugin.conf";
static constexpr char boxConfigPath[] = "/etc/kylin-os-manager/kylin-os-manager-box.conf";

Utils::Utils() = default;

Utils::~Utils() = default;

static bool lessThanPlugin(const QPair<QString, pluginconfig_t> &a, const QPair<QString, pluginconfig_t> &b)
{
    return a.second.sort < b.second.sort;
}

static bool lessThanBox(const QPair<QString, boxconfig_t> &a, const QPair<QString, boxconfig_t> &b)
{
    return a.second.sort < b.second.sort;
}

QMap<QString, pluginconfig_t> Utils::getPluginConfig(void)
{
    QMap<QString, pluginconfig_t> ret;
    ret.clear();

    QFile file(pluginConfigPath);
    if (!file.exists()) {
        qCritical() << "config file is not exists !";
        return ret;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "get config file fail !";
        return ret;
    }
    QString data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data.toLocal8Bit().data());
    QJsonObject object = doc.object();
    QStringList keys = object.keys();
    pluginconfig_t pluginConfig;
    for (int i = 0; i < keys.size(); i++) {
        QJsonObject pluginConfigObj = object.value(keys.at(i)).toObject();
        analysisPluginConfig(pluginConfigObj, pluginConfig);
        ret.insert(keys.at(i), pluginConfig);
    }

    printPluginConfig(ret);
    return ret;
}

void Utils::analysisPluginConfig(const QJsonObject &obj, pluginconfig_t &result)
{
    /* 与配置文件对应的 key 值，需跟随配置文件一起改动 */

    /* 初始化结构 */
    result.isLoad = false;
    result.isRoot = false;
    result.sort = -1;

    if (obj.contains("isLoad")) {
        result.isLoad = obj.value("isLoad").toBool();
    }

    if (obj.contains("isRoot")) {
        result.isRoot = obj.value("isRoot").toBool();
    }

    if (obj.contains("sort")) {
        result.sort = obj.value("sort").toInt();
    }

    return;
}

QVector<QString> Utils::getLoadPlugin(const QMap<QString, pluginconfig_t> &data)
{
    QMap<QString, pluginconfig_t> config = data;

    /* 移除不需要加载的项 */
    QStringList keys = config.keys();
    for (int i = 0; i < keys.size(); i++) {
        if (!config.value(keys.at(i)).isLoad) {
            config.remove(keys.at(i));
        }
    }

    /* 将 QMap 转成 QVector */
    QVector<QPair<QString, pluginconfig_t>> vec;
    for (QMap<QString, pluginconfig_t>::iterator it = config.begin(); it != config.end(); it++) {
        vec.append(qMakePair(it.key(), it.value()));
    }

    /* 根据 sort 字段排序 */
    qSort(vec.begin(), vec.end(), lessThanPlugin);

    QVector<QString> result;
    for (int i = 0; i < vec.size(); i++) {
        result.append(vec.at(i).first);
    }

    return result;
}

void Utils::printPluginConfig(QMap<QString, pluginconfig_t> &data)
{
    for (QMap<QString, pluginconfig_t>::iterator it = data.begin(); it != data.end(); it++) {
        QString key = it.key();
        pluginconfig_t config = it.value();

        qDebug() << "Name: " << key << "isLoad: " << config.isLoad << "isRoot: " << config.isRoot
                 << "sort: " << config.sort;
    }

    return;
}

QMap<QString, boxconfig_t> Utils::getBoxConfig(void)
{
    QMap<QString, boxconfig_t> ret;
    ret.clear();

    QFile file(boxConfigPath);
    if (!file.exists()) {
        qCritical() << "config file is not exists !";
        return ret;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "get config file fail !";
        return ret;
    }

    QString data = file.readAll();

    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data.toLocal8Bit().data());
    QJsonObject object = doc.object();

    QStringList keys = object.keys();
    boxconfig_t boxConfig;
    for (int i = 0; i < keys.size(); i++) {
        QJsonObject boxConfigObj = object.value(keys.at(i)).toObject();
        analysisBoxConfig(boxConfigObj, boxConfig);
        ret.insert(keys.at(i), boxConfig);
    }

    printBoxConfig(ret);

    return ret;
}

void Utils::analysisBoxConfig(const QJsonObject &obj, boxconfig_t &result)
{
    /* 与配置文件对应的 key 值，需跟随配置文件一起改动 */

    /* 初始化结构 */
    boxConfigClear(result);

    if (obj.contains("isLoad")) {
        result.isLoad = obj.value("isLoad").toBool();
    }

    if (obj.contains("isRoot")) {
        result.isRoot = obj.value("isRoot").toBool();
    }

    if (obj.contains("sort")) {
        result.sort = obj.value("sort").toInt();
    }

    if (obj.contains("name[zh_CN]")) {
        result.name_zh_CN = obj.value("name[zh_CN]").toString();
    }

    if (obj.contains("name[en_AU]")) {
        result.name_en_AU = obj.value("name[en_AU]").toString();
    }

    if (obj.contains("name[bo_CN]")) {
        result.name_bo_CN = obj.value("name[bo_CN]").toString();
    }

    if (obj.contains("name[mn_MN]")) {
        result.name_mn_MN = obj.value("name[mn_MN]").toString();
    }

    if (obj.contains("name[ky_KG]")) {
        result.name_ky_KG = obj.value("name[ky_KG]").toString();
    }

    if (obj.contains("name[kk_KZ]")) {
        result.name_kk_KZ = obj.value("name[kk_KZ]").toString();
    }

    if (obj.contains("name[ug_CN]")) {
        result.name_ug_CN = obj.value("name[ug_CN]").toString();
    }

    if (obj.contains("comment[zh_CN]")) {
        result.comment_zh_CN = obj.value("comment[zh_CN]").toString();
    }

    if (obj.contains("comment[en_AU]")) {
        result.comment_en_AU = obj.value("comment[en_AU]").toString();
    }

    if (obj.contains("comment[bo_CN]")) {
        result.comment_bo_CN = obj.value("comment[bo_CN]").toString();
    }

    if (obj.contains("comment[mn_MN]")) {
        result.comment_mn_MN = obj.value("comment[mn_MN]").toString();
    }

    if (obj.contains("comment[kk_KZ]")) {
        result.comment_kk_KZ = obj.value("comment[kk_KZ]").toString();
    }

    if (obj.contains("comment[ky_KG]")) {
        result.comment_ky_KG = obj.value("comment[ky_KG]").toString();
    }

    if (obj.contains("comment[ug_CN]")) {
        result.comment_ug_CN = obj.value("comment[ug_CN]").toString();
    }

    if (obj.contains("icon")) {
        result.icon = obj.value("icon").toString();
    }

    if (obj.contains("defaultIcon")) {
        result.defaultIcon = obj.value("defaultIcon").toString();
    }

    if (obj.contains("exec")) {
        result.exec = obj.value("exec").toString();
    }

    if (obj.contains("architecture")) {
        result.architecture = obj.value("architecture").toString();
    }

    return;
}

QVector<QString> Utils::getLoadBox(const QMap<QString, boxconfig_t> &data)
{
    QMap<QString, boxconfig_t> config = data;

    QString arch = QSysInfo::currentCpuArchitecture();
    qDebug() << "cpu架构：" << arch;
    /* 移除不需要加载的项 */
    QStringList keys = config.keys();
    for (int i = 0; i < keys.size(); i++) {
        boxconfig_t tmpConf = config.value(keys.at(i));
        if (!tmpConf.isLoad) {
            config.remove(keys.at(i));
        }
        if (!tmpConf.architecture.isEmpty()) {
            QStringList tmpList = tmpConf.architecture.split(',');
            if (!tmpList.contains(arch)) {
                config.remove(keys.at(i));
            }
        }
    }

    /* 将 QMap 转成 QVector */
    QVector<QPair<QString, boxconfig_t>> vec;
    for (QMap<QString, boxconfig_t>::iterator it = config.begin(); it != config.end(); it++) {
        vec.append(qMakePair(it.key(), it.value()));
    }

    /* 根据 sort 字段排序 */
    qSort(vec.begin(), vec.end(), lessThanBox);

    QVector<QString> result;
    for (int i = 0; i < vec.size(); i++) {
        result.append(vec.at(i).first);
    }

    return result;
}

void Utils::boxConfigClear(boxconfig_t &config)
{
    config.isLoad = false;
    config.isRoot = false;
    config.sort = -1;
    config.name_zh_CN = "";
    config.name_en_AU = "";
    config.name_bo_CN = "";
    config.name_mn_MN = "";
    config.comment_zh_CN = "";
    config.comment_en_AU = "";
    config.comment_bo_CN = "";
    config.comment_mn_MN = "";
    config.icon = "";
    config.defaultIcon = "";
    config.exec = "";
    config.architecture = "";

    return;
}

void Utils::printBoxConfig(QMap<QString, boxconfig_t> &data)
{
    for (QMap<QString, boxconfig_t>::iterator it = data.begin(); it != data.end(); it++) {
        QString key = it.key();
        boxconfig_t config = it.value();

        qDebug() << "Name: " << key << "isLoad: " << config.isLoad << "isRoot: " << config.isRoot
                 << "sort: " << config.sort << "name[zh_CN]" << config.name_zh_CN << "name[bo_CN]" << config.name_bo_CN
                 << "name[en_AU]" << config.name_en_AU << "comment[zh_CN]" << config.comment_zh_CN << "comment[bo_CN]"
                 << config.comment_bo_CN << "comment[en_AU]" << config.comment_en_AU << "icon" << config.icon
                 << "defaultIcon" << config.defaultIcon << "exec" << config.exec;
    }

    return;
}

Theme Utils::getTheme(void)
{
    QString theme = kdk::GsettingMonitor::getSystemTheme().toString();
    if (theme == "ukui-dark" || theme == "ukui-black") {
        return Theme::Dark;
    } else {
        return Theme::Light;
    }

    return Theme::Default;
}

double Utils::getFontSize(void)
{
    return kdk::GsettingMonitor::getSystemFontSize().toDouble();
}

QString Utils::getSelfAdaptionText(QWidget *w, QString text)
{
    QFontMetrics fontMetrics(w->font());
    int fontWidth = fontMetrics.width(text);
    if (fontWidth > w->width()) {
        return fontMetrics.elidedText(text, Qt::ElideRight, w->width());
    }

    return "";
}
