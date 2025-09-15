#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-Music.
 *
 * HX-Music is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-Music is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-Music.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace HX::token {

class Base64 {
public:
    // Base64 编码
    static std::string encode(std::string_view data) {
        BIODeleter deleter;
        std::unique_ptr<BIO, BIODeleter> bio(BIO_new(BIO_s_mem()), deleter);
        std::unique_ptr<BIO, BIODeleter> b64(BIO_new(BIO_f_base64()), deleter);

        BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
        bio.reset(BIO_push(b64.release(), bio.release())); // b64 -> bio

        if (BIO_write(bio.get(), data.data(), data.size()) <= 0)
            throw std::runtime_error("BIO_write failed");

        if (BIO_flush(bio.get()) != 1)
            throw std::runtime_error("BIO_flush failed");

        BUF_MEM* bufferPtr{};
        BIO_get_mem_ptr(bio.get(), &bufferPtr);
        return std::string(bufferPtr->data, bufferPtr->length);
    }

    // Base64 解码
    static std::string decode(std::string encoded) {
        BIODeleter deleter;
        std::unique_ptr<BIO, BIODeleter> bio(BIO_new_mem_buf(encoded.data(), encoded.size()), deleter);
        std::unique_ptr<BIO, BIODeleter> b64(BIO_new(BIO_f_base64()), deleter);

        BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
        bio.reset(BIO_push(b64.release(), bio.release())); // b64 -> bio

        std::string decoded(encoded.size(), '\0');
        int len = BIO_read(bio.get(), decoded.data(), decoded.size());
        if (len < 0)
            throw std::runtime_error("BIO_read failed");

        decoded.resize(len);
        return decoded;
    }

private:
    struct BIODeleter {
        void operator()(BIO* bio) const {
            if (bio) {
                BIO_free_all(bio);
            }
        }
    };
};

class Base64Url : public Base64 {
public:
    // 编码为 Base64Url
    static std::string encodeUrl(std::string_view data) {
        std::string base64 = Base64::encode(data);

        // 替换 + -> -,  / -> _, 去掉末尾 =
        std::ranges::replace(base64, '+', '-');
        std::ranges::replace(base64, '/', '_');
        base64.erase(std::remove(base64.begin(), base64.end(), '='), base64.end());

        return base64;
    }

    // Base64Url 解码为原数据
    static std::string decodeUrl(std::string urlBase64) {
        // 替换 - -> +,  _ -> /
        std::ranges::replace(urlBase64, '-', '+');
        std::ranges::replace(urlBase64, '_', '/');

        // 补齐 = 到 4 的倍数
        while (urlBase64.size() % 4 != 0) {
            urlBase64 += '=';
        }

        return Base64::decode(std::move(urlBase64));
    }
};

} // namespace HX::token