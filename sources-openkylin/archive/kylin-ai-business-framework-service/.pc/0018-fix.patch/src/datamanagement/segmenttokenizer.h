/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SEGMENTTOKENIZER_H
#define SEGMENTTOKENIZER_H

#include <vector>
#include <string>

namespace SegmentTokenizer {

std::vector<std::string> splitString(const std::string& str, const std::string& delimiters);
std::vector<std::string> splitChineseString(const std::string& str, int maxLength);
std::vector<std::string> splitEnglishString(const std::string& str, int maxLength);
bool isEnglishPeriod(const std::string& text, size_t index);
bool containsEnglishPeriod(const std::string& text);
std::vector<std::string> fixedSizeChunks(const std::string& text, int chunkSize);
void segmentTokenize(std::vector<std::string>& texts, int maxLength);

} // namespace SegmentTokenizer

#endif // SEGMENTTOKENIZER_H
