/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

/* Add shadow for a round button:
Button {
    id: roundButton
    width: 100
    height: 100
    background.radius: 50
}
DtDropShadow {
    anchors.fill: roundButton
    shadowRadius: 10
    z: -1
    shadowData: ShadowData {
        xOffset: 0
        yOffset: 0
        blurRadius: 50
        spread: 0
        shadowColor: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 1)
        childShadow: ShadowData {
            xOffset: 0
            yOffset: 12
            blurRadius: 25
            spread: 0
            shadowColor: "blue"
        }
    }
}
 */
import QtQuick 2.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype DtDropShadow
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供 DT 主题风格的阴影效果组件。

    DtDropShadow 是一个用于为其他组件添加阴影效果的组件，支持多层阴影叠加。
    它基于 ShadowedRectangle 实现，可以通过 ShadowData 配置阴影的偏移、模糊半径、
    扩散范围和颜色等属性。

    \section1 主要特性
    \list
        \li 支持单层或多层阴影效果
        \li 可配置阴影的偏移、模糊、扩散和颜色
        \li 自动计算阴影所需的空间大小
        \li 支持圆角阴影
    \endlist

    \section1 使用示例

    为圆形按钮添加阴影：
    \qml
    Button {
        id: roundButton
        width: 100
        height: 100
        background.radius: 50
    }
    DtDropShadow {
        anchors.fill: roundButton
        shadowRadius: 10
        z: -1
        shadowData: ShadowData {
            xOffset: 0
            yOffset: 0
            blurRadius: 50
            spread: 0
            shadowColor: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 1)
            childShadow: ShadowData {
                xOffset: 0
                yOffset: 12
                blurRadius: 25
                spread: 0
                shadowColor: "blue"
            }
        }
    }
    \endqml

    \sa ShadowedRectangle, DtThemeBackground
*/

Item {
    id: root
    /*!
        \qmlproperty var DtDropShadow::shadowData

        阴影数据对象，类型为 ShadowData。

        该属性定义了阴影的各项参数，包括偏移、模糊半径、扩散范围和颜色。
        支持通过 childShadow 属性设置多层阴影效果。
    */
    property var shadowData
    /*!
        \qmlproperty int DtDropShadow::shadowSize
        \readonly

        阴影占用的最大空间大小（只读）。

        该值根据所有阴影层的偏移、模糊半径和扩散范围自动计算得出，
        用于确定阴影效果所需的额外空间。
    */
    property int shadowSize: 0
    /*!
        \qmlproperty int DtDropShadow::shadowRadius

        阴影的圆角半径。

        该属性应与被添加阴影的组件的圆角半径保持一致，以确保阴影形状正确。
        默认值为 0。
    */
    property int shadowRadius: 0
    /*!
        \qmlproperty var DtDropShadow::activeShadows

        当前激活的阴影对象数组（内部使用）。

        该属性存储所有动态创建的 ShadowedRectangle 实例，用于管理多层阴影。
    */
    property var activeShadows: []

    Component.onCompleted: {
        updateShadow();
    }

    onShadowDataChanged: {
        updateShadow();
    }

    Component {
        id: shadowCompontent
        ShadowedRectangle {
            anchors.fill: root
            color: "transparent"
            radius: shadowRadius
            property var shadowRectData

            shadow {
                size: shadowRectData ? shadowRectData.blurRadius + shadowRectData.spread : 0
                color: shadowRectData ? shadowRectData.shadowColor : "black"
                xOffset: shadowRectData ? shadowRectData.xOffset : 0
                yOffset: shadowRectData ? shadowRectData.yOffset : 0
            }
        }
    }

    function updateShadow() {
        clearShadow();

        let currentShadow = shadowData;
        let maxSize = 0;
        while (currentShadow) {
            let instance = shadowCompontent.createObject(root, {shadowRectData: currentShadow});
            if (instance) {
                activeShadows.push(instance);
            }

            let offset = Math.max(currentShadow.xOffset, currentShadow.yOffset);
            maxSize = Math.max(maxSize, (offset + currentShadow.blurRadius + currentShadow.spread));

            if (currentShadow.isMultiShadow) {
                currentShadow = currentShadow.childShadow;
            } else {
                currentShadow = null;
            }
        }
        shadowSize = maxSize;
    }
    function clearShadow() {
        activeShadows.forEach(obj => {
            obj.destroy()
        } );
        activeShadows = [];
    }
}
