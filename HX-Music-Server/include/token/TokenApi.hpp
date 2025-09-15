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

#include <token/Aes256Gcm.hpp>
#include <token/Base64Url.hpp>

#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

namespace HX::token {

namespace internal {

struct KeyFromType {
    static std::string getKey() {
        return "HX::token::KeyFromType@Heng_Xin@";
    }
};

} // namespace internal

struct TokenApi {
    template <typename T>
    std::string toToken(T&& t) {
        std::string json;
        reflection::toJson(t, json);
        return Base64Url::encodeUrl(
            _aes256Gcm.encrypt(json)
        );
    }

    template <typename T>
    T fromToken(std::string urlBase64) {
        T t;
        reflection::fromJson(t, _aes256Gcm.decrypt(
            Base64Url::decodeUrl(std::move(urlBase64))
        ));
        return t; 
    }

    template <typename KeyFromType = internal::KeyFromType>
        requires(requires (KeyFromType k) {
            { KeyFromType::getKey() } -> std::convertible_to<std::string>;
        })
    static TokenApi& get() {
        thread_local static TokenApi s{KeyFromType::getKey()}; 
        return s;
    }

private:
    TokenApi(std::string key)
        : _aes256Gcm{std::move(key)}
    {}

    TokenApi& operator=(TokenApi&&) noexcept = delete;

    Aes256Gcm _aes256Gcm;
};

} // namespace HX::token