/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CACHEBASE_H
#define CACHEBASE_H

#include <QObject>
#include <QTimer>
#include <QtSql/QSqlDatabase>
#include <QFileSystemWatcher>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <iostream>
#include <string>
#include <QStringList>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include "DataBaseInterface.h"
#include <QList>

class CacheBase : public QObject
{
    Q_OBJECT
protected:
    /**     
     * @brief 数据存储时刻键
     * 
     */
    static constexpr const char* s_createTimeStampKey = "CreateTimeStamp";
    /**
     * @brief 数据上传次数键
     * 
     */
    static constexpr const char* s_uploadTimesKey = "UploadTimes";
    /**
     * @brief 数据表Id字段
     * 
     */
    static constexpr const char* s_idKey = "Id";
    /**
     * @brief 数据表Value1字段
     * 
     */
    static constexpr const char* s_infoKey = "Value1";
    /**
     * @brief 数据表Value2字段
     * 
     */
    static constexpr const char* s_stateKey = "Value2";
    /**
     * @brief 上传数据表
     * 
     */
    static constexpr const char* s_uploadInfoTableName = "Data1";
    /**
     * @brief 公共数据表
     * 
     */
    static constexpr const char* s_commonInfoTableName = "Data2";
    /**
     * @brief 重传数据表
     * 
     */
    static constexpr const char* s_reuploadInfoTableName = "Data3";
    /**
     * @brief 上传数据表
     *
     */
    static constexpr const char* s_uploadEventInfoTableName = "eventMessage";

    /**
     * @brief 重传上传数据表
     *
     */
    static constexpr const char* s_reuploadEventInfoTableName = "eventReuploadMessage";

    /**
     * @brief tid数据表
     * 
     */
    static constexpr const char* s_tidInfoTableName = "Data4";
    /**
     * @brief 状态数据表
    */
    static constexpr const char* s_stateInfoTableName = "Data5";
    /**
     * @brief 状态数据表包信息
    */
    static constexpr const char* s_stateInfoTablePackageInfo = "Value1";
    /**
     * @brief 状态数据表状态信息
    */
    static constexpr const char* s_stateInfoTableStateInfo = "Value2";
    /**
     * @brief 状态数据表tid信息
    */
    static constexpr const char* s_stateInfoTableTidInfo = "Value3";
    /**
     * @brief 状态数据表时间戳信息
    */
    static constexpr const char* s_stateInfoTableCreateDateTimeInfo = "Value4";
    /**
     * @brief 状态数据表上传状态信息
     */
    static constexpr const char* s_stateInfoTableUploadStateInfo = "Value5";
    /**
     * @brief 状态数据表包信息密文（新增，主键 Value1 改为明文哈希）
     */
    static constexpr const char* s_stateInfoTablePackageInfoCipher = "Value6";

    /**
     * @brief 
     * 
     */
    static constexpr const qlonglong s_commonTablePrimaryKey = 0;
    /**
     * @brief 最大存储字符串长度
     * 
     */
    static constexpr const int32_t s_maxStrLen = 1024;
    /**
     * @brief 最大读取函数
     * 
     */
    static constexpr const int32_t s_checkInfoNum = 10000;
    static constexpr const int32_t s_singleNum = 1;
    /**
     * @brief 数据库操纵的对象
     * 
     */
    DataBaseInterface *m_dataBaseInterface;
    QString m_tableName;
    QString m_createTableCmd;
public:
    /**
     * @brief Construct a new Cache Base object
     * 
     * @param parent 
     */
    CacheBase(QObject *parent=nullptr);
//    CacheBase(QString tableName,QString createTableCmd,QObject *parent = nullptr);
//    //
//    virtual void createTable();
//    //	saveInfo
//    virtual bool saveInfo(const QString &info);
//    virtual bool saveInfo(const QList<QString> &info);
//    //	getInfo
//    virtual	bool getInfo(qlonglong &id, QString &info, QString &status);
//    virtual	bool getInfo(QList<qlonglong> &id, QList<QString> &info, QList<QString> &status,int32_t num);
//    //	replaceInfo
//    virtual bool replaceInfo(const qlonglong &id,const QString &info, const QString &status);
//    virtual bool replaceInfo(const QList<qlonglong> &id,const QList<QString> &info, const QList<QString> status);
//    //	updateInfo
//    virtual bool updateInfo(const qlonglong &id, const QString &info, const QString &status);
//    virtual bool updateInfo(const QList<qlonglong> &id, const QList<QString> &info, const QList<QString> &status);
//    //	deleteInfo
//    virtual bool deleteInfo(const qlonglong &id);
//    virtual bool deleteInfo(const QList<qlonglong> &id);
//    //	checkInfo
//    virtual bool checkRow(qlonglong cacheRows);
//    virtual bool checkTime(qlonglong storageDuration);
//    virtual bool checkTimes(qlonglong uploadTimes);
protected:
    static constexpr const char* s_secureCipherPrefix = "KDQ1";
    // 编译期长度常量，避免在运行时调用 strlen 扫描固定前缀（规避 CWE-120 热点）
    static constexpr int32_t s_secureCipherPrefixLen = 4;
    static constexpr int32_t s_gcmIvLen = 12;
    static constexpr int32_t s_gcmTagLen = 16;
    /**
     * @brief 简化aes加密函数（AES-256-GCM，随机IV）
     *
     * @param text 待加密文本
     * @param enText 加密文本
     */
    void encryptSimple(const QString& text,QString &enText);
    /**
     * @brief 简化的aes解密函数（AES-256-GCM，随机IV；兼容旧格式解密）
     *
     * 带"KDQ1"前缀的载荷走新格式解密；否则按旧格式链
     * （AES-256-CBC / AES-128-CBC）尝试解密
     *
     * @param text 待解密文本
     * @param deText 解密文本
     * @return true 解密成功
     * @return false 解密失败（校验失败或非合法JSON）
     */
    bool decryptSimple(const QString& text,QString &deText);
    bool isValidJson(const QString &text);
    bool isSecureCipherPayload(const QByteArray &cipherPayload) const;
    bool generateSecureRandom(unsigned char *buffer, int length) const;
    QByteArray buildSecureCipherPayload(const QByteArray &iv, const QByteArray &cipherBody) const;
    bool parseSecureCipherPayload(const QByteArray &cipherPayload, QByteArray &iv, QByteArray &cipherBody) const;
private:
    /**
     * @brief 旧格式加密模式（历史 CBC 写入格式，仅用于兼容解密）
     */
    enum class LegacyMode
    {
        AES_256_CBC,
        AES_128_CBC,
        AES_256_GCM
    };
    /**
     * @brief 旧格式 aes 解密函数（CBC / 固定 IV 的 GCM）
     *
     * @param ciphertext 待解密文本
     * @param ciphertext_len 待解密文本长度
     * @param key 密码
     * @param iv 初始向量
     * @param plaintext 解密文本
     * @param mode 旧格式加密模式
     * @return int 解密结果
     */
    int decryptLegacy(char *ciphertext, int ciphertext_len, char *key,
                char *iv, char *plaintext, LegacyMode mode);
    /**
     * @brief 旧格式解密入口：按原 oldDecryptWay 顺序尝试，解出合法 JSON 即成功
     *
     * @param cipherText 旧格式密文（base64 解码后）
     * @param deText 解密文本
     * @return true 解密成功
     * @return false 解密失败
     */
    bool decryptSimpleLegacy(const QByteArray &cipherText, QString &deText);
protected:
    /**
     * @brief 计算字符串的 SHA-256 摘要并返回 Base64 编码
     *
     * @param input 输入字符串
     * @return QString Base64 编码的 SHA-256 摘要
     */
    QString sha256Base64(const QString &input) const;
    /**
     * @brief 由 packageInfo 生成状态表主键（明文哈希）
     *
     * 取 packageInfo JSON 中的 packageName 与 messageType 字段，用 JSON 数组
     * `[packageName, messageType]` 紧凑序列化后做 SHA-256/Base64；解析失败或
     * 缺失字段则回退为原始字符串的 sha256Base64。
     *
     * @param packageInfo 明文 packageInfo JSON
     * @return QString 主键哈希
     */
    QString packageInfoKey(const QString &packageInfo) const;
protected:
    /**
     * @brief aes加密函数（AES-256-GCM，随机IV）
     *
     * @param plaintext 待加密文本
     * @param plaintext_len 文本长度
     * @param key 密码
     * @param ciphertext 加密的文本
     * @return int 加密结果
     */
    int encrypt(char *plaintext, int plaintext_len, char *key,
            char *ciphertext);
    /**
     * @brief aes解密函数（AES-256-GCM，随机IV）
     *
     * @param ciphertext 待解密文本
     * @param ciphertext_len 待解密文本长度
     * @param key 密码
     * @param plaintext 解密文本
     * @return int 解密结果
     */
    int decrypt(char *ciphertext, int ciphertext_len, char *key,
            char *plaintext);
signals:

};

#endif // CACHEBASE_H
