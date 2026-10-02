/*
    SPDX-FileCopyrightText: 2022 Hongfei Shang <shanghongfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchtrail.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(TouchTrailEffect,
                              "metadata.json.stripped",
                              return TouchTrailEffect::supported();)

} // namespace KWin

#include "main.moc"
