#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-Music.
 *
 * HX-Music is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
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
#include <vector>
#include <string>

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace HX::token {

class Aes256Gcm {
public:
    inline constexpr static std::size_t KeyLen = 32;

    explicit Aes256Gcm(const std::string& key)
        : _key{reinterpret_cast<const unsigned char*>(key.data()), 
               reinterpret_cast<const unsigned char*>(key.data()) + key.size()}
        , _ctx{}
    {
        if (key.size() != KeyLen) [[unlikely]] {
            throw std::runtime_error("AES-256-GCM key size must be 32 bytes");
        }

        _ctx = EVP_CIPHER_CTX_new();
        if (!_ctx) [[unlikely]] {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }
    }

    ~Aes256Gcm() {
        if (_ctx) {
            EVP_CIPHER_CTX_free(_ctx);
            _ctx = nullptr;
        }
    }

    Aes256Gcm(const Aes256Gcm&) = delete;
    Aes256Gcm& operator=(const Aes256Gcm&) = delete;

    Aes256Gcm(Aes256Gcm&& other) noexcept
        : _ctx(other._ctx)
        , _key(std::move(other._key))
    {
        other._ctx = nullptr;
    }

    Aes256Gcm& operator=(Aes256Gcm&& other) noexcept {
        if (this != &other) {
            if (_ctx) {
                EVP_CIPHER_CTX_free(_ctx);
            }
            _ctx = other._ctx;
            _key = std::move(other._key);
            other._ctx = nullptr;
        }
        return *this;
    }

    // 生成随机IV
    static std::vector<unsigned char> generateRandomIV(std::size_t length = 12) {
        std::vector<unsigned char> iv(length);
        if (!RAND_bytes(iv.data(), static_cast<int>(iv.size()))) {
            throw std::runtime_error("IV generation failed");
        }
        return iv;
    }

    // AES-GCM加密
    std::string encrypt(const std::string& plaintext) {
        constexpr std::size_t ivLen = 12;
        auto iv = generateRandomIV(ivLen);

        std::vector<unsigned char> ciphertext(plaintext.size() + 16);
        std::vector<unsigned char> tag(16);

        if (EVP_EncryptInit_ex(_ctx, EVP_aes_256_gcm(), nullptr, _key.data(), iv.data()) != 1) [[unlikely]] {
            throw std::runtime_error("EncryptInit failed");
        }

        int outLen = 0;
        if (EVP_EncryptUpdate(
                _ctx,
                ciphertext.data(),
                &outLen,
                reinterpret_cast<const unsigned char*>(plaintext.data()),
                static_cast<int>(plaintext.size())) != 1) [[unlikely]] {
            throw std::runtime_error("EncryptUpdate failed");
        }
        int totalLen = outLen;

        if (EVP_EncryptFinal_ex(_ctx, ciphertext.data() + outLen, &outLen) != 1) [[unlikely]] {
            throw std::runtime_error("EncryptFinal failed");
        }
        totalLen += outLen;

        if (EVP_CIPHER_CTX_ctrl(_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) [[unlikely]] {
            throw std::runtime_error("Get tag failed");
        }

        // 拼接: IV + Ciphertext + Tag
        std::string res;
        res.reserve(ivLen + totalLen + tag.size());
        res.append(reinterpret_cast<const char*>(iv.data()), ivLen);
        res.append(reinterpret_cast<const char*>(ciphertext.data()), totalLen);
        res.append(reinterpret_cast<const char*>(tag.data()), tag.size());

        return res;
    }

    // AES-GCM解密
    std::string decrypt(const std::string& encrypted) {
        constexpr std::size_t ivLen = 12;
        constexpr std::size_t tagLen = 16;

        if (encrypted.size() < ivLen + tagLen) [[unlikely]] {
            throw std::runtime_error("Invalid encrypted data");
        }

        const unsigned char* iv = reinterpret_cast<const unsigned char*>(encrypted.data());
        const unsigned char* cipher = iv + ivLen;
        size_t cipherLen = encrypted.size() - ivLen - tagLen;
        const unsigned char* tag = iv + encrypted.size() - tagLen;

        std::vector<unsigned char> plaintext(cipherLen);

        if (EVP_DecryptInit_ex(_ctx, EVP_aes_256_gcm(), nullptr, _key.data(), iv) != 1) [[unlikely]] {
            throw std::runtime_error("DecryptInit failed");
        }

        int outLen = 0;
        if (EVP_DecryptUpdate(_ctx, plaintext.data(), &outLen, cipher, static_cast<int>(cipherLen)) != 1) [[unlikely]] {
            throw std::runtime_error("DecryptUpdate failed");
        }
        int totalLen = outLen;

        if (EVP_CIPHER_CTX_ctrl(_ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), const_cast<unsigned char*>(tag)) != 1) [[unlikely]] {
            throw std::runtime_error("Set tag failed");
        }

        if (EVP_DecryptFinal_ex(_ctx, plaintext.data() + outLen, &outLen) != 1) [[unlikely]] {
            throw std::runtime_error("DecryptFinal failed: authentication failed");
        }
        totalLen += outLen;

        return std::string(reinterpret_cast<char*>(plaintext.data()), totalLen);
    }

private:
    EVP_CIPHER_CTX* _ctx = nullptr;
    std::vector<unsigned char> _key;
};

} // namespace HX::token
