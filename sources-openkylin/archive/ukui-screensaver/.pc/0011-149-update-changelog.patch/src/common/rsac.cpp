/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "rsac.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>

RSA *publicKeyToRSA(const QByteArray &pubKey)
{
    BIO *pBio = BIO_new_mem_buf(pubKey.data(), pubKey.size()); // 创建内存
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(pBio, nullptr, nullptr, nullptr);
    BIO_free_all(pBio); // 释放内存
    return rsa;
}

RSA *privateKeyToRSA(const QByteArray &priKey)
{
    BIO *pBio = BIO_new_mem_buf(priKey.data(), priKey.size()); // 创建内存
    RSA *rsa = PEM_read_bio_RSAPrivateKey(pBio, nullptr, nullptr, nullptr);
    BIO_free_all(pBio); // 释放内存
    return rsa;
}

/**
 * @brief RSAC::generateKeyPair
 * 生成密钥对，并分别保存为文件
 * @param priKeyFile 私钥文件名
 * @param pubKeyFile 公钥文件名
 * @param bits 秘钥长度，一般建议1024及以上
 */
void RSAC::generateKeyPair(const QString &priKeyFile, const QString &pubKeyFile, int bits)
{
    // 生成公钥
    RSA *rsa = RSA_generate_key(bits, RSA_F4, nullptr, nullptr);
    BIO *bp = BIO_new(BIO_s_file());
    BIO_write_filename(bp, (void *)pubKeyFile.toStdString().c_str());
    PEM_write_bio_RSAPublicKey(bp, rsa);
    BIO_free_all(bp);

    // 生成私钥
    bp = BIO_new(BIO_s_file());
    BIO_write_filename(bp, (void *)priKeyFile.toStdString().c_str());
    PEM_write_bio_RSAPrivateKey(bp, rsa, nullptr, nullptr, 0, nullptr, nullptr);
    CRYPTO_cleanup_all_ex_data();
    BIO_free_all(bp);
    RSA_free(rsa);
}

/**
 * @brief RSAC::generateKeyPair
 * 生成密钥对数据
 * @param privateKey 私钥数据
 * @param publicKey 公钥数据
 * @param bits 秘钥长度，一般建议1024及以上
 */
void RSAC::generateKeyPair(QByteArray &privateKey, QByteArray &pubKey, int bits)
{
    // 生成密钥对
    RSA *keyPair = RSA_generate_key(bits, RSA_F4, nullptr, nullptr);

    BIO *pri = BIO_new(BIO_s_mem());
    BIO *pub = BIO_new(BIO_s_mem());

    PEM_write_bio_RSAPrivateKey(pri, keyPair, nullptr, nullptr, 0, nullptr, nullptr);
    PEM_write_bio_RSA_PUBKEY(pub, keyPair);

    // 获取长度
    int pri_len = BIO_pending(pri);
    int pub_len = BIO_pending(pub);

    privateKey.resize(pri_len);
    pubKey.resize(pub_len);

    BIO_read(pri, privateKey.data(), pri_len);
    BIO_read(pub, pubKey.data(), pub_len);

    // 内存释放
    RSA_free(keyPair);
    BIO_free_all(pub);
    BIO_free_all(pri);
}

/**
 * @brief RSAC::encrypt
 * RSA加密函数，使用公钥对输入数据，进行加密
 * @param in 输入数据（明文）
 * @param out 输出数据（密文）
 * @param pubKey 公钥
 * @return 执行结果
 */
bool RSAC::encrypt(const QByteArray &in, QByteArray &out, const QByteArray &pubKey)
{
    // 公钥数据转RSA
    RSA *rsa = publicKeyToRSA(pubKey);
    if (rsa == nullptr) {
        return false;
    }

    // 对任意长度数据进行加密，超长时，进行分段加密
    int keySize = RSA_size(rsa);
    int dataLen = in.size();
    const unsigned char *from = (const unsigned char *)in.data();
    QByteArray to(keySize, 0);
    int readLen = 0;
    do {
        int select = (keySize - 11) > dataLen ? dataLen : (keySize - 11);
        RSA_public_encrypt(select, (from + readLen), (unsigned char *)to.data(), rsa, RSA_PKCS1_PADDING);
        dataLen -= select;
        readLen += select;
        out.append(to);
    } while (dataLen > 0);
    RSA_free(rsa);
    return true;
}

/**
 * @brief RSAC::private_decrypt
 * RSA解密函数，使用私钥对输入数据，进行解密
 * @param in 输入数据（密文）
 * @param out 输出数据（解密后的内容）
 * @param priKey 私钥
 * @return 执行结果
 */
bool RSAC::decrypt(const QByteArray &in, QByteArray &out, const QByteArray &priKey)
{
    // 私钥数据转RSA
    RSA *rsa = privateKeyToRSA(priKey);
    if (rsa == nullptr) {
        return false;
    }

    // 对任意长度数据进行解密，超长时，进行分段解密
    int keySize = RSA_size(rsa);
    int dataLen = in.size();
    const unsigned char *from = (const unsigned char *)in.data();
    QByteArray to(keySize, 0);
    int readLen = 0;
    do {
        int size = RSA_private_decrypt(keySize, (from + readLen), (unsigned char *)to.data(), rsa, RSA_PKCS1_PADDING);
        dataLen -= keySize;
        readLen += keySize;
        out.append(to.data(), size);
    } while (dataLen > 0);
    RSA_free(rsa);
    return true;
}

/**
 * @brief RSAC::sign
 * 使用私钥对摘要数据进行签名
 * @param digest 摘要数据
 * @param sign 签名后的数据
 * @param priKey 私钥
 * @return 执行结果
 */
bool RSAC::sign(const QByteArray &digest, QByteArray &sign, const QByteArray &priKey)
{
    // 私钥数据转RSA
    RSA *rsa = privateKeyToRSA(priKey);
    if (rsa == nullptr) {
        return false;
    }

    // 对digest进行签名
    unsigned int siglen = 0;
    QByteArray temp(RSA_size(rsa), 0);
    RSA_sign(NID_sha1, (const unsigned char *)digest.data(), digest.size(), (unsigned char *)temp.data(), &siglen, rsa);
    sign.clear();
    sign.append(temp.data(), siglen);
    RSA_free(rsa);
    return true;
}

/**
 * @brief RSAC::verify
 * 使用公钥对摘要数据进行验签
 * @param digest 摘要数据
 * @param sign 签名后的数据
 * @param pubKey 公钥
 * @return 执行结果
 */
bool RSAC::verify(const QByteArray &digest, const QByteArray &sign, const QByteArray &pubKey)
{
    // 公钥数据转RSA
    RSA *rsa = publicKeyToRSA(pubKey);
    if (rsa == nullptr) {
        return false;
    }

    // 对digest、sign进行验签
    int ret = RSA_verify(
        NID_sha1,
        (const unsigned char *)digest.data(),
        digest.size(),
        (const unsigned char *)sign.data(),
        sign.size(),
        rsa);
    RSA_free(rsa);
    return (ret == 1);
}
