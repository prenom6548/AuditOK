/********************************************************************
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

var squashEffect = {
    duration: animationTime(260),
    loadConfig: function () {
        squashEffect.duration = animationTime(260);
    },
    slotWindowMinimized: function (window) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }

        // If the window doesn't have an icon in the task manager,
        // don't animate it.
        var iconRect = window.iconGeometry;
        if (iconRect.width == 0 || iconRect.height == 0) {
            return;
        }

        if (window.unminimizeAnimation) {
            if (redirect(window.unminimizeAnimation, Effect.Backward)) {
                return;
            }
            cancel(window.unminimizeAnimation);
            delete window.unminimizeAnimation;
        }

        if (window.minimizeAnimation) {
            if (redirect(window.minimizeAnimation, Effect.Forward)) {
                return;
            }
            cancel(window.minimizeAnimation);
        }

        var windowRect = window.geometry;

        window.minimizeAnimation = animate({
            window: window,
            duration: squashEffect.duration,
            animations: [
                {
                    type: Effect.Size,
                    curve: Effect.MinimizeSizeCurve,
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    },
                    to: {
                        value1: iconRect.width,
                        value2: iconRect.height
                    }
                },
                {
                    type: Effect.Translation,
                    curve: Effect.MinimizeSizeCurve,
                    from: {
                        value1: 0.0,
                        value2: 0.0
                    },
                    to: {
                        value1: iconRect.x - windowRect.x -
                            (windowRect.width - iconRect.width) / 2,
                        value2: iconRect.y - windowRect.y -
                            (windowRect.height - iconRect.height) / 2,
                    }
                },
                {
                    type: Effect.Opacity,
                    curve: Effect.MinimizeOpacityCurve,
                    from: 1.0,
                    to: 0.0
                }
            ]
        });
    },
    slotWindowUnminimized: function (window) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }

        // If the window doesn't have an icon in the task manager,
        // don't animate it.
        var iconRect = window.iconGeometry;
        if (iconRect.width == 0 || iconRect.height == 0) {
            return;
        }

        if (window.minimizeAnimation) {
            if (redirect(window.minimizeAnimation, Effect.Backward)) {
                return;
            }
            cancel(window.minimizeAnimation);
            delete window.minimizeAnimation;
        }

        if (window.unminimizeAnimation) {
            if (redirect(window.unminimizeAnimation, Effect.Forward)) {
                return;
            }
            cancel(window.unminimizeAnimation);
        }

        var windowRect = window.geometry;

        window.unminimizeAnimation = animate({
            window: window,
            duration: animationTime(300),
            animations: [
                {
                    type: Effect.Size,
                    curve: Effect.MinimizeSizeCurve,
                    from: {
                        value1: iconRect.width,
                        value2: iconRect.height
                    },
                    to: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    }
                },
                {
                    type: Effect.Translation,
                    curve: Effect.MinimizeSizeCurve,
                    from: {
                        value1: iconRect.x - windowRect.x -
                            (windowRect.width - iconRect.width) / 2,
                        value2: iconRect.y - windowRect.y -
                            (windowRect.height - iconRect.height) / 2,
                    },
                    to: {
                        value1: 0.0,
                        value2: 0.0
                    }
                },
                {
                    type: Effect.Opacity,
                    curve: Effect.MinimizeOpacityCurve,
                    from: 0.0,
                    to: 1.0
                }
            ]
        });
    },
    slotOpenWindow: function (window) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }

        // If the window doesn't have an icon in the task manager,
        // don't animate it.
        var iconRect = window.startIconGeometry;
        if (iconRect.width == 0 || iconRect.height == 0) {
            return;
        }

        // If the window is not a normal window, don't animate it.
        if (window.windowType != 0) {
            return;
        }

        var launchBy = window.startLaunchBy;
        var sizeCurve = Effect.DesktopWindowAddedSizeCurve;
        var opactiyCurve = Effect.DesktopWindowAddedOpacityCurve;
        if (launchBy == 1) {
            sizeCurve = Effect.PanelWindowAddedSizeCurve;
            opactiyCurve = Effect.PanelWindowAddedOpacityCurve;
        }

        if (window.minimizeAnimation) {
            if (redirect(window.minimizeAnimation, Effect.Backward)) {
                return;
            }
            cancel(window.minimizeAnimation);
            delete window.minimizeAnimation;
        }

        if (window.unminimizeAnimation1) {
            if (redirect(window.unminimizeAnimation1, Effect.Forward)) {
                return;
            }
            cancel(window.unminimizeAnimation1);
        }

        var windowRect = window.geometry;

        window.unminimizeAnimation1 = animate({
            window: window,
            duration: animationTime(300),
            animations: [
                {
                    type: Effect.Size,
                    curve: sizeCurve,
                    from: {
                        value1: iconRect.width,
                        value2: iconRect.height
                    },
                    to: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    }
                },
                {
                    type: Effect.Translation,
                    curve: sizeCurve,
                    from: {
                        value1: iconRect.x - windowRect.x -
                            (windowRect.width - iconRect.width) / 2,
                        value2: iconRect.y - windowRect.y -
                            (windowRect.height - iconRect.height) / 2,
                    },
                    to: {
                        value1: 0.0,
                        value2: 0.0
                    }
                },
                {
                    type: Effect.Opacity,
                    curve: opactiyCurve,
                    from: 0.0,
                    to: 1.0
                }
            ]
        });
    },
    init: function () {
        effect.configChanged.connect(squashEffect.loadConfig);
        effects.windowMinimized.connect(squashEffect.slotWindowMinimized);
        effects.windowUnminimized.connect(squashEffect.slotWindowUnminimized);
        effects.windowAdded.connect(squashEffect.slotOpenWindow);
    }
};

squashEffect.init();
