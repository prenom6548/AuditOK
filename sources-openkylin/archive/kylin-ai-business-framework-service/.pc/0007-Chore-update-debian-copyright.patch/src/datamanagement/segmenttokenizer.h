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
