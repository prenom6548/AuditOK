/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */
import QtQuick 2.0
import org.ukui.sidebar.items 1.0

Rectangle {
    property bool disable: false;
    property string iconName: "";
    property int paletteRole: PaletteRole.Button;  //枚举的颜色值

//    signal clicked();
//    signal longClicked();
//    signal entered();
//    signal exited();

//    onColorRoleChanged: {
//        dataPrivate.restore();
//    }

//    Component.onCompleted: {
//        updateComponentColor();
//        // 链接信号用于跟随主题改变颜色
//        colorHelper.styleColorChanged.connect(updateComponentColor);
//    }

//    Component.onDestruction: {
//        colorHelper.styleColorChanged.disconnect(updateComponentColor);
//    }

//    function updateComponentColor() {
//        dataPrivate.restore();
//    }

//    QtObject {
//        id: dataPrivate;

//        property int backupColorRole: null;

//        function hightLight() {
//            backupColorRole = ColorRole.HighLight;
//            buttonIcon.highLight = colorHelper.isHighLightColor(backupColorRole);
//            root.color = colorHelper.parseColor(backupColorRole);
//        }

//        function restore() {
//            backupColorRole = root.colorRole;
//            buttonIcon.highLight = colorHelper.isHighLightColor(backupColorRole);
//            root.color = colorHelper.parseColor(backupColorRole);
//        }

//        function darker() {
//            hightLight();
//            root.color = Qt.darker(root.color, 1.5);
//        }
//    }

//    IconItem {
//        id: buttonIcon;

//        //将图标居中显示在按钮中
//        width: Math.floor(Math.min(parent.width, parent.height) / 2);
//        height: width;
//        anchors.centerIn: parent;
//        source: root.iconName;
//        disable: root.disable;
//    }

//    MouseArea {
//        anchors.fill: parent;
//        hoverEnabled: true;

//        onPressAndHold: {
//            dataPrivate.restore();
//            root.longClicked();
//        }

//        onClicked: {
//            root.clicked();
//        }

//        onCanceled: {
//            dataPrivate.restore();
//        }

//        onPressed: {
//            dataPrivate.darker();
//        }

//        onReleased: {
//            dataPrivate.restore();
//        }

//        onEntered: {
//            dataPrivate.hightLight();
//            root.entered();
//        }

//        onExited: {
//            dataPrivate.restore();
//            root.exited();
//        }
//    }
}
