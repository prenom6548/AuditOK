#include "encodingdetector.h"
#include "src/frame/logger.h"

#include <QDebug>
#include <QTextCodec>
#include <cstring>
#include <cctype>

/**
 * @brief 内部方法：C 字符数组转有效 QByteArray（处理卷标数组的空字符和无效字节）
 */
QByteArray EncodingDetector::toValidByteArray(const char* charData, int arrayLen) const
{
    if (charData == nullptr || arrayLen <= 0) {
        return QByteArray();
    }

    // 步骤1：找到第一个空终止符的位置（卷标名以 \0 结尾）
    int validLen = 0;
    while (validLen < arrayLen) {
        if (charData[validLen] == '\0') {
            break;
        }
        validLen++;
    }

    // 步骤2：过滤全空白/无效字节（可选，卷标名通常无此情况）
    while (validLen > 0 && isspace(static_cast<unsigned char>(charData[validLen - 1]))) {
        validLen--;
    }

    // 步骤3：转换为 QByteArray（仅保留有效长度）
    return QByteArray(charData, validLen);
}

QString EncodingDetector::encodingTypeToString(EncodingType type)
{
    switch (type) {
    case EncodingType::UTF8:
        return "UTF8";
    case EncodingType::GBK:
        return "GBK";
    case EncodingType::ASCII:
        return "ASCII";
    case EncodingType::Invalid:
        return "Invalid";
    default:
        return "Unknown";
    }
}

/**
 * @brief 校验 UTF-8 合法性
 */
bool EncodingDetector::isUtf8Valid(const QByteArray &data) const
{
    int len = data.length();
    int i = 0;
    while (i < len) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        int bytesNeeded = 0;

        if ((c & 0x80) == 0) {         // 单字节: 0xxxxxxx
            bytesNeeded = 1;
        } else if ((c & 0xE0) == 0xC0) { // 双字节: 110xxxxx
            bytesNeeded = 2;
        } else if ((c & 0xF0) == 0xE0) { // 三字节: 1110xxxx
            bytesNeeded = 3;
        } else if ((c & 0xF8) == 0xF0) { // 四字节: 11110xxx
            bytesNeeded = 4;
        } else {                        // 无效首字节
            return false;
        }

        if (i + bytesNeeded > len) {
            return false;
        }

        for (int j = 1; j < bytesNeeded; ++j) {
            unsigned char next = static_cast<unsigned char>(data[i + j]);
            if ((next & 0xC0) != 0x80) {
                return false;
            }
        }

        i += bytesNeeded;
    }
    return true;
}

/**
 * @brief 校验 GBK 合法性
 */
bool EncodingDetector::isGbkValid(const QByteArray &data) const
{
    int len = data.length();
    int i = 0;
    while (i < len) {
        unsigned char c = static_cast<unsigned char>(data[i]);

        if (c <= 0x7F) { // 单字节 GBK（ASCII）
            i += 1;
            continue;
        }

        // 双字节 GBK 规则：首字节 0x81-0xFE，次字节 0x40-0xFE（排除0x7F）
        if ((c >= 0x81 && c <= 0xFE) && (i + 1 < len)) {
            unsigned char c2 = static_cast<unsigned char>(data[i + 1]);
            if ((c2 >= 0x40 && c2 <= 0xFE) && (c2 != 0x7F)) {
                i += 2;
                continue;
            }
        }

        return false;
    }
    return true;
}

/**
 * @brief 判断是否为纯 ASCII
 */
bool EncodingDetector::isAscii(const QByteArray &data) const
{
    for (char c : data) {
        if (static_cast<unsigned char>(c) >= 0x80) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 检测 C 风格字符数组的编码（核心接口）
 */
EncodingType EncodingDetector::detect(const char* charData, int arrayLen)
{
    QByteArray validData = toValidByteArray(charData, arrayLen);
    return detect(validData);
}

/**
 * @brief 检测 Qt 字节数组的编码
 */
EncodingType EncodingDetector::detect(const QByteArray &data)
{
    if (data.isEmpty()) {
        return EncodingType::Invalid;
    }

    if (isAscii(data)) {
        return EncodingType::ASCII;
    }

    bool utf8Valid = isUtf8Valid(data);
    bool gbkValid = isGbkValid(data);

    if (utf8Valid && !gbkValid) {
        return EncodingType::UTF8;
    } else if (!utf8Valid && gbkValid) {
        return EncodingType::GBK;
    } else if (utf8Valid && gbkValid) {
        return EncodingType::UTF8; // 卷标场景优先 UTF8
    } else {
        return EncodingType::Invalid;
    }
}

/**
 * @brief 解码 C 风格字符数组
 */
QString EncodingDetector::decode(const char* charData, int arrayLen)
{
    QByteArray validData = toValidByteArray(charData, arrayLen);
    return decode(validData);
}

/**
 * @brief 解码 Qt 字节数组
 */
QString EncodingDetector::decode(const QByteArray &data)
{
    QString volumeID = QString();
    EncodingType type = detect(data);

    switch (type) {
    case EncodingType::UTF8:
        volumeID = QString::fromUtf8(data);
        break;
    case EncodingType::GBK: {
        QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
        volumeID = gbkCodec->toUnicode(data);
        break;
    }
    default:
        break;
    }

    LOGGER->info(QString("volume encode type is : %1, volume: %2").arg(encodingTypeToString(type)).arg(volumeID));
    return volumeID;
}
