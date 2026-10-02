/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef CUSTOMPLUGIN_H
#define CUSTOMPLUGIN_H

#include "screensaverplugin.h"
#include <QObject>

class CustomPlugin : public QObject, ScreensaverPlugin
{
    Q_OBJECT
    //声明QT识别的唯一标识符
    Q_PLUGIN_METADATA(IID "org.ukui.screensaver.screensaver-default1.0.0")
    //声明实现的插件接口
    Q_INTERFACES(ScreensaverPlugin)
public:
    CustomPlugin(QObject *parent = 0);
    QString name() const override;
    QWidget *createWidget(bool isScreensaver, QWidget *parent) override;
    QString displayName() const override;
};

#endif // CUSTOMPLUGIN_H
