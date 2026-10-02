/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    StylePrivate.ParseColorInterface
    {
        id: pf
    }

    function pStartColor(dtColor)
    {
        return pf.startColor(dtColor)
    }
    function pEndColor(dtColor)
    {
       return pf.endColor(dtColor)
    }
    function isSolidPattern(dtColor)
    {
        return pf.isSolidPattern(dtColor)
    }

    function iconSource(controlRoot)
    {
        if((controlRoot.icon.source.toString() === "" && controlRoot.icon.name.toString() === "")){
            return controlRoot.icon.source
        }
        var closeBtn = false;
        var windowBtn = false;
        var importBtn = false;
        if(controlRoot.hasOwnProperty("buttonType")){
            closeBtn = (controlRoot.buttonType === "closeButton" || controlRoot.buttonType === "windowTitleBarCloseButton");
            windowBtn = (controlRoot.buttonType === "windowButton" || controlRoot.buttonType === "windowTitleBarWindowButton");
            importBtn = (controlRoot.buttonType === "importButton");
        }
        var model = "";
        var active = Qt.application.active
        if(controlRoot.hasOwnProperty("buttonType") && (controlRoot.buttonType === "windowTitleBarCloseButton" ||
                                                        controlRoot.buttonType === "windowTitleBarWindowButton")){
            if(!active && !(controlRoot.hovered || controlRoot.pressed)){
                if(appP.isDark)
                    model = "/highDisbale"
                else
                    model =  "/disenable"
            }
        }
        /*if(!controlRoot.enabled && )
            model =  "/disenable"
        else */
        if(model === ""){
            if(!controlRoot.flat && (importBtn || controlRoot.highlighted || appP.isDark))
                model =  "/highlight"
            else if((controlRoot.flat && appP.isDark )||
                    ((closeBtn || (controlRoot.flat &&(importBtn || controlRoot.highlighted || appP.isDark))) &&
                     (controlRoot.pressed || controlRoot.hovered)))

            {
                model =  "/highlight"
            }
            else if(!controlRoot.enabled)
                model =  "/disenable"
            else if(model === "" && controlRoot.enabled)
                model = "/normal"
        }

        if(model == "/highlight" && !controlRoot.enabled)
            model = "/highDisbale"

        let s = controlRoot.icon.source.toString();
        if(s === "")
            s = controlRoot.icon.name.toString();
        var lastS =pf.endString(s, "/");
        if(lastS === "disenable" || lastS === "highlight" || lastS === "normal" || lastS === "highDisbale")
            model = "";

        //var prefix = /^image:///imageProvider///i;
        var prefixStr = "image://imageProvider/";
        //            var qrcStr = "qrc:/";
        //            console.log("start width...", s.slice(0, 5), s.slice(0, prefixStr.length))
        if(s.slice(0, 5) === "qrc:/" || s.slice(0, 5) === "file:")
        {
            return prefixStr + s + model;
        }

        if(s.slice(0, prefixStr.length).toLowerCase() === prefixStr.toLowerCase()){
            prefixStr = "";
        }
//                console.log("buttoniconlabel...", prefixStr + s + model)
        return prefixStr + s + model;
    }

    function buttonIcon(controlRoot, iconName){
        var closeBtn = false;
        var windowBtn = false;
        var importBtn = false;
        if(controlRoot.hasOwnProperty("buttonType")){
            closeBtn = (controlRoot.buttonType === "closeButton");
            windowBtn = (controlRoot.buttonType === "windowButton");
            importBtn = (controlRoot.buttonType === "importButton");
        }
        var model = "";
        /*if(!controlRoot.enabled && )
            model =  "/disenable"
        else */
        if(!controlRoot.flat && (importBtn || controlRoot.highlighted || appP.isDark))
            model =  "/highlight"
        else if((controlRoot.flat && appP.isDark )|| (closeBtn || (controlRoot.flat &&(importBtn || controlRoot.highlighted || appP.isDark))) &&(controlRoot.pressed || controlRoot.hovered))
        {
            model =  "/highlight"
        }
        else if(!controlRoot.enabled)
            model =  "/disenable"
        else if(controlRoot.enabled)
            model = "/normal"

        if(model == "/highlight" && !controlRoot.enabled)
            model = "/highDisbale"

        let s = iconName
        var lastS =pf.endString(s, "/");
        if(lastS === "disenable" || lastS === "highlight" || lastS === "normal" || lastS === "highDisbale")
            model = "";

        //var prefix = /^image:///imageProvider///i;
        var prefixStr = "image://imageProvider/";
        //            var qrcStr = "qrc:/";
        //            console.log("start width...", s.slice(0, 5), s.slice(0, prefixStr.length))
        if(s.slice(0, 5) === "qrc:/" || s.slice(0, 5) === "file:")
        {
            return prefixStr + s + model;
        }

        if(s.slice(0, prefixStr.length).toLowerCase() === prefixStr.toLowerCase()){
            prefixStr = "";
        }
        //        console.log("buttoniconlabel...", prefixStr + s + model)
        return prefixStr + s + model;

    }

    function kyIconSource(controlRoot)
    {
        let s = controlRoot.icon.source.toString();
        if(s === "")
            s = controlRoot.icon.name.toString();
        return s
    }

    function getDTColor(dtColor){
        return pf.getDtColor(dtColor);
    }
}
