/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 * SPDX-FileCopyrightText: Copyright 2026 Fcitx5 for Android Contributors
 */
#ifndef FCITX_RIME_DATADIRCODEC_H
#define FCITX_RIME_DATADIRCODEC_H

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fcitx::rime::datadir {

inline std::string encode(std::string_view value) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(value.size() * 2);
    for (unsigned char ch : value) {
        result += digits[ch >> 4];
        result += digits[ch & 15];
    }
    return result;
}

inline std::optional<std::string> decode(std::string_view value) {
    if (value.size() % 2) {
        return std::nullopt;
    }
    auto digit = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return -1;
    };
    std::string result;
    result.reserve(value.size() / 2);
    for (size_t i = 0; i < value.size(); i += 2) {
        const int high = digit(value[i]);
        const int low = digit(value[i + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        result += static_cast<char>((high << 4) | low);
    }
    return result;
}

inline std::string encodePair(std::string_view name, std::string_view path) {
    return encode(name) + "|" + encode(path);
}

inline std::optional<std::pair<std::string, std::string>>
decodePair(std::string_view line) {
    const auto sep = line.find('|');
    if (sep == std::string_view::npos) {
        return std::nullopt;
    }
    auto name = decode(line.substr(0, sep));
    auto path = decode(line.substr(sep + 1));
    if (!name || !path || name->empty() || path->empty()) {
        return std::nullopt;
    }
    return std::make_pair(std::move(*name), std::move(*path));
}

// The original format did not escape '|'. Recover names containing it from
// Profiles instead of splitting their registration at the first separator.
inline std::optional<std::pair<std::string, std::string>>
decodeLegacyPair(const std::string &line,
                 const std::vector<std::string> &profiles) {
    const std::string *match = nullptr;
    for (const auto &name : profiles) {
        if ((!match || name.size() > match->size()) &&
            line.compare(0, name.size() + 1, name + "|") == 0) {
            match = &name;
        }
    }
    if (match) {
        return std::make_pair(*match, line.substr(match->size() + 1));
    }
    const auto sep = line.find('|');
    if (sep == std::string::npos || !sep || sep + 1 >= line.size()) {
        return std::nullopt;
    }
    return std::make_pair(line.substr(0, sep), line.substr(sep + 1));
}

} // namespace fcitx::rime::datadir
#endif
