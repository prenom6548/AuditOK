/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/
"use strict";
var pcDesktop;
var tmDesktop;
var appearDesktop;
var disappearDesktop;
var currentTabletMode = false;
var scaledesktopEffect = {
    
    duration: animationTime(250),
    loadConfig: function () 
    {
        scaledesktopEffect.duration = animationTime(250);
    },

    opacitydesktop: function()
    {
        var stackingOrder = effects.stackingOrder;
        var w;
        for (var i = 0; i < stackingOrder.length; ++i) {
            w = stackingOrder[i];
            if (!w.visible || w.dock || w.caption == "" || w.caption.includes("tablet_blur_background") 
                    || w.slurpedByEyeOnScreen || w.caption.includes("UKUI Panel")) {
                continue;
            }

            if(!w.desktopWindow) {
                animate({
                    window: w,
                    animations: [{
                        type: Effect.Opacity,
                        curve: QEasingCurve.Linear,
                        duration: 3 * scaledesktopEffect.duration,
                        from: 0,
                        to: 0
                    }]
                });
            }
        }
    },

    appearAnimate: function ()
    {
        scaledesktopEffect.opacitydesktop();
        var windowRect = appearDesktop.geometry;
        animate({
            window: appearDesktop,
            duration: 2 * scaledesktopEffect.duration, 
            animations: [
            {
                type: Effect.Size,
                from: {
                    value1: windowRect.width * 0.9,
                    value2: windowRect.height * 0.9
                },
                to: {
                    value1: windowRect.width,
                    value2: windowRect.height
                }
            },
            {
                type: Effect.Opacity,
                from: 0,
                to: 1.0
            }
        ]
        }); 
    },

    disappearAnimate: function ()
    {
        var windowRect = disappearDesktop.geometry;

        animate({
            window: disappearDesktop,
            duration: 2 * scaledesktopEffect.duration, 
            
            animations: [
                {
                    type: Effect.Size,
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    },
                    to: {
                        value1: windowRect.width * 0.9,
                        value2: windowRect.height * 0.9
                    }
                },
                {
                    type: Effect.Opacity,
                    from: 1.0,
                    to: 0.0
                }
            ]
        });

        animate({
            window: appearDesktop,
            duration: 2 * scaledesktopEffect.duration, 
            
            animations: [
                {
                    type: Effect.Size,
                    from: {
                        value1: windowRect.width,
                        value2: windowRect.height
                    },
                    to: {
                        value1: windowRect.width * 0.9,
                        value2: windowRect.height * 0.9
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

    toggleAnimationSignal: function(win)
    {
        if(win.desktopWindow) {
            if(currentTabletMode) {
                if(win.caption != "ukui-tablet-desktop") {
                    effects.setDesktopHide(win.caption);
                }
            } else {
                if(win.caption == "ukui-tablet-desktop") {
                    effects.setDesktopHide(win.caption);
                }
            }
        }
    },

    scaleDesktop: function (tabletmode) {
        var stackingOrder = effects.stackingOrder;
        var w;
        for (var i = 0; i < stackingOrder.length; ++i) {
            w = stackingOrder[i];

            if (!w.visible || w.dock || w.caption == "" || w.caption.includes("tablet_blur_background")
                    || w.slurpedByEyeOnScreen || w.caption.includes("UKUI Panel")) {
                continue;
            }

            if(w.desktopWindow) {
                if(w.caption == "ukui-tablet-desktop") {
                    tmDesktop = w;
                } else {
                    pcDesktop = w;
                }
            } else {
                animate({
                    window: w,
                    animations: [{
                        type: Effect.Opacity,
                        curve: QEasingCurve.Linear,
                        duration: 2 * scaledesktopEffect.duration,
                        to: 0
                    }]
                });
            }
        }
        if(tabletmode) {
            currentTabletMode = true;
            disappearDesktop = pcDesktop;
            appearDesktop = tmDesktop;  
        } else {
            currentTabletMode = false;
            disappearDesktop = tmDesktop;
            appearDesktop = pcDesktop;    
        }
        scaledesktopEffect.disappearAnimate();
        
    },
    init: function () {
        scaledesktopEffect.loadConfig();
        effects.tabletModeChanged.connect(scaledesktopEffect.scaleDesktop);
        effects.desktopAppearAnimatestart.connect(scaledesktopEffect.appearAnimate)
        effect.animationEnded.connect(scaledesktopEffect.toggleAnimationSignal);
    }
};

scaledesktopEffect.init();
