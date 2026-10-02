/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: anyongtai <anyongtai@kylinos.cn>
 *
 */
#ifndef FULL_SCREEN_STATE_HELPER_H
#define FULL_SCREEN_STATE_HELPER_H

#include <QObject>
#include "security-function-control.h"

namespace UkuiMenu {
class FullScreenStateHelper : public QObject
{
    Q_OBJECT
public:
    static FullScreenStateHelper *instance();
    bool getFullScreenState();
    void setFullScreenState(bool isFullScreen);

private:
    explicit FullScreenStateHelper(QObject *parent = nullptr);

Q_SIGNALS:
    void fullScreenStateChanged(bool isFullScreen);

private:
    bool m_gsettingsFullScreen = false;
    bool m_windowMaximizeControl = false;
};
} // UkuiMenu

#endif // FULL_SCREEN_STATE_HELPER_H
