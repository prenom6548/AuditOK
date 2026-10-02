/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kwineffects.h>

#include "multitaskviewmanager.h"

class MultitaskViewManagerPluginFactory : public KWin::EffectPluginFactory
{
    Q_OBJECT

    Q_INTERFACES(KPluginFactory)

    Q_PLUGIN_METADATA(IID EffectPluginFactory_iid FILE "windowsview.json")

public:

    MultitaskViewManagerPluginFactory() {}

    ~MultitaskViewManagerPluginFactory() override {}

    KWin::Effect* createEffect() const override
    {
        return new MultitaskViewManager();
    }

};

K_EXPORT_PLUGIN_VERSION(KWIN_EFFECT_API_VERSION)

#include "multitaskviewmanagerpluginfactory.moc"
