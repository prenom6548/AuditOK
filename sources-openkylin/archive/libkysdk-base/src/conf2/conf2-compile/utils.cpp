/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include <QRegularExpression>
#include <algorithm>

namespace
{
    static bool sortString(const QString &a, const QString &b)
    {
        static QRegularExpression regex("^(\\d+)-(.*)");
        QRegularExpressionMatch matchA = regex.match(a);
        QRegularExpressionMatch matchB = regex.match(b);

        bool hasPrefixA = matchA.hasMatch();
        bool hasPrefixB = matchB.hasMatch();

        if (!hasPrefixA && !hasPrefixB)
            return a < b;
        if (!hasPrefixA)
            return true;
        if (!hasPrefixB)
            return false;

        int prefixA = matchA.captured(1).toInt();
        int prefixB = matchB.captured(1).toInt();

        return prefixA != prefixB ? prefixA < prefixB : a < b;
    }

    static bool compareVersions(const QString &version1, const QString &version2)
    {
        auto parseVersion = [](const QString &versionStr) -> std::pair<QStringList, QStringList>
        {
            static QRegularExpression numRegex("\\d+");
            static QRegularExpression segmentRegex("\\D+");

            QStringList nums, segments;

            QRegularExpressionMatchIterator numIt = numRegex.globalMatch(versionStr);
            while (numIt.hasNext())
                nums.append(numIt.next().captured());

            QRegularExpressionMatchIterator segmentIt = segmentRegex.globalMatch(versionStr);
            while (segmentIt.hasNext())
                segments.append(segmentIt.next().captured());

            return std::make_pair(nums, segments);
        };

        std::pair<QStringList, QStringList> parts1 = parseVersion(version1);
        std::pair<QStringList, QStringList> parts2 = parseVersion(version2);

        QStringList nums1 = parts1.first;
        QStringList segments1 = parts1.second;
        QStringList nums2 = parts2.first;
        QStringList segments2 = parts2.second;

        if (segments1 != segments2)
            throw std::invalid_argument("Invalid version string: segments do not match");
        if (nums1.size() != nums2.size())
            throw std::invalid_argument("Invalid version string: number of numeric parts do not match");

        for (int i = 0; i < nums1.size(); ++i)
        {
            int num1 = nums1[i].toInt();
            int num2 = nums2[i].toInt();
            if (num1 != num2)
                return num1 < num2;
        }

        return false;
    }
} // namespace
