/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

"use strict";

const whitelist = [
    "ukui-menu ukui-menu",
    "ukui-menu org.ukui.ukui-menu"
];

var Top = 1, Right = 3, Bottom = 5, Left = 7; // KWindowSystem NET::Direction

var slideWindowEffect = {
    loadConfig: function () {
        slideWindowEffect.inDuration = animationTime(300);
        slideWindowEffect.outDuration = animationTime(300);
    },
    slotWindowAdded: function (window) {
        if (whitelist.indexOf(window.windowClass) == -1) {
            return;
        }
        if (window.popupMenu) {
            return;
        }
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!window.visible) {
            return;
        }
        if (!effect.grab(window, Effect.WindowAddedGrabRole)) {
            return;
        }
        window.setData(Effect.WindowForceBlurRole, true);
        var x = 0, y = 0, d = slideDirection();
        if (d == Top) {
            y = window.height;
        } else if (d == Bottom) {
            y = -window.height;
        } else if (d == Left) {
            x = window.width;
        } else if (d == Right) {
            x = -window.width;
        }
        window.inAnimation = animate({
            window: window,
            curve: Effect.WindowAddedSlideCurve,
            duration: slideWindowEffect.inDuration,
            type: Effect.Translation,
            from: {
                value1: x,
                value2: y
            },
            to: {
                value1: 0,
                value2: 0
            }
        });
    },
    slotWindowClosed: function (window) {
        if (whitelist.indexOf(window.windowClass) == -1) {
            return;
        }
        if (window.popupMenu) {
            return;
        }
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!window.visible) {
            return;
        }
        if (!effect.grab(window, Effect.WindowClosedGrabRole)) {
            return;
        }
        window.setData(Effect.WindowForceBlurRole, true);
        var x = 0, y = 0, d = slideDirection();
        if (d == Top) {
            y = window.height;
        } else if (d == Bottom) {
            y = -window.height;
        } else if (d == Left) {
            x = window.width;
        } else if (d == Right) {
            x = -window.width;
        }
        window.outAnimation = animate({
            window: window,
            curve: Effect.WindowClosedSlideCurve,
            duration: slideWindowEffect.outDuration,
            type: Effect.Translation,
            from: {
                value1: 0,
                value2: 0
            },
            to: {
                value1: x,
                value2: y
            }
        });
    },
    slotWindowDataChanged: function (window, role) {
        if (role == Effect.WindowAddedGrabRole) {
            if (window.inAnimation && effect.isGrabbed(window, role)) {
                cancel(window.inAnimation);
                delete window.inAnimation;
                window.setData(Effect.WindowForceBlurRole, null);
            }
        } else if (role == Effect.WindowClosedGrabRole) {
            if (window.outAnimation && effect.isGrabbed(window, role)) {
                cancel(window.outAnimation);
                delete window.outAnimation;
                window.setData(Effect.WindowForceBlurRole, null);
            }
        }
    },
    init: function () {
        slideWindowEffect.loadConfig();

        effect.configChanged.connect(slideWindowEffect.loadConfig);
        effects.windowAdded.connect(slideWindowEffect.slotWindowAdded);
        effects.windowClosed.connect(slideWindowEffect.slotWindowClosed);
        effects.windowDataChanged.connect(slideWindowEffect.slotWindowDataChanged);
    }
};

slideWindowEffect.init();
