/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef UKUIPOPUPHANDLE_H
#define UKUIPOPUPHANDLE_H

#include "ukuipopupwindowhandle.h"

namespace UKUISDKStyle {

class UKUIPopupHandle : public UKUIQQC2Style::UKUIPopupWindowHandle
{
    Q_OBJECT

    Q_PROPERTY(bool acceptFocus READ acceptFocus WRITE setAcceptFocus NOTIFY acceptFocusChanged)

public:
    explicit UKUIPopupHandle(QQuickItem *parent = nullptr);
    ~UKUIPopupHandle() override;

    bool acceptFocus() const;
    void setAcceptFocus(bool activeFocus);

signals:
    void acceptFocusChanged();

private:
    bool m_acceptFocus = true;
};

}

QML_DECLARE_TYPE(UKUISDKStyle::UKUIPopupHandle)

#endif // UKUIPOPUPHANDLE_H
