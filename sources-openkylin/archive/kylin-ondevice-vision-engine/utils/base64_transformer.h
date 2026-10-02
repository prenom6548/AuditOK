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
#pragma once

#include <string>
#include <vector>

// 函数声明
bool is_base64(unsigned char c);

std::string MatToBase64(const cv::Mat& image);

cv::Mat Base64ToMat(const std::string& base64_string);

std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len);

std::string base64_decode(const std::string& encoded_string);

std::string file_to_base64(const std::string& file_path);

bool base64_to_file(const std::string& base64_string, const std::string& output_file_path);