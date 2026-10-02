/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef LRC_PARSER_H
#define LRC_PARSER_H

#include <QList>
#include <QString>

struct LrcLine {
    int timeMs; // 时间戳（毫秒）
    QString text; // 歌词文本
};

class LrcParser {
public:
    QList<LrcLine> parseLrc(const QString &filePath);
private:
    int timeStrToMs(const QString &timeStr);
};

#endif // LRC_PARSER_H
