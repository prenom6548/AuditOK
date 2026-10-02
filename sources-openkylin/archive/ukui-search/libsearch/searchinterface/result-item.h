/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef RESULTITEM_H
#define RESULTITEM_H

#include <QString>
#include "search-result-property.h"
namespace UkuiSearch {
class ResultItemPrivate;
class ResultItem
{
public:
    explicit ResultItem();
    virtual ~ResultItem();

    explicit ResultItem(const size_t searchId);
    explicit ResultItem(const QString &itemKey);
    ResultItem(const size_t searchId, const QString &itemKey, const SearchResultPropertyMap &map);
    ResultItem(const ResultItem &item);
    ResultItem &operator=(const ResultItem &other);
    ResultItem &operator=(ResultItem &&other) Q_DECL_NOEXCEPT;

    /**
     * @brief setSearchId
     * 设置搜索id,搜索插件调用
     * @param searchId
     */
    void setSearchId(const size_t searchId);

    /**
     * @brief setItemKey
     * 设置结果项唯一标识,搜索插件调用
     * @param itemKey
     */
    void setItemKey(const QString &itemKey);

    /**
     * @brief getSearchId
     * 获取搜索id
     * @return 此次搜索的唯一id
     */
    size_t getSearchId() const;

    /**
     * @brief getItemKey
     * 获取结果项唯一标识
     */
    QString getItemKey() const;

    /**
     * @brief setValue
     * 设置对应的属性值,插件调用
     * @param property 要设置的属性
     * @param value 属性值
     */
    void setValue(SearchProperty::SearchResultProperty property, const QVariant &value);

    /**
     * @brief getValue
     * 获取对应的属性值
     * @param property 要获取值的属性
     * @return 属性值
     */
    QVariant getValue(SearchProperty::SearchResultProperty property) const;

    /**
     * @brief getAllValue
     * 获取所有属性值
     * @return 包含了所有属性值的map
     */
    SearchResultPropertyMap getAllValue() const;

private:
    ResultItemPrivate *d;
};
}

#endif // RESULTITEM_H
