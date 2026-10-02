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
 */

#ifndef AUTOTEST_FAKE_RESULT_ITEM_H
#define AUTOTEST_FAKE_RESULT_ITEM_H

#include <QVariant>
#include <QString>

#include <ukui-search/search-result-property.h>

namespace UkuiSearch {

class ResultItem
{
public:
    explicit ResultItem() = default;
    ~ResultItem() = default;

    explicit ResultItem(const size_t searchId)
        : m_searchId(searchId)
    {
    }

    explicit ResultItem(const QString &itemKey)
        : m_itemKey(itemKey)
    {
    }

    ResultItem(const size_t searchId, const QString &itemKey, const SearchResultPropertyMap &map)
        : m_searchId(searchId)
        , m_itemKey(itemKey)
        , m_values(map)
    {
    }

    ResultItem(const ResultItem &item) = default;
    ResultItem &operator=(const ResultItem &other) = default;
    ResultItem &operator=(ResultItem &&other) noexcept = default;

    void setSearchId(const size_t searchId)
    {
        m_searchId = searchId;
    }

    void setItemKey(const QString &itemKey)
    {
        m_itemKey = itemKey;
    }

    size_t getSearchId() const
    {
        return m_searchId;
    }

    QString getItemKey() const
    {
        return m_itemKey;
    }

    void setValue(SearchProperty::SearchResultProperty property, const QVariant &value)
    {
        m_values[property] = value;
    }

    QVariant getValue(SearchProperty::SearchResultProperty property) const
    {
        return m_values.value(property);
    }

    SearchResultPropertyMap getAllValue() const
    {
        return m_values;
    }

private:
    size_t m_searchId = 0;
    QString m_itemKey;
    SearchResultPropertyMap m_values;
};

}

#endif
