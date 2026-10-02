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

#include <gtest/gtest.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

#include <iomanip>
#include <sstream>
#include <string>

#include "rsa2048.h"

static const char *rsaPublicKey = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArw+9L+6kvFl5vTN5QeSW
PUH8fmDeT+UEqmmleYPZlGzUtVDVx+A7rwrhebm6wkEHTXlED2wAnNsksOYPQt9M
KWTMF+zuv8oWH8OfSzKC1QTTIY74KRJz4zkVHujJ87JcCw0gTiK1KQBebO0JJUWf
wrxnGh/sn3SeuWPbk44DpxfKzOBZDXEiWYGCMdWLRBGsyexj/a+Yhdj2/9yDHOaF
rQrMKRnipImjcZMLzWY5jxCTfoOY0AITM0PYQfIfhsSXNdXjh/DzO/SzQdUsba2f
vynrn0T2pNHB93kxFBzmxuAHlieuECVhLprhGYfEpxlKB4m4yLn9BVjFVbzaP329
UwIDAQAB
-----END PUBLIC KEY-----)";

static const char *rsaPrivateKey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEArw+9L+6kvFl5vTN5QeSWPUH8fmDeT+UEqmmleYPZlGzUtVDV
x+A7rwrhebm6wkEHTXlED2wAnNsksOYPQt9MKWTMF+zuv8oWH8OfSzKC1QTTIY74
KRJz4zkVHujJ87JcCw0gTiK1KQBebO0JJUWfwrxnGh/sn3SeuWPbk44DpxfKzOBZ
DXEiWYGCMdWLRBGsyexj/a+Yhdj2/9yDHOaFrQrMKRnipImjcZMLzWY5jxCTfoOY
0AITM0PYQfIfhsSXNdXjh/DzO/SzQdUsba2fvynrn0T2pNHB93kxFBzmxuAHlieu
ECVhLprhGYfEpxlKB4m4yLn9BVjFVbzaP329UwIDAQABAoIBAAzb7RuuPe+P6v9Y
3plWfyq0xCVMXEgmHHegFwWcB8l9TQqci3eJIxkyhWl5mRxXxn+MXtF7L3hWKJXN
8GULHf8UZmGDh+4SnBltNzBA35diKVgXtmbd8iFIuiJxGxSJj3VIW40hw4jlfAoo
RzmzqY6WZERo/+qemGIp0aE/F0mYuK+YTOqNaigeyIBs1bXzNrMe68E86WYNMvPd
201qHAjlaYAmTooJzFP9NNOwCbtPXdFB/sh4GEGSOpx2Gku509WqaP1z9bLNmqfl
PaW7403F0ulSCp51O+w/DcCu3MGczxzdqXvu4DkR6DcATC4LX2YTVNLJ4xH8KCjQ
Inu0GdECgYEA27JlyHxdeA0IFHc8a4bMoI/PJfIwUGvrcMFAaBIqjpJdF223CZG3
l43FMSMk7zxf5XQkTPuWEKVQSqC4YLU9imOJGhjp5UFRHXdjTJVsVSVlq9NraJQA
j+as/BrP+83WN1mBNVbTP8xg0zr74twk008EH951Kz0zgmCQ2Wx6gfcCgYEAy/0t
pE28TxHSanlUn4m+eOfQg0ZG4P2a8DmsopQtlU8kRbgZbOQBdkH0T9KEpRRpyVdH
Jq/J4likGGNk/gDZf+0WuQ/k9ZU2R4n/kMUitkBeARdrKQAghGnEH7du4Blk0ORw
h73gAGqEAowXyMfk9x9Uaa+8KK6OA9Y03kRbiIUCgYBT3eRRBni/7WmaNnJQMeNH
n95aA/xBlxEHuB4YHGc2idUTroRjGMdXCX2vZlUhJpENDKsW4CvHKvRdqMD52fYm
MB0UY1JWe/q1HvA/OgNi9aW8VDb6cxVL2alLjAi9fTQ7aOWUMRLvlduYhznEvnko
WU+fwtJDsVdKfAaPmYOivQKBgQCHEsqaNW8v3Fk8b8L9MTSS0s1bX7mnWcaysz4B
GaVlbNZT5CfR1tiw+dNQ19FFjlLOI6DVmSlRW+iszYQePWnIL7IQMNonU/pxPIuh
zSfDDrQwfNVUfOULSQ25ArnEYoxDVkC9YZlmLSHknedR6CtRB8nZgahSN+OJ3tl7
JECIEQKBgQCSna58O25Ap4khqbo4oAf/mXCZ4TAS4YibblTe4QGR7mvDKxn3xZQ4
91Z0P3qpdghy77Must9kNajoH5nwwQoHtLlaHTxCG+p6iNSwprFrlPifCUhM/GgH
/wVEa5KktoFLMyHbZD23P5ThE6+K96f92QQmHV/Vlzuq+R+o+KOJjw==
-----END RSA PRIVATE KEY-----)";

RSA *loadRsaPublicKeyFromMemory(const std::string &key) {
    BIO *bio = BIO_new_mem_buf(key.c_str(), key.length());
    if (bio == nullptr) {
        return nullptr;
    }

    auto *rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);

    BIO_free(bio);

    return rsa;
}
std::string toHex(const unsigned char *data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        if (i > 0 && i % 16 == 0) {
            ss << "\n    ";
        }
        ss << "0x" << std::setw(2) << static_cast<int>(data[i]);
        if (i % 16 != 15 && i != length - 1) {
            ss << ", ";
        }
    }
    return ss.str();
}
std::vector<unsigned char> encodedPbkdf2Key1 = {
    0xa5, 0x8b, 0x5a, 0x0a, 0xea, 0x96, 0xf6, 0xf4, 0x17, 0x09, 0x5d, 0xb3, 0xad, 0xbb, 0x00, 0x69, 0x39, 0x87, 0x61,
    0xe1, 0xbb, 0xc7, 0x0a, 0xd5, 0x2a, 0x8a, 0x38, 0x53, 0x6d, 0x73, 0xa2, 0x49, 0x21, 0x15, 0xd1, 0xeb, 0x6a, 0x7e,
    0x50, 0xcb, 0x3a, 0x07, 0xf9, 0x80, 0x25, 0x51, 0xa7, 0xa1, 0x37, 0x18, 0x3a, 0x41, 0x57, 0xfd, 0x93, 0x52, 0x21,
    0x10, 0xac, 0x5a, 0x7c, 0x67, 0xf7, 0x75, 0x19, 0x6f, 0xa3, 0xdf, 0xc3, 0x07, 0xaa, 0x1d, 0xe8, 0x86, 0x58, 0x6c,
    0xc6, 0x32, 0x85, 0x4d, 0xb8, 0x59, 0x1a, 0x97, 0x9c, 0x23, 0x0f, 0x8e, 0x8c, 0xbb, 0xd9, 0x90, 0x48, 0xb0, 0x2e,
    0xf2, 0x6c, 0x5b, 0x76, 0x4f, 0xb4, 0xcf, 0x94, 0xa3, 0xc5, 0x32, 0x7d, 0x7a, 0x85, 0xdc, 0x24, 0xc6, 0xdf, 0x8f,
    0x1c, 0x83, 0x77, 0x51, 0x73, 0x00, 0x2e, 0xa9, 0xb0, 0x9d, 0x8a, 0x76, 0xee, 0x80, 0xb3, 0x4e, 0x42, 0xca, 0x4a,
    0x5f, 0x17, 0x29, 0x0b, 0x76, 0x6c, 0x1f, 0x3b, 0x60, 0xea, 0xeb, 0xe5, 0xe2, 0x3a, 0x7f, 0xf7, 0x01, 0x93, 0x68,
    0x1e, 0x3f, 0x68, 0xc0, 0x5c, 0xc9, 0x82, 0x80, 0x0f, 0xd9, 0x79, 0xec, 0xce, 0x46, 0x0d, 0x45, 0x75, 0x32, 0xd3,
    0xd6, 0x82, 0x79, 0xc1, 0xfc, 0x8f, 0x3e, 0x7f, 0x65, 0x81, 0x5d, 0xc5, 0xc6, 0x7d, 0xb9, 0x56, 0xbf, 0xee, 0x32,
    0xf1, 0x1a, 0xd5, 0x9e, 0x08, 0x32, 0x6a, 0xff, 0x33, 0xbc, 0x82, 0x8f, 0x62, 0xe2, 0x77, 0x97, 0x9f, 0xbe, 0xfa,
    0xb9, 0x83, 0xab, 0x5b, 0xfe, 0x8d, 0x95, 0xf4, 0xc4, 0x9c, 0xc5, 0x98, 0xd2, 0x96, 0xd4, 0xf1, 0x85, 0xd6, 0xfb,
    0xc1, 0xb6, 0xad, 0xfa, 0xc9, 0x1d, 0xdd, 0xf5, 0x53, 0x6e, 0x67, 0x0b, 0x48, 0xff, 0x42, 0xe9, 0xec, 0xf8, 0x4a,
    0x72, 0xa9, 0xb0, 0x7f, 0x52, 0x7b, 0xb8, 0x61, 0x7e};
TEST(Rsa2048Test, PublicEncryptPrivateDecrypt) {
    RSA *publicKey = loadRsaPublicKeyFromMemory(rsaPublicKey);
    ASSERT_NE(publicKey, nullptr);

    // 获取明文数据并进行加密
    std::string plainText = "Hello, Kylin AI SDK!";
    unsigned char encryptedData[RSA_size(publicKey)];

    int encLength = RSA_public_encrypt(static_cast<int>(plainText.size()),
                                       reinterpret_cast<const unsigned char *>(plainText.c_str()), encryptedData,
                                       publicKey, RSA_PKCS1_OAEP_PADDING);

    ASSERT_NE(encLength, -1);
    std::string hexEncryptedData = toHex(encryptedData, encLength);
    printf("Encrypted Data (Hex):\n    %s\n", hexEncryptedData.c_str());

    Rsa2048 rsa(rsaPrivateKey);
    auto text = rsa.decrypt(std::vector<unsigned char>(encryptedData, encryptedData + encLength));
    EXPECT_EQ(text, plainText);
}
