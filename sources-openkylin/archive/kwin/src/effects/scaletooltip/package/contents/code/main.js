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

const blacklist = [
    "hipstray hipstray", //火绒安全-安全提示
];

var scaleTooltipEffect = {
    loadConfig: function (window) {
        scaleTooltipEffect.inDuration = effect.readConfig("inDuration", 200) || 200;
        scaleTooltipEffect.outDuration = effect.readConfig("outDuration", 150) || 150;
        scaleTooltipEffect.inScale = effect.readConfig("InScale", 0.85);
        scaleTooltipEffect.outScale = effect.readConfig("OutScale", 0.85);
        scaleTooltipEffect.inOpacity = effect.readConfig("InOpacity", 0.0);
        scaleTooltipEffect.outOpacity = effect.readConfig("OutOpacity", 0.0);
    },

    setupForcedRoles: function (window) {
        window.setData(Effect.WindowForceBackgroundContrastRole, true);
        window.setData(Effect.WindowForceBlurRole, true);
    },
    cleanupForcedRoles: function (window) {
        window.setData(Effect.WindowForceBackgroundContrastRole, null);
        window.setData(Effect.WindowForceBlurRole, null);
    },
    slotWindowAdded: function (window) {
        if (!(window.tooltip && window.visible)) {
            return;
        }
        if (blacklist.indexOf(window.windowClass) != -1) {
            return;
        }
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!effect.grab(window, Effect.WindowAddedGrabRole)) {
            return;
        }
        scaleTooltipEffect.setupForcedRoles(window);
        window.scaleInAnimation = animate({
            window: window,
            duration: scaleTooltipEffect.inDuration,
            animations: [
                {
                    type: Effect.Scale,
                    curve: Effect.WindowAddedScaleCurve,
                    from: scaleTooltipEffect.inScale
                },
                {
                    type: Effect.Opacity,
                    curve: Effect.WindowAddedOpacityCurve,
                    from: scaleTooltipEffect.inOpacity
                }
            ]
        });
    },
    slotWindowClosed: function (window) {
        if (!(window.tooltip && window.visible)) {
            return;
        }
        if (blacklist.indexOf(window.windowClass) != -1) {
            return;
        }
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        if (!effect.grab(window, Effect.WindowClosedGrabRole)) {
            return;
        }
        if (window.scaleInAnimation) {
            cancel(window.scaleInAnimation);
            delete window.scaleInAnimation;
        }
        scaleTooltipEffect.setupForcedRoles(window);
        window.scaleOutAnimation = animate({
            window: window,
            duration: scaleTooltipEffect.outDuration,
            animations: [
                {
                    type: Effect.Scale,
                    curve: Effect.WindowClosedScaleCurve,
                    to: scaleTooltipEffect.outScale
                },
                {
                    type: Effect.Opacity,
                    curve: Effect.WindowClosedOpacityCurve,
                    to: scaleTooltipEffect.outOpacity
                }
            ]
        });
    },
    slotWindowDataChanged: function (window, role) {
        if (role == Effect.WindowAddedGrabRole) {
            if (window.scaleInAnimation && effect.isGrabbed(window, role)) {
                cancel(window.scaleInAnimation);
                delete window.scaleInAnimation;
                scaleTooltipEffect.cleanupForcedRoles(window);
            }
        } else if (role == Effect.WindowClosedGrabRole) {
            if (window.scaleOutAnimation && effect.isGrabbed(window, role)) {
                cancel(window.scaleOutAnimation);
                delete window.scaleOutAnimation;
                scaleTooltipEffect.cleanupForcedRoles(window);
            }
        }
    },
    init: function () {
        scaleTooltipEffect.loadConfig();

        effect.configChanged.connect(scaleTooltipEffect.loadConfig);
        effect.animationEnded.connect(scaleTooltipEffect.cleanupForcedRoles);
        effects.windowAdded.connect(scaleTooltipEffect.slotWindowAdded);
        effects.windowClosed.connect(scaleTooltipEffect.slotWindowClosed);
        effects.windowGeometryShapeChanged.connect(scaleTooltipEffect.slotWindowAdded);
        effects.windowDataChanged.connect(scaleTooltipEffect.slotWindowDataChanged);
    }
};

scaleTooltipEffect.init();
