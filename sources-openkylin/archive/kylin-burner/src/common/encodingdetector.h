#ifndef ENCODINGDETECTOR_H
#define ENCODINGDETECTOR_H

#include <QByteArray>
#include <QString>

enum class EncodingType {
    UTF8,
    GBK,
    ASCII,
    Invalid
};

/**
 * @brief 编码检测类（适配 C 风格字符数组/Qt 字节数组，专注卷标名检测）
 */
class EncodingDetector
{
public:
    EncodingDetector() = default;
    EncodingDetector(const EncodingDetector&) = delete;
    EncodingDetector& operator=(const EncodingDetector&) = delete;

    /**
     * @brief 检测 C 风格字符数组的编码
     * @param charData 原始字符数组
     * @param arrayLen 字符数组总长度
     * @return 编码类型
     */
    EncodingType detect(const char* charData, int arrayLen);

    /**
     * @brief 检测 Qt 字节数组的编码
     * @param data 原始字节数组
     * @return 编码类型
     */
    EncodingType detect(const QByteArray &data);

    /**
     * @brief 解码 C 风格字符数组为 QString
     * @param charData 原始字符数组
     * @param arrayLen 字符数组总长度
     * @return 解码后的字符串
     */
    QString decode(const char* charData, int arrayLen);

    /**
     * @brief 解码 Qt 字节数组为 QString
     * @param data 原始字节数组
     * @return 解码后的字符串
     */
    QString decode(const QByteArray &data);

    /**
     * @brief 将 C 风格字符数组转换为有效 QByteArray（剔除末尾空字符/无效字节）
     * @param charData 原始字符数组
     * @param arrayLen 数组总长度
     * @return 有效字节数组
     */
    QByteArray toValidByteArray(const char* charData, int arrayLen) const;


    QString encodingTypeToString(EncodingType type);
public:

    /**
     * @brief 校验 UTF-8 合法性
     */
    bool isUtf8Valid(const QByteArray &data) const;

    /**
     * @brief 校验 GBK 合法性
     */
    bool isGbkValid(const QByteArray &data) const;

    /**
     * @brief 判断是否为纯 ASCII
     */
    bool isAscii(const QByteArray &data) const;
};

#endif // ENCODINGDETECTOR_H
