/*
 * Copyright 2026 KylinSoft Co., Ltd.
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
#include "universal_image_reader.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

std::string getFormatFromData(const std::vector<unsigned char>& data) {
    if (data.size() < 4) {
        return "jpg";
    }

    const unsigned char* d = data.data();

    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (data.size() >= 8 && d[0] == 0x89 && d[1] == 0x50 && d[2] == 0x4E && d[3] == 0x47 && d[4] == 0x0D &&
        d[5] == 0x0A && d[6] == 0x1A && d[7] == 0x0A) {
        return "png";
    }

    // JPEG: FF D8 FF ...
    if (d[0] == 0xFF && d[1] == 0xD8 && d[2] == 0xFF) {
        return "jpeg";
    }

    // GIF: "GIF87a" or "GIF89a"
    if (data.size() >= 6 && d[0] == 'G' && d[1] == 'I' && d[2] == 'F' && d[3] == '8' && (d[4] == '7' || d[4] == '9') &&
        d[5] == 'a') {
        return "gif";
    }

    // BMP: "BM"
    if (data.size() >= 2 && d[0] == 'B' && d[1] == 'M') {
        return "bmp";
    }

    // WebP: RIFF + ... + WEBP
    if (data.size() >= 12 && d[0] == 'R' && d[1] == 'I' && d[2] == 'F' && d[3] == 'F' && d[8] == 'W' && d[9] == 'E' &&
        d[10] == 'B' && d[11] == 'P') {
        return "webp";
    }

    // TIFF (little-endian): II 2A 00
    if (data.size() >= 4 && d[0] == 0x49 && d[1] == 0x49 && d[2] == 0x2A && d[3] == 0x00) {
        return "tiff";
    }
    // TIFF (big-endian): MM 00 2A
    if (data.size() >= 4 && d[0] == 0x4D && d[1] == 0x4D && d[2] == 0x00 && d[3] == 0x2A) {
        return "tiff";
    }

    return "jpg";
}

uint32_t readLE32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); }

void writeLE32(uint8_t* p, uint32_t val) {
    p[0] = val & 0xFF;
    p[1] = (val >> 8) & 0xFF;
    p[2] = (val >> 16) & 0xFF;
    p[3] = (val >> 24) & 0xFF;
}

cv::Mat readAnimatedWebPWithOpenCV(const std::vector<uint8_t>& data) {
    // 检查最小长度：至少需要 68 + 4(tag) + 4(size) = 76 字节
    if (data.size() < 76) {
        std::cerr << "Input data too small.\n";
        return cv::Mat();
    }

    // 跳到 VP8L 块开始（offset=68）
    const uint8_t* p = data.data() + 68;

    // 读 tag
    std::string tag(reinterpret_cast<const char*>(p), 4);
    if (tag != "VP8L") {
        std::cerr << "Expected VP8L, got: " << tag << "\n";
        return cv::Mat();
    }

    // 读 size (little-endian)
    uint32_t payload_size = readLE32(p + 4);
    std::cout << "VP8L payload size: " << payload_size << "\n";

    // 检查 payload 是否在数据范围内
    size_t payload_start = 68 + 8;  // 68 (header) + 4 (tag) + 4 (size)
    if (payload_start + payload_size > data.size()) {
        std::cerr << "Payload exceeds input data size.\n";
        return cv::Mat();
    }

    // 提取 payload
    const uint8_t* payload_ptr = data.data() + payload_start;

    // 构造完整 WebP 文件
    uint32_t chunk_size = 8 + payload_size;  // "VP8L" + size + payload
    uint32_t riff_size = 4 + chunk_size;     // "WEBP" + chunk

    std::vector<uint8_t> new_webp;
    new_webp.resize(12 + chunk_size);

    // RIFF header
    memcpy(new_webp.data(), "RIFF", 4);
    writeLE32(new_webp.data() + 4, riff_size);
    memcpy(new_webp.data() + 8, "WEBP", 4);

    // VP8L chunk
    memcpy(new_webp.data() + 12, "VP8L", 4);
    writeLE32(new_webp.data() + 16, payload_size);
    memcpy(new_webp.data() + 20, payload_ptr, payload_size);

    return cv::imdecode(new_webp, cv::IMREAD_UNCHANGED);
}

cv::Mat ensureBGR(const cv::Mat& src_img) {
    if (src_img.empty()) {
        return cv::Mat();  // 返回空 Mat，避免后续操作崩溃
    }

    cv::Mat bgr;

    switch (src_img.channels()) {
        case 4:
            cv::cvtColor(src_img, bgr, cv::COLOR_BGRA2BGR);
            break;
        case 3:
            bgr = src_img.clone();  // 避免修改原图
            break;
        case 1:
            cv::cvtColor(src_img, bgr, cv::COLOR_GRAY2BGR);
            break;
        default:
            // 不支持的通道数（如 2 通道），返回空 Mat 或抛出异常
            std::cerr << "Unsupported number of channels: " << src_img.channels() << "\n";
            return cv::Mat();
    }

    return bgr;
}

cv::Mat loadImageFromData(const std::vector<uchar>& data, const std::string& format) {
    if (data.empty()) {
        std::cerr << "Error: Input data is empty\n";
        return cv::Mat();
    }
    cv::Mat src_img = cv::imdecode(data, cv::IMREAD_COLOR);
    // 目前存在一种webp的下属格式无法读取，需要先判断其无法被opencv识别，再进行进一步处理
    if (src_img.empty() && format == "webp") {
        src_img = readAnimatedWebPWithOpenCV(data);
    }
    return src_img;
}

std::vector<uchar> readFile(const std::string& filepath) {
    // 以二进制模式打开文件，并直接定位到末尾以获取大小
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filepath << "'\n";
        return {};
    }

    // 获取文件大小
    std::streamsize size = file.tellg();
    if (size < 0) {
        std::cerr << "Error: Cannot determine file size\n";
        return {};
    }

    // 返回到文件开头
    file.seekg(0, std::ios::beg);

    // 预分配 vector 空间
    std::vector<uchar> buffer(static_cast<size_t>(size));

    // 一次性读取全部内容
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Error: Failed to read file content\n";
        return {};
    }

    return buffer;
}