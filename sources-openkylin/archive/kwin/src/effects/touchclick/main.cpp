/*
    SPDX-FileCopyrightText: 2022 Hongfei Shang <shanghongfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchclick.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(TouchClickEffect,
                              "metadata.json.stripped",
                              return TouchClickEffect::supported();)
} // namespace KWin

#include "main.moc"
