/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WAYLANDWORKSPACEADJUSTER_H
#define WAYLANDWORKSPACEADJUSTER_H

#include "workspaceadjuster.h"

class WaylandWlcomWorkspaceAdjuster : public WorkspaceAdjuster {
public:
    WaylandWlcomWorkspaceAdjuster();
    ~WaylandWlcomWorkspaceAdjuster() override = default;

    void raiseInputArea(QWindow * /*window*/, const QRect &rect) override;
    void fallInputArea() override;

private:
    void setSurfaceWindowProperty(const QRect &rect, const int32_t &enabled);

private:
    QWindow *surfaceWindow_ = nullptr;
};
#endif
