/********************************************************************
 This file is part of the KDE project.

 Copyright (C) 2023 zhangshoucheng <zhangshoucheng@kylinos.cn>

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

function shakeAnimation(window) {
    var windowRect = window.geometry;
    window.modalShakeAnimation = animate({
        window: window,
        curve: Effect.ModalShakeCurve,
        duration: 100,
        animations: [{
            type: Effect.Position,
            to: {
                value1: windowRect.x + windowRect.width / 2 + 10,
                value2: windowRect.y + windowRect.height / 2
            },
            from: {
                value1: windowRect.x + windowRect.width / 2 - 10,
                value2: windowRect.y + windowRect.height / 2
            }
        }]
    });
}

var modalShakeEffect = {
    duration: animationTime(120),
    loadConfig: function () {
        modalShakeEffect.duration = animationTime(120);
    },
    modalShake: function (window) {
        shakeAnimation(window);
    },
    windowClose: function (window) {
        if (!window.modalShakeEffect) {
            return;
        }
        if (redirect(window.modalShakeEffect, Effect.Backward)) {
            return;
        }
        cancel(window.modalShakeAnimation);
        delete window.modalShakeEffect;
    },
    restoreForceBlurState: function(window) {
        window.setData(Effect.WindowForceBlurRole, null);
    },
    init: function () {
        effect.configChanged.connect(modalShakeEffect.loadConfig);
        effects.modalShake.connect(modalShakeEffect.modalShake);
        effects.windowClosed.connect(modalShakeEffect.windowClose)
        effect.animationEnded.connect(modalShakeEffect.restoreForceBlurState);
    }
};
modalShakeEffect.init();