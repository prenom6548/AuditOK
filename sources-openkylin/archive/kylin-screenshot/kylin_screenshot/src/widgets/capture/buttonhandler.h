/* Copyright (C) 2020, KylinSoft Co., Ltd.

*             2020 KylinSoft Co., Ltd.
 * This file is part of Kylin-Screenshot.
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
#pragma once

#include "capturebutton.h"
#include <QVector>
#include <QObject>
#include <QRegion>

class CaptureButton;
class QRect;
class QPoint;

class ButtonHandler : public QObject {
    Q_OBJECT
public:
    ButtonHandler(const QVector<CaptureButton*>&, QObject *parent = nullptr);
    explicit ButtonHandler(QObject *parent = nullptr);

    void hideSectionUnderMouse(const QPoint &p);

    bool isVisible() const;
    bool buttonsAreInside() const;
    size_t size() const;
    int getSeparatorWidth();
    bool m_isTooldrawMode;
    void setButtons(const QVector<CaptureButton*>);
    bool contains(const QPoint &p) const;
    void updateScreenRegions(const QVector<QRect> &rects);
    void updateScreenRegions(const QRect &rect);
    void setIsMoved(bool ismoved);
    void clearButtons();
    const QVector<CaptureButton*> getButtons();
    int move_FontSize_Color_Chose_Window(int y);
    QPoint move_Save_Location_Window(int x,int y);
    QPoint move_Font_Options_Window(int x,int y);

    int FontSize_Color_Chose_Window_Y;
    QPoint Save_Location_Window_Pos;
    QPoint Font_Options_Window_Pos;
public slots:
    void updatePosition(const QRect &selection);
    void hide();
    void show();
    void setScrollMode(bool scroll_mode);
    void setScrolltoolState();
    bool isScrollMode();
private:
    QVector<QPoint> horizontalPoints(const QPoint &center, const int elements,
                               const bool leftToRight) const;
    QVector<QPoint> verticalPoints(const QPoint &center, const int elements,
                               const bool upToDown) const;

    QRect intersectWithAreas(const QRect &rect);

    QVector<CaptureButton*> m_vectorButtons;

    QRegion m_screenRegions;
    QRect m_buttonCurScreen;    //工具栏按钮所在的屏幕（扩展时，按钮在选区多的屏幕上）

    QRect m_selection;
    QVector<QRect> allrect;
    int m_separator;
    int m_buttonExtendedSize;
    int m_buttonBaseSize;

    bool m_buttonsAreInside;
    bool m_blockedRight;
    bool m_blockedLeft;
    bool m_blockedBotton;
    bool m_blockedTop;
    bool m_oneHorizontalBlocked;
    bool m_horizontalyBlocked;
    bool m_allSidesBlocked;
    bool m_isTabletMode; // 平板模式
    bool m_isFirstMove;
    bool m_isScrollMode;
    bool m_isSelectionResized;
    // aux methods
    void init();
    void resetRegionTrack();
    void updateBlockedSides();
    void expandSelection();
    void positionButtonsInside(int index);
    void ensureSelectionMinimunSize();
    void moveButtonsToPoints(const QVector<QPoint> &points, int &index);
    void adjustHorizontalCenter(QPoint &center);
};
