/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.12

Item {
    property string visibleItem: ""
    property point mousePos: Qt.point(-1, -1)
    property double curWidW: 0
    property double curWidH: 0
    property point mouseCurPos: Qt.point(-1, -1)
    property bool navigatorState: false
    signal needShowNavitor()
    //缩放比例常数
    property int scalingconstant: 100
    property int imageAniDuration: 300
    property int imageZoomSpeed: 20
    property bool changImageUseAnimation: false
    //4个角度
    property int angle90: 90
    property int angle180: 180
    property int angle270: 270
    property int angle360: 360
    property int operateWay: 0
    property int operateImageWay:12
    property string format: ""
    property bool wheelChange: false
    property var signPaintList: []

    property int painterType : -1                    //记录画笔类型
    property int painterThickness : 12               //记录画笔粗细
    property color painterColor : Qt.rgba(255,0,0)   //记录画笔颜色
    property bool isBold : false;                    //加粗
    property bool isDeleteLine : false               //删除
    property bool isUnderLine :false                 //下划线
    property bool isItalics : false                  //斜体
    property string fontType : "Monospace"           //字体
    property int fontSize : 12                       //字号

    property var objList:[]
    property int operateTime: 0

    property double mouseqX: 0
    property double mouseqY: 0
    property double acturalWidth: 0
    property double acturalHeight: 0
    property double acturalX: 0
    property double acturalY: 0
    property double acturalRotate: 0

    property bool painterRelease: false
    property bool coreNeedPaint: false
    property bool needExitPaint: false
    signal painterRectSignal(var rect)
    signal painterCircleSignal(var leftX, var leftY, var painterW, var painterH)
    signal painterLinePaint(var startPos, var endPos);
    signal painterArrowPaint(var startPos, var endPos, var arrowStartPos, var arrowEndPos);
    signal painterPencilPaint(var pointList)
    signal painterMarkPaint(var startPos, var endPos,var alpha)
    signal painterTextPaint(var textContent, var startPos, var type)
    signal painterBlurPaint(var blurRect)

    property bool createNewTextobj: false
    signal painterTextChanged(var mx,var my)
    property double textMouseX: -1000
    property double textMouseY: -1000
    property bool signMouseStart: false
}
