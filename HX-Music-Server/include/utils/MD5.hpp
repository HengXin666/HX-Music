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

#include <stdexcept>
#include <string>
#include <array>

#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>

#include <HXLibs/utils/NumericBaseConverter.hpp>

namespace HX::utils {

class Md5Hasher {
public:
    Md5Hasher() {
        // 创建EVP上下文
        _ctx = EVP_MD_CTX_new();
        if (!_ctx) {
            throw std::runtime_error("EVP_MD_CTX_new failed");
        }

        // 初始化, 选择MD5算法
        if (EVP_DigestInit_ex(_ctx, EVP_md5(), nullptr) != 1) {
            EVP_MD_CTX_free(_ctx);
            throw std::runtime_error("EVP_DigestInit_ex failed");
        }
    }

    Md5Hasher& operator=(Md5Hasher&&) noexcept = delete;

    void update(const std::string& data) {
        if (EVP_DigestUpdate(_ctx, data.data(), data.size()) != 1) {
            throw std::runtime_error("EVP_DigestUpdate failed");
        }
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> finalizeRaw(std::size_t& outLen) {
        std::array<unsigned char, EVP_MAX_MD_SIZE> res{};
        unsigned int len = 0;
        if (EVP_DigestFinal_ex(_ctx, res.data(), &len) != 1) {
            throw std::runtime_error("EVP_DigestFinal_ex failed");
        }

        outLen = len;
        return res;
    }

    std::string finalizeHex() {
        std::size_t outLen = 0;
        auto raw = finalizeRaw(outLen);
        std::string res{};
        res.reserve(outLen * 2);
        for (auto v : raw) {
            res += utils::NumericBaseConverter::hexadecimalConversion(v);
        }
        return res;
    }

    ~Md5Hasher() {
        cleanup();
    }

private:
    void cleanup() {
        if (_ctx) {
            EVP_MD_CTX_free(_ctx);
            _ctx = nullptr;
        }
    }

    EVP_MD_CTX* _ctx{nullptr};
};

/**
 * @brief 获取 MD5 字符串
 * @param data 
 * @return std::string 
 */
inline std::string md5(std::string const& data) {
    Md5Hasher hasher;
    hasher.update(data);
    return hasher.finalizeHex();
}

} // namespace HX::utils