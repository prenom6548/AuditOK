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

var menuCount = 0;
var popupmenuCount = 0;

var fadeCascadingMenuEffect = {
    loadConfig: function () {
        fadeCascadingMenuEffect.fadeInDuration = animationTime(220);
        fadeCascadingMenuEffect.fadeOutDuration = animationTime(180);
    },
    slotWindowAdded: function (window) {
        if (window.menu) {
            menuCount += 1;
            if (menuCount == 1) {
                return;
            }
        }
        else if (window.popupMenu) {
            popupmenuCount += 1;
            if (popupmenuCount == 1) {
                return;
            }
        }
        else {
            return;
        }
        window.fadeInAnimation = animate({
            window: window,
            curve: Effect.WindowAddedOpacityCurve,
            duration: fadeCascadingMenuEffect.fadeInDuration,
            animations: [
                {
                    type: Effect.Opacity,
                    from: 0.0,
                    to: 1.0
                },
                {
                    type: Effect.Position,
                    from: {
                        value1: window.x + window.width / 2,
                        value2: window.y + window.height / 2 + 6
                    },
                    to: {
                        value1: window.x + window.width / 2,
                        value2: window.y + window.height / 2
                    }
                }
            ]
        });
    },
    slotWindowClosed: function (window) {
        if (window.menu) {
            menuCount -= 1;
            if (menuCount == 0) {
                return;
            }
        }
        else if (window.popupMenu) {
            popupmenuCount -= 1;
            if (popupmenuCount == 0) {
                return;
            }
        }
        else {
            return;
        }
        window.fadeOutAnimation = animate({
            window: window,
            curve: Effect.WindowClosedOpacityCurve,
            duration: fadeCascadingMenuEffect.fadeOutDuration,
            animations: [
                {
                    type: Effect.Opacity,
                    from: 1.0,
                    to: 0.0
                },
                {
                    type: Effect.Position,
                    from: {
                        value1: window.x + window.width / 2,
                        value2: window.y + window.height / 2
                    },
                    to: {
                        value1: window.x + window.width / 2,
                        value2: window.y + window.height / 2 + 6
                    }
                }
            ]
        });
    },
    slotWindowDataChanged: function (window, role) {
        if (role == Effect.WindowAddedGrabRole) {
            if (window.fadeInAnimation && effect.isGrabbed(window, role)) {
                cancel(window.fadeInAnimation);
                delete window.fadeInAnimation;
            }
        } else if (role == Effect.WindowClosedGrabRole) {
            if (window.fadeOutAnimation && effect.isGrabbed(window, role)) {
                cancel(window.fadeOutAnimation);
                delete window.fadeOutAnimation;
            }
        }
    },
    init: function () {
        fadeCascadingMenuEffect.loadConfig();

        effect.configChanged.connect(fadeCascadingMenuEffect.loadConfig);
        effects.windowAdded.connect(fadeCascadingMenuEffect.slotWindowAdded);
        effects.windowClosed.connect(fadeCascadingMenuEffect.slotWindowClosed);
        effects.windowDataChanged.connect(fadeCascadingMenuEffect.slotWindowDataChanged);
    }
};

fadeCascadingMenuEffect.init();
