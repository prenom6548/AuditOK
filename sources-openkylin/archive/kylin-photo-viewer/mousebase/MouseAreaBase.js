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

function checkComboForShortcut(combo, wheelDelta) {

    if(wheelDelta.x === 0 && wheelDelta.y === 0){
        return;
    }

    if (wheelDelta.y > 0) {
        whatToDoWithFoundShortcut([0,"__zoomIn"], wheelDelta)
    } else if (wheelDelta.x < 0){
        whatToDoWithFoundShortcut([0,"__zoomIn"], wheelDelta)
    } else {
        whatToDoWithFoundShortcut([0,"__zoomOut"], wheelDelta)
    }

}

function whatToDoWithFoundShortcut(sh, wheelDelta) {

    var cmd = sh[1]

    if(cmd === "__zoomIn") {
        imageArea.zoomIn(wheelDelta)
    } else if (cmd === "__zoomOut") {
        imageArea.zoomOut(wheelDelta)
    } else if(cmd === "__zoomReset") {
        imageArea.zoomReset()
    } else if(cmd === "__zoomActual") {
        imageArea.zoomActual()
    } else if(cmd === "__rotateL") {
        imageArea.rotate(-variables.angle90)
    } else if(cmd === "__rotateR") {
        imageArea.rotate(variables.angle90)
    } else if(cmd === "__rotate0") {
        imageArea.rotateReset()
    } else if(cmd === "__flipH") {
        imageArea.mirrorH()
    } else if(cmd === "__flipV") {
        imageArea.mirrorV()
    } else if(cmd === "__flipReset") {
        imageArea.mirrorReset()
    }

}
