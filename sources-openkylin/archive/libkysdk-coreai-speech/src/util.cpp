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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "util.h"

#include <glib.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

namespace util {

std::string base64Encode(const void* data, std::size_t size) {
    if (data == nullptr) {
        return "";
    }

    BIO* bio{nullptr};
    BIO* b64{nullptr};
    BUF_MEM* bufferPtr{nullptr};

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, data, static_cast<int>(size));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

std::string base64Encode(const std::vector<uint8_t>& input) { return base64Encode(input.data(), input.size()); }

std::string base64Encode(const std::string& input) { return base64Encode(input.data(), input.size()); }
std::vector<uint8_t> base64Decode(const std::string& input) {
    gsize decodedLength{};
    guchar* decodedData = g_base64_decode(input.c_str(), &decodedLength);
    std::vector<uint8_t> result(decodedData, decodedData + decodedLength);
    g_free(decodedData);
    return result;
}

bool stringContains(const std::string& str, const std::string& sub) {
    if (str.length() < sub.length()) {
        return false;
    };
    return str.find(sub) != std::string::npos;
}

}  // namespace util