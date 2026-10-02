/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "main-window.h"
#include <QDebug>

#include "status-info.h"

#include "progress-bar.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_centralWidget = new QWidget(this);
    this->setCentralWidget(m_centralWidget);
    m_layout = new QVBoxLayout(m_centralWidget);

    m_shortcutManager = UkuiShortcut::ShortcutManager::getInstance();
    m_shortcutPlugins = m_shortcutManager->getShortcuts();
    for(auto plugin : m_shortcutPlugins) {
        connect(plugin, &UkuiShortcut::UkuiShortcutPlugin::enableStatusChanged, [ = ](bool isEnable){
            qDebug() << plugin << "isEnable" << isEnable;
        });
//        if(!plugin->isEnable()) {
//            continue;
//        }
        UkuiShortcut::StatusInfo si = plugin->currentStatus();
        qDebug() << "find shortcut :" << plugin->pluginId();

        if (plugin->pluginType() == UkuiShortcut::UkuiShortcutPlugin::PluginType::Icon) {
            Shortcut *shortcut = new Shortcut(si.getName(), si.getIcon(), si.getColor(), this);
            connect(plugin, &UkuiShortcut::UkuiShortcutPlugin::statusChanged, shortcut, &Shortcut::statusChanged);
            connect(shortcut, &Shortcut::clicked, plugin, [=] {
                plugin->active(UkuiShortcut::UkuiShortcutPlugin::Click);
            });
            m_layout->addWidget(shortcut, 0, Qt::AlignHCenter);

        } else if(plugin->pluginType() == UkuiShortcut::UkuiShortcutPlugin::PluginType::ProgressBar){
            ProgressBar *progressBar = new ProgressBar(this);
            progressBar->setValue(si.getValue());
            progressBar->setIcon(si.getIcon());

            m_layout->addWidget(progressBar, 0, Qt::AlignHCenter);

            connect(plugin, &UkuiShortcut::UkuiShortcutPlugin::statusChanged, progressBar, [=] (const UkuiShortcut::StatusInfo &statusInfo) {
                progressBar->setValue(statusInfo.getValue());
                progressBar->setIcon(statusInfo.getIcon());
            });

            connect(progressBar, &ProgressBar::add, plugin,[=](int size) {
                plugin->setValue(size);
            });
        } else if(plugin->pluginType() == UkuiShortcut::UkuiShortcutPlugin::PluginType::MenuButton) {
            qDebug() << plugin->pluginId() <<plugin->isEnable();
            if(plugin->isEnable()) {
                qDebug() << plugin->pluginId() << "-------" << plugin->currentStatus().getName();
                qDebug() << plugin->pluginId() << "-------" << plugin->currentStatus().getIcon();
            }
        }
    }


}

MainWindow::~MainWindow()
{
}
