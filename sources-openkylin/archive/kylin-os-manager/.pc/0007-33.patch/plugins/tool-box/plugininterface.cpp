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

#include "plugininterface.h"
#include "toolboxwidget.h"
#include <QTranslator>
#include <QDebug>
#include <QApplication>

PluginInterface::~PluginInterface() {}

std::string PluginInterface::name()
{
    return "ToolBox";
}

std::string PluginInterface::i18nName()
{
    return QObject::tr("ToolBox").toStdString();
}

std::string PluginInterface::icon()
{
	return "ukui-full-backup-symbolic";
}

int PluginInterface::sort()
{
    return 4;
}

QWidget *PluginInterface::createWidget()
{
    QString tranPath("/usr/share/kylin-os-manager/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-tool-box", "_", tranPath)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "ProblemFeedback load translation file fail !";
    }
    ToolBoxWidget *widget = new ToolBoxWidget();
    return widget;
}

KomApplicationInterface *PluginProvider::create() const
{
    return new PluginInterface();
}
