/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0 as Platform
import ThumbnailManager 1.0
import PreviewHelper 1.0

Item {
    property bool isSpace: false
    property bool isUserAddedFile: false
    property bool opened: mainWindow.previewPageVisible
    property int rootSpacing: 24
    property int controlSpacing: 8
    property string displayName: isSpace? currentView.viewSpaceName: previewHelper.displayName

    visible: mainWindow.currentView.spaceId > 0

    anchors.fill: parent

    onDisplayNameChanged: {
        if (isSpace) {
            startPreviewSpace(currentView.spaceId);
        }
    }

    Item {
        id: rootItem
        anchors.fill: parent
        anchors.margins: rootSpacing

        ThemeIcon {
            id: previewIcon
            width: parent.width
            height: parent.width * 0.534
            anchors.top: parent.top
            //source: "window-close-symbolic"

            Component.onCompleted: {
                if (previewIcon.hasOwnProperty("keepAspectRatio"))
                    previewIcon.keepAspectRatio = true;
            }
        }

        Text {
            id: nameLabel
            Layout.maximumWidth: parent.width/2
            anchors.top: previewIcon.bottom
            anchors.left: parent.left
            anchors.right: relatedFilesBox.visible? relatedFilesBox.left: parent.right
            anchors.topMargin: rootSpacing
            elide: Text.ElideRight
            text: displayName
        }

        ComboBox {
            id: relatedFilesBox
            Layout.maximumWidth: parent.width/2
            anchors.right: parent.right
            anchors.verticalCenter: nameLabel.verticalCenter
            anchors.topMargin: rootSpacing
            visible: nameLabel.visible && opened && x > parent.x && false
            model : [qsTr("Related Files"), "1", "2", "3"]
        }

        ThemeSeparator {
            id: separator1
            anchors.top: nameLabel.bottom
            anchors.topMargin: controlSpacing
            width: parent.width
        }

        DisabledText {
            anchors.top: separator1.bottom
            anchors.topMargin: controlSpacing
            visible: !isSpace
            id: locationText
            text: qsTr("File Location")
        }

        DisabledText {
            id: locationTextContent
            Layout.maximumWidth: parent.width/2
            anchors.top: locationText.top
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
            anchors.left: locationText.right
            anchors.leftMargin: 2
            elide: Text.ElideRight
            maximumLineCount: 3
            text: previewHelper.filePath
            visible: locationText.visible && opened
        }

        DisabledText {
            id: accessTimeText
            anchors.top: locationText.visible? locationTextContent.bottom: separator1.bottom
            anchors.topMargin: controlSpacing
            text: isSpace? qsTr("Create Time"): qsTr("Access Time")
        }

        DisabledText {
            anchors.verticalCenter: accessTimeText.verticalCenter
            maximumLineCount: 1
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
            anchors.left: accessTimeText.right
            anchors.leftMargin: 2
            elide: Text.ElideRight
            text: previewHelper.accessTime
            visible: accessTimeText.visible && opened
        }

        DisabledText {
            id: modifiedTime
            anchors.top: accessTimeText.bottom
            anchors.topMargin: controlSpacing
            text: qsTr("Modification Time")
        }

        DisabledText {
            anchors.verticalCenter: modifiedTime.verticalCenter
            maximumLineCount: 1
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
            anchors.left: modifiedTime.right
            anchors.leftMargin: 2
            anchors.right: parent.right
            text: previewHelper.modificationTime
            visible: modifiedTime.visible && opened
        }

        DisabledText {
            id: sizeText
            visible: !isSpace && !previewHelper.isFolder
            anchors.top: modifiedTime.bottom
            anchors.topMargin: controlSpacing
            text: qsTr("File Size")
        }

        DisabledText {
            anchors.verticalCenter: sizeText.verticalCenter
            maximumLineCount: 1
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
            anchors.left: sizeText.right
            anchors.leftMargin: 2
            text: previewHelper.fileSize
            elide: Text.ElideRight
            visible: sizeText.visible  && opened
        }

        DisabledText {
            id: sourceText
            visible: !isSpace && false
            anchors.top: sizeText.bottom
            anchors.topMargin: controlSpacing
            text: qsTr("File Source")
        }

        DisabledText {
            anchors.verticalCenter: sourceText.verticalCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.right: parent.right
            //fixme: file source
            visible: sourceText.visible && opened
        }

        ThemeSeparator {
            id: separator2
            width: parent.width
            anchors.top: isSpace? modifiedTime.bottom: (sizeText.visible? sizeText.bottom: modifiedTime.bottom)
            anchors.topMargin: controlSpacing
        }

        LinkButton {
            id: openButton
            anchors.top: separator2.bottom
            anchors.topMargin: controlSpacing
            text: isSpace? qsTr("Edit Conditions"): qsTr("Open File")

            onTriggered: {
                if (isSpace) {
                    modelController.editSpaceCondition(mainWindow.currentView.spaceId);
                } else {
                    modelController.openFile(previewHelper.filePath);
                }
            }
        }

        LinkButton {
            id: openDirectoryButton
            anchors.top: openButton.bottom
            anchors.topMargin: controlSpacing
            text: isSpace? qsTr("Rename"): qsTr("Open File Directory")

            onTriggered: {
                if (isSpace) {
                    modelController.requestRenameSpace(mainWindow.currentView.spaceId);
                } else {
                    modelController.openFileInDirectory(previewHelper.filePath);
                }
            }
        }

        LinkButton {
            id: removeSpaceButton
            anchors.top: openDirectoryButton.bottom
            anchors.topMargin: controlSpacing
            text: isUserAddedFile? qsTr("Remove From Space"): qsTr("Delete Space")
            visible: isSpace || isUserAddedFile

            onTriggered: {
                if (isSpace) {
                    if (mainWindow.currentView.spaceId >= 0)
                        spaceTabBar.currentItem.spaceRemoveRequest(mainWindow.currentView.spaceId);
                } else {
                    modelController.removeFileFromSpace(previewHelper.filePath, mainWindow.currentView.proxyModel);
                    startPreviewSpace(mainWindow.currentView.spaceId);
                }
            }
        }

        Component.onCompleted: {
            thumbnailManager.thumbnailFinished.connect(function(path, image, errorMessage){
                console.log(path, image, errorMessage);
                if (errorMessage === "")
                    previewIcon.source = image;
                else
                    console.log(errorMessage);
            });
        }
    }

    Button {
        id: aiAssistantButton
        visible: false
        x: parent.x + rootSpacing - width/2
        y: parent.y + rootSpacing - height/2
        width: 36
        height: 36

        ThemeIcon {
            anchors.fill: parent
            source: "/usr/share/pixmaps/AI-Writing.svg"
        }
    }

    function startPreviewFile(path, decoration, userAdded) {
        if (!visible)
            return;

        isSpace = false;
        isUserAddedFile = userAdded;

        // todo: use ukui file metadata api for generating thumbnail
        previewIcon.source = decoration;
        thumbnailManager.setStrictedMode(true);
        thumbnailManager.startPreview(path);

        previewHelper.setPreviewFile(path);
    }

    function startPreviewSpace(spaceId) {
        if (!visible)
            return;

        isSpace = true;
        isUserAddedFile = false;
        // fixme: display space icon
        previewIcon.source = "folder";

        previewHelper.setPreviewSpaceId(spaceId);
    }

    onVisibleChanged: {
        if (visible) {
            startPreviewSpace(mainWindow.currentView.spaceId);
        } else {
            previewIcon.source = "";
        }
    }
}
