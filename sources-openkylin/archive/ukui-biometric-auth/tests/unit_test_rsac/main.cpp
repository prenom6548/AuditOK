/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#include <gtest/gtest.h>
#include "../../uniauth-backend/src/rsac.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <QByteArray>
#include <QString>
#include <iostream>
#include <QFile>
 
// 辅助函数，用于初始化OpenSSL库
void InitOpenSSL() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}
 
// 辅助函数，用于清理OpenSSL库
void CleanupOpenSSL() {
    EVP_cleanup();
    ERR_free_strings();
}
 
// 测试类
class RSACTest : public ::testing::Test {
protected:
    void SetUp() override {
        InitOpenSSL();
    }
 
    void TearDown() override {
        CleanupOpenSSL();
    }
 
    RSAC rsac;
};
 
// 测试密钥生成并保存到文件
TEST_F(RSACTest, GenerateKeyPairToFile) {
    QString priKeyFile = "test_private.pem";
    QString pubKeyFile = "test_public.pem";
    rsac.generateKeyPair(priKeyFile, pubKeyFile, 1024);
 
    // 这里可以添加额外的检查来验证文件是否成功生成且内容有效
    // 例如，打开文件并读取内容，检查是否为有效的PEM格式密钥
    // 但由于这是一个简单的测试示例，我们省略了这些步骤
}
 
// 测试密钥生成并保存到QByteArray
TEST_F(RSACTest, GenerateKeyPairToByteArray) {
    QByteArray privateKey;
    QByteArray publicKey;
    rsac.generateKeyPair(privateKey, publicKey, 1024);
 
    // 检查生成的密钥是否非空
    EXPECT_FALSE(privateKey.isEmpty());
    EXPECT_FALSE(publicKey.isEmpty());
 
    // 这里可以添加额外的检查来验证密钥的有效性
    // 例如，使用OpenSSL函数解析生成的密钥并检查其属性
    // 但由于这是一个简单的测试示例，我们省略了这些步骤
}
 
// 测试加密和解密
TEST_F(RSACTest, EncryptDecrypt) {
    QByteArray privateKey;
    QByteArray publicKey;
    rsac.generateKeyPair(privateKey, publicKey, 1024);
 
    QByteArray plaintext = "Hello, RSA!";
    QByteArray ciphertext;
    QByteArray decryptedtext;
 
    bool encryptResult = rsac.encrypt(plaintext, ciphertext, publicKey);
    EXPECT_TRUE(encryptResult);
 
    bool decryptResult = rsac.decrypt(ciphertext, decryptedtext, privateKey);
    EXPECT_TRUE(decryptResult);
 
    EXPECT_EQ(plaintext, decryptedtext);
}
 
// 测试签名和验签
TEST_F(RSACTest, SignVerify) {
    QByteArray privateKey;
    QByteArray publicKey;
    rsac.generateKeyPair(privateKey, publicKey, 1024);
 
    QByteArray message = "This is a test message.";
    QByteArray signature;
    bool signResult = rsac.sign(message, signature, privateKey);
    EXPECT_TRUE(signResult);
 
    bool verifyResult = rsac.verify(message, signature, publicKey);
    EXPECT_TRUE(verifyResult);
 
    // 尝试使用错误的消息进行验签，应该失败
    bool verifyWithWrongMessageResult = rsac.verify("Wrong message.", signature, publicKey);
    EXPECT_FALSE(verifyWithWrongMessageResult);
}
 
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
