/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AISUBSYSTEMPLUGININTERFACE_H
#define AISUBSYSTEMPLUGININTERFACE_H

#include <QWidget>
#include <QtPlugin>

class KylinAiSubSystemPluginInterface {
public:
    virtual ~KylinAiSubSystemPluginInterface() {}
    virtual QWidget *createModelConfigWidget() = 0;
    virtual QWidget *createComputePowerSharingWidget() = 0;
};

Q_DECLARE_INTERFACE(KylinAiSubSystemPluginInterface,
                    "org.kylinaisubsystem.PluginInterface")

#endif // AISUBSYSTEMPLUGININTERFACE_H
