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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "util.h"

#include <glib.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <unicode/ustdio.h>
#include <unicode/ustring.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "logger.h"

namespace util {

std::string jsonToString(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}

Json::Value stringToJson(const std::string& string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return Json::objectValue;
    }
}

std::string removeJsonTrailingCommas(const std::string& jsonStr) {
    std::string s = jsonStr;
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i + 1 < s.size();) {
            if (s[i] == ',') {
                size_t j = i + 1;
                while (j < s.size() && (s[j] == ' ' || s[j] == '\n' || s[j] == '\r' || s[j] == '\t')) {
                    ++j;
                }
                if (j < s.size() && (s[j] == ']' || s[j] == '}')) {
                    s.erase(i, 1);
                    changed = true;
                    continue;
                }
            }
            ++i;
        }
    }
    return s;
}

std::optional<Json::Value> stringToJson2(const std::string& string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return std::nullopt;
    }
}

std::string currentDate(const std::string& format) {
    setlocale(LC_TIME, "C");
    time_t rawTime = time(nullptr);
    tm* gmt = localtime(&rawTime);
    char tstr[30];
    strftime(tstr, sizeof(tstr), format.c_str(), gmt);
    return tstr;
}

std::vector<uint8_t> base64Decode(const std::string& input) {
    gsize decodedLength{};
    guchar* decodedData = g_base64_decode(input.c_str(), &decodedLength);
    std::vector<uint8_t> result(decodedData, decodedData + decodedLength);
    g_free(decodedData);
    return result;
}

std::string base64Encode(const char* data, int length) {
    BIO *bio, *b64;
    BUF_MEM* bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, data, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}

std::string base64Encode(const std::string& input) { return base64Encode(input.data(), input.size()); }

void removeSequenceFromString(std::string& str, const std::string& sequence) {
    size_t pos = 0;
    while ((pos = str.find(sequence, pos)) != std::string::npos) {
        str.erase(pos, sequence.length());
    }
}

void removeZeroWidthChar(std::string& str) {
    static const char* zeroWidthChars[] = {
        "\xE2\x80\x8B",  // Zero width space
        "\xE2\x80\x8C",  // Zero width non-joiner
        "\xE2\x80\x8D",  // Zero width joiner
        "\xE2\x80\x8E",  // Left to right mark
        "\xE2\x80\x8F",  // Right to left mark
    };
    for (const char* zeroWidthChar : zeroWidthChars) {
        removeSequenceFromString(str, zeroWidthChar);
    }
}

bool stringContains(const std::string& str, const std::string& sub) {
    if (str.length() < sub.length()) {
        return false;
    };
    return str.find(sub) != std::string::npos;
}

const std::string readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

bool stringEndWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    size_t startPos = str.length() - suffix.length();
    return str.substr(startPos) == suffix;
}

/**
 * 查找子串在字符串中最后一次出现的位置
 * @param str 要搜索的字符串
 * @param substr 要查找的子串
 * @param from 开始搜索的位置（从0开始），默认从字符串末尾开始
 * @return 子串最后一次出现的起始位置，未找到返回-1
 */
int stringLastIndexOf(const std::string& str, const std::string& substr, size_t from) {
    // 处理空字符串情况
    if (str.empty() || substr.empty()) {
        return -1;
    }

    // 确保from不超过字符串长度
    if (from == std::string::npos || from >= str.length()) {
        from = str.length() - 1;
    }

    // 子串长度
    size_t substr_len = substr.length();

    // 如果子串比字符串长，直接返回-1
    if (substr_len > str.length()) {
        return -1;
    }

    // 计算最大可能的起始位置
    size_t max_start = from >= substr_len - 1 ? from - substr_len + 1 : 0;

    // 从后向前搜索
    for (size_t i = max_start;; --i) {
        if (str.compare(i, substr_len, substr) == 0) {
            return static_cast<int>(i);
        }

        // 到达字符串开头，退出循环
        if (i == 0) {
            break;
        }
    }

    // 未找到子串
    return -1;
}

std::string stringRight(const std::string& str, size_t length) {
    // 如果指定长度为0，返回空字符串
    if (length == 0) {
        return "";
    }

    // 如果指定长度大于等于字符串长度，返回整个字符串
    if (length >= str.length()) {
        return str;
    }

    // 否则返回从右侧截取的指定长度子串
    return str.substr(str.length() - length, length);
}

// UTF-8转宽字符串
std::wstring utf8ToWString(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return std::wstring{};
    }

    // 打开UTF-8转换器
    UConverter* converter = ucnv_open("UTF-8", nullptr);
    if (!converter) {
        return std::wstring{};
    }

    // 计算需要的宽字符缓冲区大小
    const char* source = utf8_str.c_str();
    int32_t source_len = static_cast<int32_t>(utf8_str.length());
    int32_t target_cap = ucnv_toUChars(converter, nullptr, 0, source, source_len, nullptr);

    if (target_cap <= 0) {
        ucnv_close(converter);
        return std::wstring{};
    }

    // 分配缓冲区并进行转换
    std::vector<UChar> target_buf(target_cap + 1);
    int32_t result = ucnv_toUChars(converter, target_buf.data(), target_cap, source, source_len, nullptr);

    ucnv_close(converter);  // 确保关闭转换器

    if (result <= 0) {
        return std::wstring{};
    }

    // 转换为std::wstring
    target_buf[target_cap] = 0;  // 添加终止符
    return std::wstring(reinterpret_cast<wchar_t*>(target_buf.data()), target_cap);
}

// 宽字符串转UTF-8
std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string{};
    }

    // 打开UTF-8转换器
    UConverter* converter = ucnv_open("UTF-8", nullptr);
    if (!converter) {
        return std::string{};
    }

    // 计算需要的UTF-8缓冲区大小
    const UChar* source = reinterpret_cast<const UChar*>(wstr.c_str());
    int32_t source_len = static_cast<int32_t>(wstr.length());
    int32_t target_cap = ucnv_fromUChars(converter, nullptr, 0, source, source_len, nullptr);

    if (target_cap <= 0) {
        ucnv_close(converter);
        return std::string{};
    }

    // 分配缓冲区并进行转换
    std::vector<char> target_buf(target_cap + 1);
    int32_t result = ucnv_fromUChars(converter, target_buf.data(), target_cap, source, source_len, nullptr);

    ucnv_close(converter);  // 确保关闭转换器

    if (result <= 0) {
        return std::string{};
    }

    // 保存结果
    target_buf[target_cap] = 0;  // 添加终止符
    return std::string(target_buf.data(), target_cap);
}

// 从宽字符串中移除指定字符
void wstringRemove(std::wstring& wstr, wchar_t ch) {
    // 使用erase-remove惯用法安全移除字符
    wstr.erase(std::remove(wstr.begin(), wstr.end(), ch), wstr.end());
}

// 综合函数：处理UTF-8字符串，移除指定宽字符
void stringRemove(std::string& utf8Str, wchar_t chToRemove) {
    // 转换为宽字符串
    std::wstring wstr = utf8ToWString(utf8Str);
    if (wstr.empty()) {
        return;
    }

    // 移除指定字符
    wstringRemove(wstr, chToRemove);

    // 转换回UTF-8
    utf8Str = wstringToUtf8(wstr);
}

std::vector<std::string> stringSplit(const std::string& str, const std::string& delim) {
    std::vector<std::string> result;
    size_t begin = 0;
    for (size_t end; end = str.find(delim, begin), end != std::string::npos; begin = end + delim.size()) {
        result.emplace_back(str.substr(begin, end - begin));
    }
    result.emplace_back(str.substr(begin));
    return result;
}

// 转换函数的实现，使用libicu库
static std::string convert(const std::string& input, const char* from_encoding, const char* to_encoding) {
    UErrorCode status = U_ZERO_ERROR;

    // 创建转换器
    UConverter* from_conv = ucnv_open(from_encoding, &status);
    if (U_FAILURE(status)) {
        return "";
    }

    UConverter* to_conv = ucnv_open(to_encoding, &status);
    if (U_FAILURE(status)) {
        ucnv_close(from_conv);
        return "";
    }

    // 首先将输入转换为UTF-16
    int32_t utf16_len = ucnv_toUChars(from_conv, nullptr, 0, input.c_str(), input.size(), &status);
    status = U_ZERO_ERROR;  // 重置错误码

    if (utf16_len <= 0) {
        ucnv_close(from_conv);
        ucnv_close(to_conv);
        return "";
    }

    std::unique_ptr<UChar[]> utf16_buf(new UChar[utf16_len + 1]);
    ucnv_toUChars(from_conv, utf16_buf.get(), utf16_len + 1, input.c_str(), input.size(), &status);
    if (U_FAILURE(status)) {
        ucnv_close(from_conv);
        ucnv_close(to_conv);
        return "";
    }

    // 再将UTF-16转换为目标编码
    status = U_ZERO_ERROR;
    int32_t output_len = ucnv_fromUChars(to_conv, nullptr, 0, utf16_buf.get(), utf16_len, &status);
    status = U_ZERO_ERROR;  // 重置错误码

    if (output_len <= 0) {
        ucnv_close(from_conv);
        ucnv_close(to_conv);
        return "";
    }

    std::string output;
    output.resize(output_len);
    ucnv_fromUChars(to_conv, &output[0], output_len + 1, utf16_buf.get(), utf16_len, &status);

    // 清理资源
    ucnv_close(from_conv);
    ucnv_close(to_conv);

    if (U_FAILURE(status)) {
        return "";
    }

    return output;
}

std::string utf8ToGbk(const std::string& utf8_str) {
    // GBK在ICU中的名称是"GBK"
    return convert(utf8_str, "UTF-8", "GBK");
}

std::string gbkToUtf8(const std::string& gbk_str) {
    // GBK在ICU中的名称是"GBK"
    return convert(gbk_str, "GBK", "UTF-8");
}

std::string stringMid(const std::string& str, size_t position, int n) {
    // 处理起始位置超出字符串长度的情况，返回空字符串
    if (position >= str.length()) {
        return std::string{};
    }

    // 处理n为0的情况，返回空字符串
    if (n == 0) {
        return std::string{};
    }

    // 如果n为-1或超过剩余可用字符数，则提取到字符串末尾
    size_t available = str.length() - position;
    if (n == -1 || static_cast<size_t>(n) > available) {
        return str.substr(position);
    }

    // 正常提取n个字符
    return str.substr(position, static_cast<size_t>(n));
}

std::string stringMidChinese(const std::string& str, size_t position, int n) {
    std::string gbk = utf8ToGbk(str);
    position = position * 2;
    n = n * 2;
    std::string result = stringMid(gbk, position, n);
    return gbkToUtf8(result);
}

bool stringStartsWith(const std::string& str, const std::string& prefix) {
    // 如果前缀长度大于字符串长度，直接返回false
    if (prefix.length() > str.length()) {
        return false;
    }

    // 比较前prefix.length()个字符
    return str.substr(0, prefix.length()) == prefix;
}

std::string stringReplace(const std::string& str, const std::string& find, const std::string& replace) {
    std::string res(str);
    std::string::size_type pos = res.find(find);
    if (pos != std::string::npos) {
        res.replace(pos, find.size(), replace);
    }
    return res;
}

}  // namespace util
