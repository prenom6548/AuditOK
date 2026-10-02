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

#ifndef MENUPLUGINMANAGER_H
#define MENUPLUGINMANAGER_H

#include <QObject>
#include "peony-core_global.h"
#include "menu-plugin-iface.h"
#include <QListWidgetItem>
#include <QPushButton>
#include <QMenu>
#include <QMainWindow>
#include <QPainter>
#include <QStyleOptionButton>

#include "color-pushbutton.h"

namespace Peony {

class PEONYCORESHARED_EXPORT MenuPluginManager : public QObject
{
    Q_OBJECT
public:
    bool registerPlugin(MenuPluginInterface *plugin);
    bool unregisterPlugin(MenuPluginInterface *plugin);

    static MenuPluginManager *getInstance();
    const QStringList getPluginIds();
    MenuPluginInterface *getPlugin(const QString &pluginId);
    bool insertFileSafePlugin(MenuPluginInterface *plugin);
    MenuPluginInterface *getFileSafePlugin(const QString &pluginId);
    QMap<QString, MenuPluginInterface *> getComputerViewPlugins() const;
    void close();

private:
    QHash<QString, MenuPluginInterface*> m_hash;
    QHash<QString, MenuPluginInterface*> m_fileSafeHash;
    QMap<QString, MenuPluginInterface*> m_computerViewPlugins;
    explicit MenuPluginManager(QObject *parent = nullptr);
    ~MenuPluginManager();
};

class CreateLinkInternalPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
public:
    explicit CreateLinkInternalPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony-Qt Create Link Extension");
    }
    const QString description() override {
        return tr("Create Link Menu Extension.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create link";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

class FileLabelWidget;
class PEONYCORESHARED_EXPORT FileLabelInternalMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    friend class FileLabelWidget;
public:
    explicit FileLabelInternalMenuPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony File Labels Menu Extension");
    }
    const QString description() override {
        return tr("Tag a File with Menu.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create file label";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

class LabelManagementButton : public QPushButton
{
public:
    using QPushButton::QPushButton;

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QStyleOptionButton opt;
        initStyleOption(&opt);  // 初始化按钮样式选项
        QPainter p(this);

        // 绘制按钮基础
        style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &p, this);

        // 获取内容区域
        QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
        int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);; // 固定图标大小

        // 绘制图标（左对齐）
        if (!opt.icon.isNull()) {
            QRect iconRect = contentRect;
            iconRect.setWidth(iconSize);
            // 垂直居中
            iconRect.setTop(contentRect.top() + (contentRect.height() - iconSize) / 2);
            iconRect.setHeight(iconSize);

            // 创建图标样式选项
            QStyleOptionButton iconOpt;
            iconOpt.initFrom(this);
            iconOpt.rect = iconRect;
            iconOpt.icon = opt.icon;
            iconOpt.iconSize = QSize(iconSize, iconSize);

            // 复制按钮的状态，这样样式系统就能正确处理高亮和反白
            iconOpt.state = opt.state;
            iconOpt.features = opt.features;

            // 使用样式绘制图标（会自动处理高亮、反白等效果）
            style()->drawControl(QStyle::CE_PushButtonLabel, &iconOpt, &p, this);
        }

        // 绘制文本（图标右侧）
        if (!opt.text.isEmpty()) {
            QRect textRect = contentRect;
            textRect.setLeft(contentRect.left() + iconSize + 6); // 图标右边留6像素间距
            textRect.setWidth(contentRect.width() - iconSize - 6);

            // 使用正确的 QPalette 角色
            QPalette::ColorRole textRole = isEnabled() ? QPalette::ButtonText : QPalette::WindowText;
            p.setPen(opt.palette.color(textRole));
            p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);
        }

    }
};

class FileLabelWidget : public QWidget
{
public:
    friend class FileLabelInternalMenuPlugin;
    Q_OBJECT

Q_SIGNALS:
    void changeText(const QString &text);

public Q_SLOTS:
    void clickItem(int index);

protected:
    void paintEvent(QPaintEvent *e);

private:
    explicit FileLabelWidget(const QStringList &selectionUris);
    QStringList  *m_selectionUris;
    QButtonGroup *m_colorgroup;
    ColorPushButton *m_colorbutton;
    QList<int> m_ids;
};

class CreateSharedFileLinkMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
public:
    explicit CreateSharedFileLinkMenuPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony-Qt Share File menu Extension");
    }
    const QString description() override {
        return tr("Tag  with Menu.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create share file link label";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

}

#endif // MENUPLUGINMANAGER_H
