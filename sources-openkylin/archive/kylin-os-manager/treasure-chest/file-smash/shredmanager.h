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

#include <QObject>
#include <QString>
#include "plugininterface.h"
#include "shreddialog.h"

//插件入口
class ShredManager : public QObject, PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "com.kylin.Plugin.PluginInterface" FILE "shred.json") //指定IID和.json文件
#endif

public:
    explicit ShredManager(QObject *parent = 0);
    virtual ~ShredManager();
    QWidget *centralWidget();

public slots:

public:
    virtual QString getGuid();
    virtual QString getName();
    virtual QString getDescribe();
    virtual QString getPicture();
    virtual void doAction();

private:
    ShredDialog *shred_dialog = nullptr;
};
