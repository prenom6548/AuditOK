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
import QtQuick.Templates 2.5 as T
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import org.ukui.qqc2style.private 1.0 as StylePrivate



Rectangle {
    id:root

    property T.AbstractButton controlRoot: root.parent
    property int _radius : 6
    property bool useGradient: (controlStartColor !== controlEndColor)
    property color controlStartColor
    property color controlEndColor
    property color borderColor
    // visualFocus 区分「键盘 Tab/Backtab 聚焦」vs「鼠标聚焦」，但无法处理「Tab 聚焦后再点击」
    // （此时焦点未离开、focusReason 仍为 Tab，visualFocus 一直为 true）。所以保留一个 focusEnable
    // 状态机：visualFocus 变 true 时置 true（键盘焦点进入），按下时置 false（点击抑制），焦点离开时置 false。
    // 焦点色由 border.color 绑定按 showFocusBorder 切换，changeColor 不再碰焦点色，避免命令式时序污染。
    property bool focusEnable: false
    property bool showFocusBorder: app.focusEnable && focusEnable
    // 键盘 Space 时由控件 Keys.onPressed 置 true（鼠标点击不触发 Keys；AbstractButton 仅 Space 激活）；
    // onPressedChanged 据此区分：keyboardPressing=false（鼠标）才抑制，松开复位。
    property bool keyboardPressing: false

    color: (gradientRec.visible  || (controlRoot.flat && !controlRoot.pressed && !controlRoot.hovered &&
                                    (!controlRoot.checkable || (controlRoot.checkable && !controlRoot.checked))))? "transparent" :controlStartColor
//    opacity: (controlRoot.flat && !controlRoot.pressed && !controlRoot.hovered &&
//              (!controlRoot.checkable || (controlRoot.checkable && !controlRoot.checked))) ? 0 : 1
    radius: _radius // 可选：设置圆角半径
    border.width:/* controlRoot.flat ? 0 :*/ (showFocusBorder ? btn.focusBorderWidth : btn.borderWidth)
    // 用绑定直接由 showFocusBorder 切换焦点色，避免依赖 changeColor() 命令式赋值的调用时机
    // （onFocusChanged 触发时 visualFocus 可能尚未更新，会把 borderColor 误写成焦点色）
    border.color: showFocusBorder ? parseInterface.pStartColor(btn.focusBorderColor) : borderColor

    Rectangle {
        id: gradientRec
        anchors.fill: parent
        radius: _radius // 可选：设置圆角半径
        border.width:parent.border.width
        border.color: parent.border.color

        gradient:  Gradient {
            id:gradient
            GradientStop { position: 0.0; color: (controlRoot.flat && !controlRoot.pressed && !controlRoot.hovered &&
                                                  (!controlRoot.checkable || (controlRoot.checkable && !controlRoot.checked))) ? "transparent" : controlStartColor }

            GradientStop { position: 1.0; color: (controlRoot.flat && !controlRoot.pressed && !controlRoot.hovered &&
                                                  (!controlRoot.checkable || (controlRoot.checkable && !controlRoot.checked))) ? "transparent" : controlEndColor }
        }
        visible: useGradient
    }

//    Rectangle{
//    anchors.fill: parent
//    radius: parent.radius
//    border.width:/* controlRoot.flat ? 0 :*/ (controlRoot.focus ? btn.focusBorderWidth : btn.borderWidth)
//    border.color: borderColor
//    color: "transparent"

//    }

    ParseInterface{
        id: parseInterface
    }
    StylePrivate.UKUIButton {
        id: btn
        onParametryChanged:{
            changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))
            //root.color = "red"
            root.update();
        }
        //property alias lRMargin: leftRightMargin
    }
    StylePrivate.APPParameter{
        id: app
        onParametryChanged:{
            changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))

            root.update();

        }
    }
    function changeColor(highlighted, enable, click, hover, focus, checked){
        var closeBtn = false;
        var windowBtn = false;
        var importBtn = false;
        var transparent = true;
        if(controlRoot.hasOwnProperty("buttonType")){
            closeBtn = (controlRoot.buttonType === "closeButton" || controlRoot.buttonType === "windowTitleBarCloseButton");
            windowBtn = (controlRoot.buttonType === "windowButton" || controlRoot.buttonType === "windowTitleBarWindowButton");
            importBtn = (controlRoot.buttonType === "importButton");
        }
        if(controlRoot.hasOwnProperty("transparent")){
            transparent = controlRoot.transparent;
        }

        if(highlighted || importBtn){
            controlStartColor = !enable ? parseInterface.pStartColor(btn.disableHBC) :
                                 click || checked ? parseInterface.pStartColor(btn.clickedHBC) :
                                 hover ? parseInterface.pStartColor(btn.hoveredHBC) :
                                         parseInterface.pStartColor(btn.normalHBC)
            controlEndColor = !enable ? parseInterface.pEndColor(btn.disableHBC) :
                                 click || checked ? parseInterface.pEndColor(btn.clickedHBC) :
                                 hover ? parseInterface.pEndColor(btn.hoveredHBC) :
                                                          parseInterface.pEndColor(btn.normalHBC)

            borderColor = !enable ? parseInterface.pStartColor(btn.disableBorderHColor) :
                                    click || checked ? parseInterface.pStartColor(btn.clickBorderHColor) :
                                    hover ? parseInterface.pStartColor(btn.hoverBorderHColor) :
                                            parseInterface.pStartColor(btn.normalBorderHColor)
        }
        else if(closeBtn){
            controlStartColor = !enable ? parseInterface.pStartColor(btn.disableCloseBC) :
                             click || checked ? parseInterface.pStartColor(btn.clickedCloseBC) :
                             hover ? parseInterface.pStartColor(btn.hoveredCloseBC) :
                                     parseInterface.pStartColor(btn.normalCloseBC)

            controlEndColor = !enable ? parseInterface.pEndColor(btn.disableCloseBC) :
                             click || checked ? parseInterface.pEndColor(btn.clickedCloseBC) :
                             hover ? parseInterface.pEndColor(btn.hoveredCloseBC) :
                                     parseInterface.pEndColor(btn.normalCloseBC)

            borderColor = !enable ? parseInterface.pStartColor(btn.disableBorderCloseColor) :
                            click || checked ? parseInterface.pStartColor(btn.clickBorderCloseColor) :
                            hover ? parseInterface.pStartColor(btn.hoverBorderCloseColor) :
                                    parseInterface.pStartColor(btn.normalBorderCloseColor)
        }
        else if(windowBtn && !transparent){
            controlStartColor = !enable ? parseInterface.pStartColor(btn.disableWindowBC) :
                             click || checked ? parseInterface.pStartColor(btn.clickedWindowBC) :
                             hover ? parseInterface.pStartColor(btn.hoveredWindowBC) :
                                     parseInterface.pStartColor(btn.normalWindowBC)

            controlEndColor = !enable ? parseInterface.pEndColor(btn.disableWindowBC) :
                             click || checked ? parseInterface.pEndColor(btn.clickedWindowBC) :
                             hover ? parseInterface.pEndColor(btn.hoveredWindowBC) :
                                     parseInterface.pEndColor(btn.normalWindowBC)

            borderColor = !enable ? parseInterface.pStartColor(btn.disableBorderWindowColor) :
                            click || checked ? parseInterface.pStartColor(btn.clickBorderWindowColor) :
                            hover ? parseInterface.pStartColor(btn.hoverBorderWindowColor) :
                                    parseInterface.pStartColor(btn.normalBorderWindowColor)
        }
        else if(windowBtn && transparent){
            controlStartColor = !enable ? parseInterface.pStartColor(btn.disableWindowTransparentBC) :
                             click || checked ? parseInterface.pStartColor(btn.clickedWindowTransparentBC) :
                             hover ? parseInterface.pStartColor(btn.hoveredWindowTransparentBC) :
                                     parseInterface.pStartColor(btn.normalWindowTransparentBC)

            controlEndColor = !enable ? parseInterface.pEndColor(btn.disableWindowTransparentBC) :
                                click || checked ? parseInterface.pEndColor(btn.clickedWindowTransparentBC) :
                                hover ? parseInterface.pEndColor(btn.hoveredWindowTransparentBC) :
                                         parseInterface.pEndColor(btn.normalWindowTransparentBC)

            borderColor = !enable ? parseInterface.pStartColor(btn.disableBorderWindowTransparentColor) :
                            click || checked ? parseInterface.pStartColor(btn.clickedBorderWindowTransparentColor) :
                            hover ? parseInterface.pStartColor(btn.hoveredBorderWindowTransparentColor) :
                                    parseInterface.pStartColor(btn.normalBorderWindowTransparentColor)
        }
        else{
            var disableStartColor       = parseInterface.pStartColor(btn.disableBC);
            var disableEndColor       = parseInterface.pEndColor(btn.disableBC);

            var clickStartColor         = parseInterface.pStartColor(btn.clickedBC);
            var clickEndColor         = parseInterface.pEndColor(btn.clickedBC);

            var normalStartColor        = (controlRoot.flat ? "transparent" : parseInterface.pStartColor(btn.normalBC));
            var normalEndColor        = (controlRoot.flat ? "transparent" : parseInterface.pEndColor(btn.normalBC));

            var hoverStartColor    = parseInterface.pStartColor(btn.hoveredBC);
            var hoverEndColor      = parseInterface.pEndColor(btn.hoveredBC);

            var disableBorderColor = parseInterface.pStartColor(btn.disableBorderColor);
            var focusBorderColor   = parseInterface.pStartColor(btn.focusBorderColor  );
            var clickBorderColor   = parseInterface.pStartColor(btn.clickBorderColor  );
            var hoverBorderColor   = parseInterface.pStartColor(btn.hoverBorderColor  );
            var normalBorderColor  = parseInterface.pStartColor(btn.normalBorderColor );

            if(transparent){
                disableStartColor     = parseInterface.pStartColor(btn.disableTransparentBC);
                disableEndColor         = parseInterface.pEndColor(btn.disableTransparentBC);

                clickStartColor         = parseInterface.pStartColor(btn.clickedTransparentBC);
                clickEndColor         = parseInterface.pEndColor(btn.clickedTransparentBC);

                normalStartColor        = (controlRoot.flat ? "transparent" : parseInterface.pStartColor(btn.normalTransparentBC));
                normalEndColor        = (controlRoot.flat ? "transparent" : parseInterface.pEndColor(btn.normalTransparentBC));

                hoverStartColor    = parseInterface.pStartColor(btn.hoveredTransparentBC);
                hoverEndColor      = parseInterface.pEndColor(btn.hoveredTransparentBC);

                disableBorderColor = parseInterface.pStartColor(btn.disableBorderTransparentBC);
                focusBorderColor   = parseInterface.pStartColor(btn.focusBorderColor  );
                clickBorderColor   = parseInterface.pStartColor(btn.clickedBorderTransparentBC  );
                hoverBorderColor   = parseInterface.pStartColor(btn.hoveredBorderTransparentBC  );
                normalBorderColor  = parseInterface.pStartColor(btn.normalBorderTransparentBC );
            }

            if(controlRoot.hasOwnProperty("disableStartColor"))
                disableStartColor = controlRoot.disableStartColor
            if(controlRoot.hasOwnProperty("disableEndColor"))
                disableEndColor = controlRoot.disableEndColor
            if(controlRoot.hasOwnProperty("clickStartColor"))
                clickStartColor = controlRoot.clickStartColor
            if(controlRoot.hasOwnProperty("clickEndColor"))
                clickEndColor = controlRoot.clickEndColor

            if(controlRoot.hasOwnProperty("normalStartColor"))
                normalStartColor = controlRoot.normalStartColor
            if(controlRoot.hasOwnProperty("normalEndColor"))
                normalEndColor = controlRoot.normalEndColor

            if(controlRoot.hasOwnProperty("hoverStartColor"))
                hoverStartColor = controlRoot.hoverStartColor
            if(controlRoot.hasOwnProperty("hoverEndColor"))
                hoverEndColor = controlRoot.hoverEndColor

            if(controlRoot.hasOwnProperty("disableBorderColor"))
                disableBorderColor = controlRoot.disableBorderColor

            if(controlRoot.hasOwnProperty("focusBorderColor"))
                focusBorderColor = controlRoot.focusBorderColor
            if(controlRoot.hasOwnProperty("clickBorderColor"))
                clickBorderColor = controlRoot.clickBorderColor

            if(controlRoot.hasOwnProperty("hoverBorderColor"))
                hoverBorderColor = controlRoot.hoverBorderColor
            if(controlRoot.hasOwnProperty("normalBorderColor"))
                normalBorderColor = controlRoot.normalBorderColor


            controlStartColor = !enable ? disableStartColor :
                             click || checked ? clickStartColor :
                             hover ? hoverStartColor :
                                     normalStartColor

            controlEndColor = !enable ? disableEndColor :
                             click || checked? clickEndColor :
                             hover ? hoverEndColor :
                                     normalEndColor

            borderColor = !enable ? disableBorderColor :
                            click || checked ? clickBorderColor :
                            hover ? hoverBorderColor :
                                    normalBorderColor
        }

    }

    Component.onCompleted: {
        changeColor(controlRoot.highlighted, controlRoot.enabled, false, false, false, (controlRoot.checkable && controlRoot.checked));
    }

    Connections {
        target: controlRoot
        // visualFocus 变化时同步 focusEnable：键盘焦点进入→true，离开→false
        function onVisualFocusChanged() {
            focusEnable = controlRoot.visualFocus
        }
        function onPressedChanged() {
            // 鼠标点击抑制焦点框：keyboardPressing=true 表示键盘 Space（不抑制），
            // 鼠标点击不触发 Keys → keyboardPressing=false → 抑制。松开复位。
            if (controlRoot.pressed && !keyboardPressing)
                focusEnable = false
            if (!controlRoot.pressed)
                keyboardPressing = false
            changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))
        }
        function onHoveredChanged() {
            changeColor(controlRoot.highlighted, controlRoot.enabled, false, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))
        }
        function onFocusChanged(){
            changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))
        }
        function onCheckedChanged(){
            changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, showFocusBorder,
                        (controlRoot.checkable && controlRoot.checked))

        }
    }
}

