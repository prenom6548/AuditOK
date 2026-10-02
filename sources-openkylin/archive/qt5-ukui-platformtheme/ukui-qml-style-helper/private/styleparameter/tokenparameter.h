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

#ifndef TOKENPARAMETER_H
#define TOKENPARAMETER_H

#include <QObject>
#include <QMetaType>
#include "../../../ukui-styles/readconfig.h"
#include "ukuibutton.h"

namespace UKUIQQC2Style {

class TokenParameter
{
    //Q_PROPERTY(std::shared_ptr<UKUIQQC2Style::UKUIButton> button READ button WRITE setButton NOTIFY buttonChanged)

public:
    explicit TokenParameter();
    ~TokenParameter();

    UKUIGlobalDTConfig::GlobalDTConfig* getInstance();
    void deleteInstance();


//    const std::shared_ptr<UKUIQQC2Style::UKUIButton> &button() const;
//    void setButton(std::shared_ptr<UKUIQQC2Style::UKUIButton> &newButton);

public Q_SLOTS:



private:

    UKUIGlobalDTConfig::GlobalDTConfig* dt = nullptr;
//    UKUIButton m_button;
};
}
Q_DECLARE_METATYPE(UKUIQQC2Style::TokenParameter*)

#endif // TOKENPARAMETER_H
