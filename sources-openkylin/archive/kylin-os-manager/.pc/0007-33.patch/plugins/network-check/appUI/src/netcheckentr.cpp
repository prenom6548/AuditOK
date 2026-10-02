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

#include "netcheckentr.h"
#include "netcheckwidget.h"
#include "mainwindow.h"
#include <QTranslator>
#include "frame.h"

NetcheckEntr::~NetcheckEntr() {}

void NetcheckEntr::init(void (*frameCallback)(const char *funcName, ...))
{
    Frame::setFrameCallback(frameCallback);
}

std::string NetcheckEntr::name()
{
    return "NetworkCheck";
}

std::string NetcheckEntr::i18nName()
{
    return QObject::tr("NetworkCheck").toStdString();
}

std::string NetcheckEntr::icon()
{
	return "ukui-troubleshooting-symbolic";
}

int NetcheckEntr::sort()
{
    return 1;
}

QWidget *NetcheckEntr::createWidget()
{
    QString tranPath("/usr/share/kylin-os-manager/network-check/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-netcheck-tools", "_", tranPath)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "ProblemFeedback load translation file fail !";
    }

    NetCheckWidget *widget = new NetCheckWidget();
    return widget;
}

KomApplicationInterface *PluginProvider::create() const
{
    return new NetcheckEntr();
}
