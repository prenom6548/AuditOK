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

var popupmenuCount = 0;
var rightmenuCount = 0;

var fadeMenuEffect = {
    loadConfig: function () {
        fadeMenuEffect.fadeInDuration = animationTime(200);
        fadeMenuEffect.fadeOutDuration = animationTime(150);
    },
    slotWindowAdded: function (window) {
        if (!window.dropdownMenu && !window.comboBox && !window.popupMenu) {
            return;
        }
        if (window.popupMenu) {
            popupmenuCount += 1;
            if (isRightButtonPressed()) {
                rightmenuCount += 1;
                return;
            }
            if (popupmenuCount != 1) {
                if (rightmenuCount != 0) {
                    rightmenuCount += 1;
                }
                return;
            }
        }
        window.fadeInAnimation = animate({
            window: window,
            curve: Effect.WindowAddedOpacityCurve,
            duration: fadeMenuEffect.fadeInDuration,
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
                        value2: window.y + window.height / 2 - 16
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
        if (!window.dropdownMenu && !window.comboBox && !window.popupMenu) {
            return;
        }
        if (window.popupMenu) {
            popupmenuCount -= 1;
            if (rightmenuCount > 0) {
                rightmenuCount -= 1;
                return;
            }
            if (popupmenuCount != 0) {
                return;
            }
        }
        window.fadeOutAnimation = animate({
            window: window,
            curve: Effect.WindowClosedOpacityCurve,
            duration: fadeMenuEffect.fadeOutDuration,
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
                        value2: window.y + window.height / 2 - 16
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
        fadeMenuEffect.loadConfig();

        effect.configChanged.connect(fadeMenuEffect.loadConfig);
        effects.windowAdded.connect(fadeMenuEffect.slotWindowAdded);
        effects.windowClosed.connect(fadeMenuEffect.slotWindowClosed);
        effects.windowDataChanged.connect(fadeMenuEffect.slotWindowDataChanged);
    }
};

fadeMenuEffect.init();
