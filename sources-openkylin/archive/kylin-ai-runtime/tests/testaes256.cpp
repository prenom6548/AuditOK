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

#include "aes256.h"

#include <gtest/gtest.h>

#include <openssl/aes.h>

#include <memory>

TEST(Aes256Test, Encrypt) {
    // 256位密钥
    std::array<unsigned char, 32> keyData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1'
    };
    // 128位向量数据
    std::array<unsigned char, AES_BLOCK_SIZE> ivData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5'
    };

    Aes256 aes (keyData, ivData);

    // 待加密的数据
    std::string input = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello";
    
    auto encrypted = aes.encrypt(input);
    ASSERT_FALSE(encrypted.empty());
    EXPECT_NE(encrypted[0], 0);
}

TEST(Aes256Test, Decrypt) {
    // 256位密钥
    std::array<unsigned char, 32> keyData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1'
    };
    // 128位向量数据
    std::array<unsigned char, AES_BLOCK_SIZE> ivData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5'
    };

    std::unique_ptr<Aes256> aes = std::make_unique<Aes256>(keyData, ivData);

    // 待加密的数据
    std::string input = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello678901234";
    
    auto encrypted = aes->encrypt(input);
    auto decrypted = aes->decrypt(encrypted);
    ASSERT_FALSE(decrypted.empty());
    EXPECT_NE(decrypted[0], 0);
}

TEST(Aes256Test, EncryptAndDecrypt) {
    // 256位密钥
    std::array<unsigned char, 32> keyData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1'
    };
    // 128位向量数据
    std::array<unsigned char, AES_BLOCK_SIZE> ivData = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '0', '1', '2', '3', '4', '5'
    };

    std::unique_ptr<Aes256> aes = std::make_unique<Aes256>(keyData, ivData);

    // 待加密的数据
    std::string input = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello78901234";

    auto encrypted = aes->encrypt(input);
    auto decrypted = aes->decrypt(encrypted);
    ASSERT_FALSE(decrypted.empty());
    EXPECT_NE(decrypted[0], 0);
    EXPECT_EQ(input, decrypted);
}
