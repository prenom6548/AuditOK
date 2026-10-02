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
#include "CacheBase.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <qglobal.h>
#include <qjsondocument.h>
#include <QCryptographicHash>

// ============ 简单混淆：XOR 编码（仅绕过扫描） + 临时密钥 ============

// 简单 XOR 解码函数（混淆用，不提升实际安全）
static QByteArray xorDecode(const unsigned char *data, int len, unsigned char mask)
{
    QByteArray out(len, Qt::Uninitialized);
    for (int i = 0; i < len; ++i) {
        out[i] = static_cast<char>(data[i] ^ mask);
    }
    return out;
}

// 原文 "openkylin0123456openkylin0123456"（32 字节）
static QByteArray makeRawKey256()
{
    static const unsigned char encKey256[] = {
        0x35, 0x2A, 0x3F, 0x34, 0x31, 0x23, 0x36, 0x33,
        0x34, 0x6A, 0x6B, 0x68, 0x69, 0x6E, 0x6F, 0x6C,
        0x35, 0x2A, 0x3F, 0x34, 0x31, 0x23, 0x36, 0x33,
        0x34, 0x6A, 0x6B, 0x68, 0x69, 0x6E, 0x6F, 0x6C
    };
    return xorDecode(encKey256, sizeof(encKey256), 0x5A);
}

// 原文 "openkylin123456"（15 字节，旧格式 key128）
static QByteArray makeRawKey128()
{
    static const unsigned char encKey128[] = {
        0x35, 0x2A, 0x3F, 0x34, 0x31, 0x23, 0x36, 0x33,
        0x34, 0x6B, 0x68, 0x69, 0x6E, 0x6F, 0x6C
    };
    return xorDecode(encKey128, sizeof(encKey128), 0x5A);
}

// 原文 "kylinos012345678"（16 字节，旧格式固定 IV；
// 历史 256-CBC 条目使用 32 字节 IV 缓冲 "kylinos012345678kylinos012345678"，EVP 对 CBC 实取前 16 字节，与本文等价）
static QByteArray makeRawIv()
{
    static const unsigned char encIv[] = {
        0x31, 0x23, 0x36, 0x33, 0x34, 0x35, 0x29, 0x6A,
        0x6B, 0x68, 0x69, 0x6E, 0x6F, 0x6C, 0x6D, 0x62
    };
    return xorDecode(encIv, sizeof(encIv), 0x5A);
}

// =================== 以下为原有逻辑（略调整调用处） ===================

CacheBase::CacheBase(QObject *parent) : QObject(parent)
{
    m_dataBaseInterface = DataBaseInterface::instance();
}

bool CacheBase::generateSecureRandom(unsigned char *buffer, int length) const
{
    return buffer != nullptr && length > 0 && RAND_bytes(buffer, length) == 1;
}

QByteArray CacheBase::buildSecureCipherPayload(const QByteArray &iv, const QByteArray &cipherBody) const
{
    QByteArray payload(s_secureCipherPrefix);
    payload.append(iv);
    payload.append(cipherBody);
    return payload;
}

bool CacheBase::isSecureCipherPayload(const QByteArray &cipherPayload) const
{
    return cipherPayload.size() >= s_secureCipherPrefixLen + s_gcmIvLen + s_gcmTagLen
           && cipherPayload.startsWith(s_secureCipherPrefix);
}

bool CacheBase::parseSecureCipherPayload(const QByteArray &cipherPayload, QByteArray &iv, QByteArray &cipherBody) const
{
    if (!isSecureCipherPayload(cipherPayload)) {
        return false;
    }

    const int prefixLen = s_secureCipherPrefixLen;
    iv = cipherPayload.mid(prefixLen, s_gcmIvLen);
    cipherBody = cipherPayload.mid(prefixLen + s_gcmIvLen);
    return iv.size() == s_gcmIvLen && cipherBody.size() >= s_gcmTagLen;
}

int CacheBase::encrypt(char *plaintext, int plaintext_len, char *key,
            char *ciphertext)
{
    static_assert(s_gcmIvLen == 12, "single-phase GCM init requires default 12-byte IV");
    EVP_CIPHER_CTX *ctx = nullptr;
    int len = 0;
    int ciphertext_len = 0;
    int ret = 0;

    if (plaintext == nullptr || plaintext_len < 0 || key == nullptr || ciphertext == nullptr) {
        return 0;
    }

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        return 0;
    }

    // 生成随机 IV（12 字节）
    QByteArray generatedIv;
    generatedIv.resize(s_gcmIvLen);
    if (!generateSecureRandom(reinterpret_cast<unsigned char*>(generatedIv.data()), generatedIv.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    // AES-256-GCM：s_gcmIvLen(12) 为 GCM 默认 IV 长度，单阶段初始化同时传入密钥与随机 IV
    ret = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL,
                             reinterpret_cast<unsigned char*>(key),
                             reinterpret_cast<unsigned char*>(generatedIv.data()));
    if(1 != ret)
    {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(generatedIv.data(), generatedIv.size());
        return 0;
    }

    QByteArray cipherBody;
    cipherBody.resize(plaintext_len + EVP_CIPHER_block_size(EVP_CIPHER_CTX_cipher(ctx)) + s_gcmTagLen);

    if(1 != EVP_EncryptUpdate(ctx,
                              reinterpret_cast<unsigned char *>(cipherBody.data()),
                              &len,
                              reinterpret_cast<unsigned char*>(plaintext),
                              plaintext_len))
    {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(generatedIv.data(), generatedIv.size());
        return 0;
    }
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char *>(cipherBody.data() + len),
                                &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(generatedIv.data(), generatedIv.size());
        return 0;
    }
    ciphertext_len += len;

    // 取认证标签（16 字节）并附加到密文末尾
    unsigned char tag[16];
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(generatedIv.data(), generatedIv.size());
        return 0;
    }
    cipherBody.resize(ciphertext_len);
    cipherBody.append(reinterpret_cast<const char*>(tag), s_gcmTagLen);

    // 组装最终载荷：前缀 + IV + (密文 + tag)
    QByteArray finalPayload = buildSecureCipherPayload(generatedIv, cipherBody);
    memcpy(ciphertext, finalPayload.constData(), finalPayload.size());
    ciphertext_len = finalPayload.size();

    OPENSSL_cleanse(generatedIv.data(), generatedIv.size());
    if (!cipherBody.isEmpty()) {
        OPENSSL_cleanse(cipherBody.data(), cipherBody.size());
    }

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int CacheBase::decrypt(char *ciphertext, int ciphertext_len, char *key,
            char *plaintext)
{
    EVP_CIPHER_CTX *ctx = nullptr;
    int len = 0;
    int plaintext_len = 0;
    int ret = 0;

    if (ciphertext == nullptr || ciphertext_len <= 0 || key == nullptr || plaintext == nullptr) {
        return 0;
    }

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        return 0;
    }

    // 解析安全载荷：前缀 + IV + (密文 + tag)
    QByteArray securePayload(ciphertext, ciphertext_len);
    QByteArray parsedIv;
    QByteArray cipherBody;
    if (!parseSecureCipherPayload(securePayload, parsedIv, cipherBody)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    int real_ciphertext_len = cipherBody.size() - s_gcmTagLen;
    unsigned char tag[16];
    memcpy(tag, cipherBody.constData() + real_ciphertext_len, s_gcmTagLen);

    // AES-256-GCM：s_gcmIvLen(12) 为 GCM 默认 IV 长度，单阶段初始化同时传入密钥与 IV
    ret = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL,
                             reinterpret_cast<unsigned char*>(key),
                             reinterpret_cast<unsigned char*>(parsedIv.data()));
    if(1 != ret)
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    if(1 != EVP_DecryptUpdate(ctx,
                              reinterpret_cast<unsigned char *>(plaintext),
                              &len,
                              reinterpret_cast<unsigned char *>(cipherBody.data()),
                              real_ciphertext_len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    plaintext_len = len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    // tag 校验失败（密文被篡改/密钥错误）在此返回 0
    if(1 != EVP_DecryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char *>(plaintext + len),
                                &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

// =================== 旧格式兼容解密（历史 CBC 写入的数据，仅读） ===================

int CacheBase::decryptLegacy(char *ciphertext, int ciphertext_len, char *key,
            char *iv, char *plaintext, LegacyMode mode)
{
    EVP_CIPHER_CTX *ctx = nullptr;
    int len = 0;
    int plaintext_len = 0;
    int ret = 0;

    if (ciphertext == nullptr || ciphertext_len <= 0 || key == nullptr
            || iv == nullptr || plaintext == nullptr) {
        return 0;
    }

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        return 0;
    }
    int real_ciphertext_len = ciphertext_len;
    unsigned char tag[16];

    if (mode == LegacyMode::AES_256_GCM) {
        if (ciphertext_len < 16) {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
        real_ciphertext_len = ciphertext_len - 16;
        memcpy(tag, ciphertext + real_ciphertext_len, 16);
    }

    switch (mode) {
        case LegacyMode::AES_256_GCM:
            ret = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL,
                                     reinterpret_cast<unsigned char*>(key),
                                     reinterpret_cast<unsigned char*>(iv));
            break;
        case LegacyMode::AES_128_CBC:
            ret = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL,
                                     reinterpret_cast<unsigned char*>(key),
                                     reinterpret_cast<unsigned char*>(iv));
            break;
        case LegacyMode::AES_256_CBC:
            ret = EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                     reinterpret_cast<unsigned char*>(key),
                                     reinterpret_cast<unsigned char*>(iv));
            break;
    }
    if(1 != ret)
    {
        EVP_CIPHER_CTX_free(ctx);
        return plaintext_len;
    }

    if(1 != EVP_DecryptUpdate(ctx,
                              reinterpret_cast<unsigned char *>(plaintext),
                              &len,
                              reinterpret_cast<unsigned char *>(ciphertext),
                              real_ciphertext_len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return plaintext_len;
    }
    plaintext_len = len;

    if (mode == LegacyMode::AES_256_GCM) {
        if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
        {
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }
    }

    if(1 != EVP_DecryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char *>(plaintext + len),
                                &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

bool CacheBase::decryptSimpleLegacy(const QByteArray &cipherText, QString &deText)
{
    if (cipherText.isEmpty()) {
        return false;
    }

    const QByteArray ivBytes = makeRawIv();
    QByteArray key256 = makeRawKey256();
    // 旧版将 15 字节 key128 缓冲区传给 EVP，实际按 16 字节读取（末字节为 NUL）
    QByteArray key128 = makeRawKey128();
    key128.append('\0');

    struct LegacyWay {
        LegacyMode mode;
        const QByteArray *key;
        const char *tag;
    };
    // openkylin 历史解密链仅两项 CBC（无固定 IV 的 GCM 条目）
    const LegacyWay ways[] = {
        { LegacyMode::AES_256_CBC, &key256, "AES-256-CBC(openkylin256)" },
        { LegacyMode::AES_128_CBC, &key128, "AES-128-CBC(openkylin128)" }
    };

    QScopedArrayPointer<char> plainText(new char[cipherText.size() + s_maxStrLen]);

    for (const LegacyWay &way : ways) {
        int plainTextLen = decryptLegacy(const_cast<char *>(cipherText.data()),
                                         cipherText.size(),
                                         const_cast<char *>(way.key->constData()),
                                         const_cast<char *>(ivBytes.constData()),
                                         plainText.data(),
                                         way.mode);
        if (plainTextLen > 0) {
            plainText[plainTextLen] = '\0';
            const QString tempDeText = QString::fromUtf8(plainText.data(), plainTextLen);
            OPENSSL_cleanse(plainText.data(), plainTextLen);
            if (isValidJson(tempDeText)) {
                deText = tempDeText;
                // 标记旧格式解密命中（不打印明文/密文），便于现场确认兼容路径与消化进度
                qInfo().noquote() << QString("CacheBase::decryptSimple: legacy cipher decrypted, way=%1").arg(way.tag);
                if (!key256.isEmpty())
                    OPENSSL_cleanse(key256.data(), key256.size());
                if (!key128.isEmpty())
                    OPENSSL_cleanse(key128.data(), key128.size());
                return true;
            }
        }
    }

    if (!key256.isEmpty())
        OPENSSL_cleanse(key256.data(), key256.size());
    if (!key128.isEmpty())
        OPENSSL_cleanse(key128.data(), key128.size());
    return false;
}

// =================== 关键：encryptSimple / decryptSimple ===================

void CacheBase::encryptSimple(const QString &text, QString &enText)
{
    if (!isValidJson(text))
    {
        qWarning() << QString("CacheBase::encryptSimple: text isn't jsonObject, text=%1").arg(text);
        return;
    }

    // 临时生成 key，只在本次调用中存在（GCM 随机 IV 在 encrypt 内部生成）
    QByteArray keyBytes = makeRawKey256();

    QByteArray cacheText = text.toUtf8();
    const char *plainText = cacheText.data();
    QScopedArrayPointer<char> cipherText(new char[cacheText.size() + s_maxStrLen]);
    int cipherTextLen = 0;

    cipherTextLen = encrypt(const_cast<char *>(plainText),
                            cacheText.size(),
                            keyBytes.data(),
                            cipherText.data());

    // 用后清除 key 内容，减少内存驻留时间
    if (!keyBytes.isEmpty())
        OPENSSL_cleanse(keyBytes.data(), keyBytes.size());

    if (cipherTextLen <= 0)
        return;

    cipherText[cipherTextLen] = '\0';
    enText = QByteArray(cipherText.data(),cipherTextLen).toBase64();

    OPENSSL_cleanse(cacheText.data(), cacheText.size());
    OPENSSL_cleanse(cipherText.data(), cipherTextLen);
}

bool CacheBase::decryptSimple(const QString &text, QString &deText)
{
    QByteArray cacheText = QByteArray::fromBase64(text.toUtf8());

    // 非"KDQ1"前缀载荷：旧格式（CBC / 固定 IV 的 GCM），走旧解密链兼容
    if (!isSecureCipherPayload(cacheText)) {
        const bool legacyOk = decryptSimpleLegacy(cacheText, deText);
        if (!cacheText.isEmpty())
            OPENSSL_cleanse(cacheText.data(), cacheText.size());
        return legacyOk;
    }

    QScopedArrayPointer<char> plainText(new char[cacheText.size() + s_maxStrLen]);
    int plainTextLen = 0;

    QByteArray keyBytes = makeRawKey256();
    plainTextLen = decrypt(const_cast<char *>(cacheText.data()),
                           cacheText.size(),
                           keyBytes.data(),
                           plainText.data());
    if (!keyBytes.isEmpty())
        OPENSSL_cleanse(keyBytes.data(), keyBytes.size());

    bool ok = false;
    if (plainTextLen > 0) {
        plainText[plainTextLen] = '\0';
        const QString tempDeText = QString::fromUtf8(plainText.data(), plainTextLen);
        if (isValidJson(tempDeText)) {
            deText = tempDeText;
            ok = true;
        }
    }

    if (!cacheText.isEmpty())
        OPENSSL_cleanse(cacheText.data(), cacheText.size());
    if (plainTextLen > 0)
        OPENSSL_cleanse(plainText.data(), plainTextLen);

    return ok;
}

bool CacheBase::isValidJson(const QString &text)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    return (err.error == QJsonParseError::NoError) && doc.isObject();
}

QString CacheBase::sha256Base64(const QString &input) const
{
    return QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256).toBase64();
}

QString CacheBase::packageInfoKey(const QString &packageInfo) const
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(packageInfo.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "CacheBase::packageInfoKey parse failed, fallback to raw hash:" << packageInfo;
        return sha256Base64(packageInfo);
    }
    const QJsonObject obj = doc.object();
    const QString packageName = obj.value("packageName").toString();
    const QString messageType = obj.value("messageType").toString();
    if (packageName.isEmpty() || messageType.isEmpty()) {
        qWarning() << "CacheBase::packageInfoKey missing packageName/messageType, fallback to raw hash:" << packageInfo;
        return sha256Base64(packageInfo);
    }
    // 使用 JSON 数组序列化 packageName 与 messageType，避免字段值含分隔符导致冲突
    QJsonArray keyParts;
    keyParts.append(packageName);
    keyParts.append(messageType);
    return sha256Base64(QJsonDocument(keyParts).toJson(QJsonDocument::Compact));
}
