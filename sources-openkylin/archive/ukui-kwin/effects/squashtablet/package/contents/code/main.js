/********************************************************************
 This file is part of the UKUI project.

 Copyright (C) 2022 zhangshoucheng <zhangshoucheng@kylinos.cn>

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

var squashtabletEffect = {
    duration: animationTime(300),
    loadConfig: function () {
        squashtabletEffect.duration = animationTime(300);
    },
    slotWindowMinimizeTablet: function (window) {
        if (effects.hasActiveFullScreenEffect) {
            return;
        }
        var windowRect = window.geometry;

        window.minimizeAnimation = animate({
            window: window,
            curve: QEasingCurve.InOutSine,
            duration: squashtabletEffect.duration,
            animations: [
                {
                    type: Effect.Size,
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    },
                    to: {
                        value1: 0.0,
                        value2: 0.0
                    }
                },
                {
                    type: Effect.Position,
                    to: {
                        value1: windowRect.x + windowRect.width / 2,
                        value2: windowRect.height
                    },
                    from: {
                        value1: windowRect.x + windowRect.width / 2,
                        value2: (windowRect.y + windowRect.height) / 2
                    }
                },
                {
                    type: Effect.Opacity,
                    from: 1.0,
                    to: 0.0
                }
            ]
        });
    },
    slotWindowClosed: function (window) {
        window.fadeOutAnimation = animate({
            window: window,
            curve: QEasingCurve.InOutSine,
            duration: squashtabletEffect.duration,
            type: Effect.Opacity,
            from: 1.0,
            to: 0.0
        });
    },
    init: function () {
        effect.configChanged.connect(squashtabletEffect.loadConfig);
        effects.tabletWindowMinimize.connect(squashtabletEffect.slotWindowMinimizeTablet);
        effects.tabletBlurMinimized.connect(squashtabletEffect.slotWindowClosed);
    }
};

squashtabletEffect.init();
