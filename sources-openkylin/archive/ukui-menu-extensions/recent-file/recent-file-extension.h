/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef UKUI_MENU_RECENT_FILE_EXTENSION_H
#define UKUI_MENU_RECENT_FILE_EXTENSION_H

#include <QMenu>
#include <QPointer>

class QThread;

#include <ukui-menu/menu-extension-plugin.h>
#include <ukui-menu/widget-extension.h>

namespace UkuiMenu {

class RecentFilesModel;
class RecentFileProvider;

/**
 * @brief 最近文件项数据结构，表示单个最近访问文件的信息
 */
class RecentFile
{
public:
    quint64 accessTime{0};
    QString uri;
    QString name;
    QString icon;
    QString infoId;
};

class RecentFileExtensionPlugin : public MenuExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_MENU_EXTENSION_I_FACE_TYPE FILE "metadata.json")
    Q_INTERFACES(UkuiMenu::MenuExtensionPlugin)
public:
    ~RecentFileExtensionPlugin() override;
    QString id() override;
    WidgetExtension *createWidgetExtension() override;
    ContextMenuExtension *createContextMenuExtension() override;
};

class RecentFileExtension : public WidgetExtension
{
    Q_OBJECT
public:
    explicit RecentFileExtension(QObject *parent = nullptr);
    ~RecentFileExtension() override;

    MetadataMap metadata() const override;
    int index() const override;
    QVariantMap data() override;
    void receive(const QVariantMap &data) override;

private:
    MetadataMap m_metadata;
    QPointer<QMenu> m_contextMenu;
    QVector<RecentFile> m_recentFile;
    QVariantMap         m_data;
    RecentFilesModel   *m_recentFilesModel = nullptr;
    QThread            *m_recentFilesProviderThread = nullptr;
    RecentFileProvider *m_recentFileProvider = nullptr;

    void openFile(const QString& fileUrl);
    void creatContextMenu(const QString &path, const int &index);
    void creatEmptyAreaContextMenu();

    QString getDefaultAppForUrl(const std::string &url);
    QIcon getAppIcon(const QString &desktoFile);

Q_SIGNALS:
    void loadRecentFiles();
    void openFileASync(const QString &path);
};

} // UkuiMenu

#endif //UKUI_MENU_RECENT_FILE_EXTENSION_H
