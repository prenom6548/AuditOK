/*
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "short-cut-manager.h"
#include <QDir>
#include <QJsonObject>
#include <QJsonValue>
#include <QPluginLoader>
#include <QTranslator>
#include <QCoreApplication>
#include <QDebug>

using namespace UkuiShortcut;

ShortcutManager *instance = nullptr;

class UkuiShortcut::ShortcutManagerPrivate
{
public:
    void findShortcuts(QDir pluginsDir, QString translationsDir);
    QList<Shortcut*> getShortcuts();
    QList<Shortcut*> m_allShortcuts;
};

void ShortcutManagerPrivate::findShortcuts(QDir pluginsDir, QString translationsDir)
{
    const QString pluginType(UKUI_SHORTCUT_PLUGIN_IFACE_TYPE);

    for(const QString& fileName : pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject metaData = pluginLoader.metaData().value("MetaData").toObject();
        QString type = metaData.value("Type").toString();
        QString version = metaData.value("Version").toString();

        if (type != pluginType) {
            continue;
        }

        if (version.left(3) != QString(UKUI_SHORTCUT_PLUGIN_IFACE_VERSION).left(3)) {
            qWarning() << "UKUI_SHORT_CUT version check failed:" << fileName << "version:" << version
                       << "iface version : " << UKUI_SHORTCUT_PLUGIN_IFACE_VERSION;
            continue;
        }

        QObject *obj = pluginLoader.instance();
        if (!obj) {
            continue;
        }

        UkuiShortcutPlugin *plugin = qobject_cast<UkuiShortcutPlugin *>(obj);
        if (!plugin) {
            obj->deleteLater();
            continue;
        }

        // 加载翻译
        for (const auto &trName : plugin->translations()) {
            auto translator = new QTranslator(QCoreApplication::instance());
            if (translator->load(QLocale(), trName, QStringLiteral("_"), translationsDir)) {
                // 加载翻译成功
                QCoreApplication::installTranslator(translator);
            } else {
                delete translator;
            }
        }

        Shortcut *shortcut = plugin->createShortcut();
        if (shortcut) {
            m_allShortcuts.append(shortcut);
        }

        plugin->deleteLater();
    }
}

QList<Shortcut *> ShortcutManagerPrivate::getShortcuts()
{
    return m_allShortcuts;
}

ShortcutManager *ShortcutManager::getInstance()
{
    if (!instance) {
        instance = new ShortcutManager;
    }
    return instance;
}

ShortcutManager::ShortcutManager() : d(new ShortcutManagerPrivate)
{
    qRegisterMetaType<StatusInfo>("StatusInfo");

    QDir usrDir(PLUGIN_INSTALL_DIRS);
    QString usrTranslationsDir(SHORTCUT_TRANSLATION_FILE_DIR);
    d->findShortcuts(usrDir, usrTranslationsDir);

    QDir optDir("/opt/system/lib/ukui-shortcut-plugins");
    QString optTranslationsDir("/opt/system/resource/ukui/ukui-sidebar/shortcuts/translations");
    d->findShortcuts(optDir, optTranslationsDir);
}

QList<Shortcut *> ShortcutManager::getShortcuts()
{
    return d->getShortcuts();
}

QPair<bool, QString> ShortcutManager::getWidgetInfo(const QString &widgetId)
{
    QPair<bool, QString> widgetInfo = {false, {}};
    for (const auto &shortcut : d->m_allShortcuts) {
        if (shortcut->currentStatus().getWidgetId() == widgetId) {
             widgetInfo.first = true;
             widgetInfo.second = shortcut->currentStatus().getName();
             return widgetInfo;
        }
    }
    return widgetInfo;
}

ShortcutManager::~ShortcutManager() = default;
