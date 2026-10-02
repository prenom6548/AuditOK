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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef UKUI_SEARCH_ONE_RESULT_H
#define UKUI_SEARCH_ONE_RESULT_H

#include "search-plugin-iface.h"
namespace UkuiSearch {
Q_NAMESPACE
enum AdditionalRoles {
    ShowToolTip = Qt::UserRole + 1,
    DisplayNameAsUrl = Qt::UserRole + 2,
    IconName = Qt::UserRole + 3,
};
Q_ENUM_NS(AdditionalRoles)

class OneResult {
public:
    explicit OneResult(const SearchPluginIface::ResultInfo &ri);

    void setShowToolTip(bool show);
    [[nodiscard]] bool showToolTip() const;
    [[nodiscard]] const SearchPluginIface::ResultInfo &info() const;

private:
    SearchPluginIface::ResultInfo m_info;
    bool m_showToolTip = true;
};
}
#endif //UKUI_SEARCH_ONE_RESULT_H
