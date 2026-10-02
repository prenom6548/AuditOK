#include "utils.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <uuid/uuid.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <regex>
#include <string>

std::string Utils::generateUUID() {
    uuid_t uuid;
    char uuidStr[37];

    uuid_generate_time(uuid);
    uuid_unparse(uuid, uuidStr);

    return uuidStr;
}

uint64_t Utils::generateRandomNumber() {
    static thread_local std::mt19937_64 engine(std::random_device{}());
    static thread_local std::uniform_int_distribution<uint64_t> distribution;
    return distribution(engine);
}

int Utils::ceilDivide(int a, int b) {
    int ceil_result = (a + b - 1) / b;
    return ceil_result;
}

Json::Value Utils::formatJsonFromString(const std::string& stringJson) {
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(stringJson.c_str(), stringJson.c_str() + stringJson.size(), &obj, &error);
    return obj;
}

std::string Utils::base64Encode(const std::string& input) {
    BIO *bio, *b64;
    BUF_MEM* bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}

std::string Utils::base64Decode(const std::string& input) {
    BIO* bio{nullptr};
    BIO* b64{nullptr};
    char* buffer = new char[input.size()];
    std::copy(input.begin(), input.end(), buffer);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(buffer, static_cast<int>(input.size()));
    bio = BIO_push(b64, bio);

    int decodedLength = BIO_read(bio, buffer, static_cast<int>(input.size()));
    std::string result(buffer, decodedLength);
    BIO_free_all(bio);
    delete[] buffer;

    return result;
}

std::vector<uint8_t> Utils::resampleAudio(const std::vector<uint8_t>& inputData, int inputRate, int outputRate) {
    if (inputData.empty() || inputRate <= 0 || outputRate <= 0) {
        return std::vector<uint8_t>();
    }

    // 如果输入和输出采样率相同，则直接返回输入数据的副本
    if (inputRate == outputRate) {
        return inputData;
    }

    // 计算输出数据大小
    size_t inputSamples = inputData.size() / sizeof(int16_t);  // 假设16位音频
    size_t outputSamples = static_cast<size_t>(inputSamples * static_cast<double>(outputRate) / inputRate);

    // 创建输出数据容器
    std::vector<uint8_t> outputData(outputSamples * sizeof(int16_t));

    // 执行重采样 - 使用线性插值
    const int16_t* input = reinterpret_cast<const int16_t*>(inputData.data());
    int16_t* output = reinterpret_cast<int16_t*>(outputData.data());

    for (size_t i = 0; i < outputSamples; ++i) {
        // 计算在输入中的位置
        double position = i * static_cast<double>(inputRate) / outputRate;
        size_t index = static_cast<size_t>(position);
        double fraction = position - index;

        // 边界检查
        if (index >= inputSamples) {
            index = inputSamples - 1;
        }

        if (index == inputSamples - 1) {
            // 最后一个样本，无法插值
            output[i] = input[index];
        } else {
            // 线性插值
            output[i] = static_cast<int16_t>(input[index] + fraction * (input[index + 1] - input[index]));
        }
    }

    return outputData;
}

std::string Utils::removeDoubleAt(const std::string& input) {
    std::regex re("@@");
    return std::regex_replace(input, re, "");
}

size_t Utils::utf8CharLen(unsigned char ch) {
    if ((ch & 0x80) == 0) return 1;
    if ((ch & 0xE0) == 0xC0) return 2;
    if ((ch & 0xF0) == 0xE0) return 3;
    if ((ch & 0xF8) == 0xF0) return 4;
    return 1;
}

size_t Utils::utf8Prev(const std::string& s, size_t pos) {
    if (pos == 0) return 0;
    pos--;
    while (pos > 0 && (static_cast<unsigned char>(s[pos]) & 0xC0) == 0x80) {
        pos--;
    }
    return pos;
}

std::string Utils::getUtf8Char(const std::string& s, size_t pos) {
    size_t len = utf8CharLen((unsigned char)s[pos]);
    if (pos + len > s.size()) return s.substr(pos);
    return s.substr(pos, len);
}

bool Utils::inList(const std::string& c, const std::vector<std::string>& list) {
    return std::find(list.begin(), list.end(), c) != list.end();
}

size_t Utils::countUtf8Chars(const std::string& text, size_t start, size_t maxChars) {
    size_t end = start;
    size_t count = 0;

    while (end < text.size() && count < maxChars) {
        size_t len = utf8CharLen((unsigned char)text[end]);
        end += len;
        count++;
    }
    return end;
}

// 向前（老句子中）查找可能存在的标点分割点
bool Utils::findOldSplitPosition(const std::string& text,
                                 size_t searchStart,  // 搜索起点（一般为 endPos）
                                 size_t searchEnd,    // 搜索终点（一般为 endPos - searchWindow）
                                 const std::vector<std::string>& puncts, const std::vector<std::string>& allPuncts,
                                 bool allowSpace, size_t& splitPos) {
    size_t spaceSplitPos = 0;  // 候选空格位置（标点优先）
    for (size_t i = searchStart; i > searchEnd;) {
        size_t len = utf8CharLen((unsigned char)text[i]);
        size_t pos = i;
        i--;

        std::string ch = getUtf8Char(text, pos);

        // 1) 标点优先
        if (inList(ch, puncts)) {
            splitPos = pos + len;
            return true;
        }

        // 2) 空格：标点优先，空格作为备选
        if (allowSpace && ch == " " && spaceSplitPos == 0) {
            size_t next = pos + len;
            if (next >= text.size()) {  // 文本结尾的空格
                spaceSplitPos = pos + len;
                continue;
            }
            std::string nextChar = getUtf8Char(text, next);
            if (!inList(nextChar, allPuncts)) {  // 空格后不是标点
                // 跳过英文单词间的空格
                if (pos > 0 && next < text.size()) {
                    unsigned char prevByte = static_cast<unsigned char>(text[pos - 1]);
                    unsigned char nextByte = static_cast<unsigned char>(text[next]);
                    if (std::isalpha(prevByte) && std::isalpha(nextByte)) {
                        continue;
                    }
                }
                spaceSplitPos = pos + len;  // 记录但不返回，继续找标点
            }
        }
    }
    if (spaceSplitPos > 0) {
        splitPos = spaceSplitPos;
        return true;
    }
    return false;
}

// 向后（新句子中）查找可能的标点分割点
bool Utils::findNewSplitPosition(const std::string& text, size_t searchStart, size_t searchEnd,
                                 const std::vector<std::string>& puncts, const std::vector<std::string>& allPuncts,
                                 bool allowSpace, size_t& splitPos) {
    if (searchEnd > text.size()) {
        searchEnd = text.size();
    }
    size_t spaceSplitPos = 0;  // 候选空格位置（标点优先）
    for (size_t i = searchStart; i < searchEnd;) {
        size_t len = utf8CharLen((unsigned char)text[i]);
        size_t pos = i;
        i += len;

        std::string ch = getUtf8Char(text, pos);

        // 标点优先
        if (inList(ch, puncts)) {
            splitPos = pos + len;
            return true;
        }

        if (allowSpace && ch == " " && spaceSplitPos == 0) {
            if (i >= text.size()) {
                spaceSplitPos = pos + len;
                continue;
            }
            std::string nextChar = getUtf8Char(text, i);
            if (!inList(nextChar, allPuncts)) {
                // 跳过英文单词间的空格
                if (pos > 0 && i < text.size()) {
                    unsigned char prevByte = static_cast<unsigned char>(text[pos - 1]);
                    unsigned char nextByte = static_cast<unsigned char>(text[i]);
                    if (std::isalpha(prevByte) && std::isalpha(nextByte)) {
                        continue;
                    }
                }
                spaceSplitPos = pos + len;  // 记录但不返回，继续找标点
            }
        }
    }
    if (spaceSplitPos > 0) {
        splitPos = spaceSplitPos;
        return true;
    }
    return false;
}

inline size_t utf8Next(const std::string& s, size_t pos) {
    const size_t n = s.size();
    if (pos >= n) {
        return n;
    }

    unsigned char c = static_cast<unsigned char>(s[pos]);

    // ASCII: 0xxxxxxx
    if ((c & 0x80) == 0x00) {
        return pos + 1;
    }

    // 110xxxxx → 2 bytes
    if ((c & 0xE0) == 0xC0) {
        return std::min(pos + 2, n);
    }

    // 1110xxxx → 3 bytes
    if ((c & 0xF0) == 0xE0) {
        return std::min(pos + 3, n);
    }

    // 11110xxx → 4 bytes
    if ((c & 0xF8) == 0xF0) {
        return std::min(pos + 4, n);
    }

    // 非法 UTF-8 起始字节（防御性处理）
    return pos + 1;
}

// 删除所有千分位符号，避免过度停顿或突然停顿
inline std::string removeThousandsComma(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == ',' && i > 0 && i + 3 < text.size() && std::isdigit(static_cast<unsigned char>(text[i - 1])) &&
            std::isdigit(static_cast<unsigned char>(text[i + 1])) &&
            std::isdigit(static_cast<unsigned char>(text[i + 2])) &&
            std::isdigit(static_cast<unsigned char>(text[i + 3]))) {
            continue;
        }
        result.push_back(text[i]);
    }
    return result;
}

// 判断是否为数字
inline bool isAsciiDigit(const std::string& ch) {
    return ch.size() == 1 && std::isdigit(static_cast<unsigned char>(ch[0]));
}

// 判断是否为英文字母
inline bool isAsciiAlpha(const std::string& ch) {
    return ch.size() == 1 && std::isalpha(static_cast<unsigned char>(ch[0]));
}

// 将 UTF-8 编码的字符转换为 Unicode 码点（codepoint）为后续的中文/CJK 字符判断提供基础
inline uint32_t utf8Codepoint(const std::string& ch) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(ch.data());
    if (ch.size() == 1) {
        return bytes[0];
    }
    if (ch.size() == 2) {
        return ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
    }
    if (ch.size() == 3) {
        return ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
    }
    if (ch.size() == 4) {
        return ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) | ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
    }
    return 0;
}

// 判断是否为统一表意字符(识别是否为中文字符)
inline bool isCjkChar(const std::string& ch) {
    uint32_t codepoint = utf8Codepoint(ch);
    return (codepoint >= 0x3400 && codepoint <= 0x4DBF) || (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
           (codepoint >= 0xF900 && codepoint <= 0xFAFF) || (codepoint >= 0x20000 && codepoint <= 0x2A6DF) ||
           (codepoint >= 0x2A700 && codepoint <= 0x2B73F) || (codepoint >= 0x2B740 && codepoint <= 0x2B81F) ||
           (codepoint >= 0x2B820 && codepoint <= 0x2CEAF);
}

// 如果是数字开头，就要向后找到文本一起切分
inline bool needsTextCompanionForLeadingNumber(const std::string& text, size_t start, size_t splitPos,
                                               const std::vector<std::string>& puncts) {
    bool hasDigit = false;
    for (size_t pos = start; pos < splitPos;) {
        std::string ch = Utils::getUtf8Char(text, pos);
        pos += ch.size();

        if (isAsciiDigit(ch)) {
            hasDigit = true;
            continue;
        }

        if (isAsciiAlpha(ch) || isCjkChar(ch)) {
            return false;
        }

        if (ch.size() == 1 && std::isspace(static_cast<unsigned char>(ch[0]))) {
            continue;
        }

        if (Utils::inList(ch, puncts)) {
            continue;
        }

        return false;
    }

    return hasDigit;
}

// 统一计数：每个中文字 = 1 单位，每个英文单词 = 1 单位。
// 数字、标点、空格不计入单位。返回计满 maxUnits 个单位的字节位置。
inline size_t countTextUnits(const std::string& text, size_t start, size_t maxUnits) {
    size_t pos = start;
    size_t units = 0;
    bool inEnglish = false;

    while (pos < text.size() && units < maxUnits) {
        unsigned char c = static_cast<unsigned char>(text[pos]);

        if ((c & 0x80) == 0) {
            if (std::isalpha(c)) {
                if (!inEnglish) {
                    inEnglish = true;
                    units++;
                }
            } else {
                inEnglish = false;
            }
            pos++;
        } else {
            // 多字节 UTF-8 字符（中文等）— 每个字计 1 单位
            inEnglish = false;
            size_t len = Utils::utf8CharLen(c);
            units++;
            pos += len;
        }
    }

    // 如果停在英文单词中间，继续到单词末尾
    if (units >= maxUnits && inEnglish) {
        while (pos < text.size()) {
            unsigned char c = static_cast<unsigned char>(text[pos]);
            if ((c & 0x80) == 0 && std::isalpha(c)) {
                pos++;
            } else {
                break;
            }
        }
    }

    return pos;
}

std::vector<std::string> Utils::splitText(const std::string& text, size_t maxChars, size_t searchWindow) {
    std::string cleanedText = removeThousandsComma(text);

    auto isDecimalPoint = [&](size_t pos) -> bool {
        if (pos == 0 || pos + 1 >= cleanedText.size()) return false;
        if (cleanedText[pos] != '.') return false;
        char prevChar = cleanedText[pos - 1];
        char nextChar = cleanedText[pos + 1];
        return (prevChar >= '0' && prevChar <= '9') && (nextChar >= '0' && nextChar <= '9');
    };

    std::vector<std::string> segments;

    // 分隔符
    static const std::vector<std::string> strong = {"。", "！", "？", ".", "!", "?"};
    static const std::vector<std::string> weak = {"，", "；", "：", ",", ";", ":", "、"};
    static const std::vector<std::string> all = {"。", "！", "？", "；", "：", "，", "、",
                                                 ".",  "!",  "?",  ";",  ":",  ",",  " "};

    size_t start = 0;
    const size_t textSize = cleanedText.size();

    while (start < textSize) {
        // 按"单位"计数：每个中文字=1，每个英文单词=1
        size_t endPos = countTextUnits(cleanedText, start, maxChars);

        if (endPos <= start) {
            endPos = utf8Next(cleanedText, start);
        }

        // 收尾
        if (endPos >= textSize) {
            segments.emplace_back(cleanedText.substr(start));
            break;
        }

        size_t splitPos = endPos;

        // 第一块：从 endPos 向后搜索 searchWindow 单位找标点，找不到再强制截断
        if (start == 0) {
            bool found = false;
            size_t searchEnd = countTextUnits(cleanedText, endPos, searchWindow);
            if (searchEnd > textSize) searchEnd = textSize;

            for (size_t pos = start; pos < searchEnd;) {
                std::string ch = getUtf8Char(cleanedText, pos);
                size_t next = pos + ch.size();

                if (ch != " " && inList(ch, all)) {
                    if (ch == "." && isDecimalPoint(pos)) {
                        pos = next;
                        continue;
                    }
                    splitPos = next;
                    found = true;
                    break;
                }
                pos = next;
            }

            if (!found) {
                splitPos = endPos;
            }

            if (needsTextCompanionForLeadingNumber(cleanedText, start, splitPos, all)) {
                found = false;
                for (size_t pos = splitPos; pos < endPos;) {
                    std::string ch = getUtf8Char(cleanedText, pos);
                    size_t next = pos + ch.size();

                    if (inList(ch, all)) {
                        if (ch == "." && isDecimalPoint(pos)) {
                            pos = next;
                            continue;
                        }
                        splitPos = next;
                        found = true;
                        break;
                    }

                    pos = next;
                }

                if (!found) {
                    splitPos = endPos;
                }
            }

            if (splitPos <= start) {
                splitPos = utf8Next(cleanedText, start);
            }

            segments.emplace_back(cleanedText.substr(start, splitPos - start));
            start = splitPos;
            continue;
        }

        // 第二块：前后搜索标点
        // 向后搜 searchWindow 个"单位"（中文1字=1单位，英文1词=1单位）
        size_t searchBegin = endPos;
        {
            size_t unitsBack = 0;
            while (unitsBack < searchWindow && searchBegin > start) {
                size_t prev = utf8Prev(cleanedText, searchBegin);
                unsigned char c = static_cast<unsigned char>(cleanedText[prev]);
                if ((c & 0x80) == 0) {
                    // ASCII：只有单词开头才计 1 单位
                    if (std::isalpha(c)) {
                        bool isWordStart = (prev == start || prev == 0 ||
                            !std::isalpha(static_cast<unsigned char>(cleanedText[prev - 1])));
                        if (isWordStart) unitsBack++;
                    }
                } else {
                    // 多字节 UTF-8（中文等）：每字 1 单位
                    unitsBack++;
                }
                searchBegin = prev;
            }
        }

        bool found = false;
        for (size_t i = endPos; i > searchBegin;) {
            size_t prev = utf8Prev(cleanedText, i);
            std::string ch = getUtf8Char(cleanedText, prev);
            if (inList(ch, strong)) {
                if (ch == "." && isDecimalPoint(prev)) {
                    i = prev;
                    continue;
                }
                splitPos = prev + ch.size();
                found = true;
                break;
            }
            i = prev;
        }

        // 向后搜弱标点+空格
        size_t backSplitPos = 0;
        bool foundBack = !found && findOldSplitPosition(cleanedText, endPos, searchBegin, weak, all, true, backSplitPos);

        // 向前搜弱标点+空格（标点优先于空格，向前结果优先于向后结果）
        size_t forwardEnd = countTextUnits(cleanedText, endPos, searchWindow);
        size_t fwdSplitPos = 0;
        bool foundFwd = !found && findNewSplitPosition(cleanedText, endPos, std::min(textSize, forwardEnd), weak, all,
                                                       true, fwdSplitPos);

        if (!found) {
            if (foundFwd) {
                splitPos = fwdSplitPos;        // 向前找到（标点或空格）→ 优先
                found = true;
            } else if (foundBack) {
                splitPos = backSplitPos;       // 只有向后找到
                found = true;
            }
        }

        if (!found) {
            splitPos = endPos;             // 都没找到 → 单位边界
        }

        if (splitPos <= start) {
            splitPos = utf8Next(cleanedText, start);
        }

        segments.emplace_back(cleanedText.substr(start, splitPos - start));
        start = splitPos;
    }

    return segments;
}

std::vector<uint8_t> Utils::normalizePcm16Rms(const std::vector<uint8_t>& inputData, float targetDbfs,
                                              float maxGainDb) {
    if (inputData.empty() || inputData.size() % sizeof(int16_t) != 0) {
        return {};
    }

    std::vector<uint8_t> outputData(inputData.size());

    const int16_t* input = reinterpret_cast<const int16_t*>(inputData.data());
    int16_t* output = reinterpret_cast<int16_t*>(outputData.data());

    size_t sampleCount = inputData.size() / sizeof(int16_t);

    double sumSquares = 0.0;

    for (size_t i = 0; i < sampleCount; ++i) {
        double x = input[i] / 32768.0;
        sumSquares += x * x;
    }

    double rms = std::sqrt(sumSquares / sampleCount);

    if (rms < 1e-5) {
        return inputData;
    }

    double currentDbfs = 20.0 * std::log10(rms);
    double gainDb = targetDbfs - currentDbfs;

    gainDb = std::clamp(gainDb, static_cast<double>(-maxGainDb), static_cast<double>(maxGainDb));

    double gain = std::pow(10.0, gainDb / 20.0);

    for (size_t i = 0; i < sampleCount; ++i) {
        double y = input[i] * gain;
        y = std::clamp(y, -32768.0, 32767.0);
        output[i] = static_cast<int16_t>(y);
    }

    return outputData;
}
