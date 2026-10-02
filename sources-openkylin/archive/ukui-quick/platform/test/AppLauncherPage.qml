/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.15
import org.ukui.quick.platform 1.0 as PlatForm
import org.ukui.quick.items 1.0

Column {
    id: root
    Row {
        width: 900
        height: 50
        spacing: 4
        TextInputWidget {
            id: desktopFileText
            width: 400
            height: parent.height
            focus: true
        }
        TextInputWidget {
            id: launchAppArg
            width: 100
            height: parent.height
        }
        AppLauncherButton {
            text: "launchApp"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.launchApp(desktopFileText.text);
            }
        }
        AppLauncherButton {
            text: "launchAppWithArgs"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.launchAppWithArguments(desktopFileText.text, launchAppArg.text.split(' '));
            }
        }
    }
    Row {
        width: 900
        height: 50
        spacing: 4
        TextInputWidget {
            id: uriText
            width: 400
            height: parent.height
            focus: true
        }
        AppLauncherButton {
            text: "openUri"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.openUri(uriText.text, "");
            }
        }
    }
}