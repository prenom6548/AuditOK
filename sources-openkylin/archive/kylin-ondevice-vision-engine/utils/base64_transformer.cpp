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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>  // 添加这个头文件
#include <opencv2/opencv.hpp>
#include "base64_transformer.h"

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
    std::string ret;
    int val = 0, valb = -6;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = bytes_to_encode[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        ret.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (ret.size() % 4) {
        ret.push_back('=');
    }
    return ret;
}

std::string base64_decode(const std::string& encoded_string) {
    std::vector<unsigned char> decoded;
    std::vector<int> T(256, -1);
    for (size_t i = 0; i < base64_chars.size(); i++) {
        T[base64_chars[i]] = i;
    }
    int val = 0, valb = -8;
    for (size_t i = 0; i < encoded_string.size(); i++) {
        unsigned char c = encoded_string[i];
        if (c == '=') break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return std::string(decoded.begin(), decoded.end());
}

std::string MatToBase64(const cv::Mat& image) {
    std::vector<uchar> buf;
    cv::imencode(".png", image, buf);
    return base64_encode(buf.data(), buf.size());
}

cv::Mat Base64ToMat(const std::string& base64_string) {
    if (base64_string.size() < 1)
        return cv::Mat();
    std::string decoded_data = base64_decode(base64_string);
    std::vector<uchar> data(decoded_data.begin(), decoded_data.end());
    return cv::imdecode(data, cv::IMREAD_COLOR);
}

// 读取文件并转换为 Base64 编码
std::string file_to_base64(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);  // 打开文件为二进制模式
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << file_path << std::endl;
        return "";
    }

    // 读取文件内容
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string file_contents = oss.str();
    
    // 将文件内容转换为 Base64 编码
    return base64_encode(reinterpret_cast<const unsigned char*>(file_contents.c_str()), file_contents.size());
}

// 将 Base64 编码转换回文件
bool base64_to_file(const std::string& base64_string, const std::string& output_file_path) {
    std::string decoded_data = base64_decode(base64_string);

    std::ofstream output_file(output_file_path, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error opening output file: " << output_file_path << std::endl;
        return false;
    }

    output_file.write(decoded_data.c_str(), decoded_data.size());
    return true;
}