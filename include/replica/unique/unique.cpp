//
// Created by xavier on 6/7/2023.
//

#include <vector>
#include <filesystem>
#include <algorithm>
#include "unique.h"

namespace Replica {
    Unique::Unique(std::filesystem::path &directory) {
        std::ranges::for_each(std::filesystem::recursive_directory_iterator(directory), [&] (auto& entry) {
            if (!entry.is_directory()) {
                _allFiles.emplace(entry.file_size(), entry.path());
            }
        });
        //TODO: Loop not properly adding files to the proper container.
        if (!_allFiles.empty()) {
            std::ranges::for_each(_allFiles, [&] (auto& file) {
                if (_allFiles.contains(file.first)) {
                    _hashedFiles.emplace(hashFile(file.second), file.second);
                } else {
                    _uniqueFiles.emplace(file.second);
                }
            });
        }
    }

    XXH64_hash_t Unique::hashFile(std::filesystem::path &file) {
        auto buffer { std::make_unique<char[]>(file_size(file)) };

        return XXH64(buffer.get(), file_size(file), 0);
    }

    std::set<std::filesystem::path> Unique::Get() { return _uniqueFiles; }
}