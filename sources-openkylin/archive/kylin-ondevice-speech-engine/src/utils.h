#pragma once

#include <jsoncpp/json/json.h>

#include <string>
#include <vector>

namespace Utils {

std::string generateUUID();

uint64_t generateRandomNumber();

int ceilDivide(int a, int b);

Json::Value formatJsonFromString(const std::string &stringJson);

std::string base64Encode(const std::string &input);

std::string base64Decode(const std::string &input);

std::vector<uint8_t> resampleAudio(const std::vector<uint8_t> &inputData, int inputRate, int outputRate);

std::string removeDoubleAt(const std::string &input);

size_t utf8CharLen(unsigned char ch);

size_t utf8Prev(const std::string &s, size_t pos);

std::string getUtf8Char(const std::string &s, size_t pos);

bool inList(const std::string &c, const std::vector<std::string> &list);

size_t countUtf8Chars(const std::string &text, size_t start, size_t maxChars);

bool findOldSplitPosition(const std::string &text,
                    size_t searchStart,  // 搜索起点（一般为 endPos）
                    size_t searchEnd,    // 搜索终点（一般为 endPos - searchWindow）
                    const std::vector<std::string> &puncts, const std::vector<std::string> &allPuncts, bool allowSpace,
                    size_t &splitPos);

bool findNewSplitPosition(const std::string &text,
                   size_t searchStart,  // 搜索起点（一般为 startPos）
                   size_t searchEnd,    // 搜索终点（一般为 startPos + searchWindow）
                   const std::vector<std::string> &puncts, const std::vector<std::string> &allPuncts, bool allowSpace,
                   size_t &splitPos);

std::vector<std::string> splitText(const std::string &text,
                                   size_t maxChars = 50,      // 多少字开始切
                                   size_t searchWindow = 100  // 前后搜索标点多少字
);

std::vector<uint8_t> normalizePcm16Rms(const std::vector<uint8_t>& inputData, float targetDbfs = -20.0f,
                                       float maxGainDb = 60.0f);// 输入参数分别是刚返回的音频，目标音量，为了达到目标音量可以改动的范围

}  // namespace Utils