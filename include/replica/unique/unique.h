//
// Created by xavier on 6/7/2023.
//

#pragma once

#include <xxh3.h>
#include <map>
#include <set>

namespace Replica {
    class Unique {
    public:
        explicit Unique(std::filesystem::path& directory);

        std::set<std::filesystem::path> Get();
    private:
        std::multimap<size_t, std::filesystem::path> _allFiles;
        std::map<xxh_u64, std::filesystem::path> _hashedFiles;
        std::set<std::filesystem::path> _uniqueFiles;

        static XXH64_hash_t hashFile(std::filesystem::path& file);
    };

}
