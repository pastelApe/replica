//
// Created by xavier on 6/12/2023.
//

#pragma once


#include <filesystem>
#include <optional>
#include <vector>
#include <sys/stat.h>
#include <set>
#include <map>
#include "xxhash.h"

namespace Replica {
    class File {
    public:

        File() = default;
        explicit File(const std::filesystem::path &sourcePath,
                      std::optional<std::filesystem::path> &outputPath =
                              (std::optional<std::filesystem::path> &) std::nullopt);
        ~File();

        [[nodiscard]] uintmax_t Size() const;
        [[nodiscard]] bool Progress() const;
        void Update_Offset(off64_t bytesRead);
        void SortFilesInto(std::vector<std::unique_ptr<Replica::File>>& uniqueFiles,
                           std::vector<std::unique_ptr<Replica::File>>& duplicateFiles);

    private:
        void* address   {};
        off64_t offset  {};
        uintmax_t size  {};
        const int inFd  {};
        const int outFd {};

        std::unique_ptr<struct stat> metadata {};
        XXH128_hash_t completeHash   {};
        XXH128_hash_t firstHashBlock {};
        XXH128_hash_t lastHashBlock  {};

        const std::filesystem::path source;
        std::optional<std::filesystem::path> output;

        void statFile();
        void* mmapFile();
    };
}