/*
 * Peony-Qt
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
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#ifndef COMMON_H
#define COMMON_H

#define PANEL_SETTINGS "org.ukui.panel.settings"
#define UKUI_STYLE_SETTINGS "org.ukui.style"

#define SCREEN_ID "metadata::peony-qt-desktop-id"
#define ITEM_POS_ATTRIBUTE "metadata::peony-qt-desktop-item-position"
#define RESTORE_ITEM_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-item-position"
#define RESTORE_EXTEND_ITEM_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-extend-item-position"
#define RESTORE_SINGLESCREEN_ITEM_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-singlescreen-item-position"
#define ORIGANIZATION_SPACE_INSTANCEID "metadata::peony-qt-desktop-item-space-instanceid"
#define ORIGANIZATION_SPACE_INSTANCEID_UNORDER "metadata::peony-qt-desktop-item-space-instanceid-unorder"
#define ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM "metadata::peony-qt-desktop-item-space-instanceid-mergefrom"
#define ORIGANIZATION_MANUAL_CREATE_PARTITION "metadata::peony-qt-desktop-manual-create-partition"
#define ITEM_GRID_POS_ATTRIBUTE "metadata::peony-qt-desktop-item-grid-pos"
#define RESTORE_ITEM_GRID_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-item-grid-position"
#define RESTORE_DRAG_ITEM_GRID_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-drag-item-grid-position"
#define RESTORE_EXTEND_ITEM_GRID_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-extend-item-grid-position"
#define RESTORE_SINGLESCREEN_ITEM_GRID_POS_ATTRIBUTE "metadata::peony-qt-desktop-restore-singlescreen-item-grid-position"

#include <QtCore>
enum DesktopIconRole {
    UriRole = Qt::UserRole,
    IsLinkRole = Qt::UserRole + 1,
    PositionRole = Qt::UserRole + 2,
    ScreenIdRole = Qt::UserRole + 3,
    ExceptedPositionRole = Qt::UserRole + 4,
    SinglesSreenPositionRole = Qt::UserRole + 5,
    ExtendScreenPositionRole = Qt::UserRole + 6,
    IsFloatItemRole = Qt::UserRole + 7,
    emblemRole = Qt::UserRole + 8,
    IsDirRole = Qt::UserRole + 9,
    SoftLinkRole = Qt::UserRole + 10,
    LabelColors = Qt::UserRole + 11,
    FileType = Qt::UserRole + 12,
    FileSize = Qt::UserRole + 13,
    isDesktop = Qt::UserRole + 14
};

enum ZoomLevel {
    Invalid,
    Small, //icon: 24x24; grid: 64x64; hover rect: 60x60; font: system*0.8
    Normal, //icon: 48x48; grid: 96x96; hover rect = 90x90; font: system
    Large, //icon: 64x64; grid: 115x135; hover rect = 105x118; font: system*1.2
    Huge //icon: 96x96; grid: 140x170; hover rect = 120x140; font: system*1.4
};

#endif // COMMON_H
