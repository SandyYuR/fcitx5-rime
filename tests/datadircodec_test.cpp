/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 * SPDX-FileCopyrightText: Copyright 2026 Fcitx5 for Android Contributors
 */
#include "datadircodec.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main() {
    using namespace fcitx::rime::datadir;
    auto check = [](bool value) {
        if (!value) {
            std::cerr << "Data directory codec regression\n";
            std::exit(1);
        }
    };
    for (const auto &name : {"rime", "foo|bar", "中文|方案", "quote\"name"}) {
        for (const auto &path : {"/storage/emulated/0/Rime",
                                 "/storage/1234-5678/Rime|同步/ trailing "}) {
            auto decoded = decodePair(encodePair(name, path));
            check(decoded && decoded->first == name && decoded->second == path);
        }
    }
    check(!decodePair("not-hex|2f"));
    check(!decodePair("61|2"));
    check(!decodePair("61|2f|62"));
    check(!decodePair("|2f"));
    check(!decodePair("61|"));
    const std::vector<std::string> profiles{"foo", "foo|bar", "rime"};
    auto migrated = decodeLegacyPair("foo|bar|/storage/Rime|sync", profiles);
    check(migrated && migrated->first == "foo|bar" &&
          migrated->second == "/storage/Rime|sync");
    auto again = decodePair(encodePair(migrated->first, migrated->second));
    check(again == migrated);
    auto normal = decodeLegacyPair("rime|/storage/Rime", profiles);
    check(normal && normal->first == "rime" &&
          normal->second == "/storage/Rime");
    check(!decodeLegacyPair("broken", profiles));
    std::cout << "Data directory codec regressions passed\n";
}
