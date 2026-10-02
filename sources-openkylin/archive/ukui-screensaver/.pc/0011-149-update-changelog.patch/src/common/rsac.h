/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef RSAC_H
#define RSAC_H

#include <QString>

/**
 * @brief The RSAC class
 * RSA算法相关实现，包括密钥对生成，加密与解密，签名与验签。
 */
class RSAC
{
public:
    // 生成秘钥对
    void generateKeyPair(const QString &priKeyFile, const QString &pubKeyFile, int bits = 1024);
    void generateKeyPair(QByteArray &priKey, QByteArray &pubKey, int bits = 1024);

    // 对数据进行加解密
    bool encrypt(const QByteArray &in, QByteArray &out, const QByteArray &pubKey);
    bool decrypt(const QByteArray &in, QByteArray &out, const QByteArray &priKey);

    // 对摘要进行签名和验签
    bool sign(const QByteArray &digest, QByteArray &sign, const QByteArray &priKey);
    bool verify(const QByteArray &digest, const QByteArray &sign, const QByteArray &pubKey);
};

#endif // RSAC_H
