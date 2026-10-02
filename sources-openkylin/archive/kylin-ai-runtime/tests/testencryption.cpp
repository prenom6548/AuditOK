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
#include <openssl/evp.h>
#include <string.h>

#include "encryption.h"

TEST(EncryptionTest, Base64Encode) {
    std::string str = "Hello, World!";
    std::vector<unsigned char> from(str.begin(), str.end());

    auto result = Encryption::base64Encode(from);
    EXPECT_EQ(result, "SGVsbG8sIFdvcmxkIQ==");
}

TEST(EncryptionTest, Base64Decode) {
    std::string str = "Hello, World!";
    std::vector<unsigned char> from(str.begin(), str.end());

    auto encoded = Encryption::base64Encode(from);
    auto decoded = Encryption::base64Decode(encoded);

    EXPECT_EQ(from, decoded);
}

TEST(EncryptionTest, EncryptAndDecrypt) {
    const char* testKey = "TestKey";
    const auto& encryption = Encryption::getInstance();
    auto encrypted = encryption.encrypt(testKey);
    auto decrypted = encryption.decrypt(encrypted);

    EXPECT_EQ(decrypted, testKey);
}
