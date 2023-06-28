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
                      std::filesystem::path &outputPath);
        ~File();

        [[nodiscard]] bool Complete() const;
        [[nodiscard]] int FD() const;
        [[nodiscard]] off64_t Offset() const;
        void SetOffset(uintmax_t bytesRead);
        void* Buffer();
        void SortFilesInto(std::vector<std::unique_ptr<Replica::File>>& uniqueFiles,
                           std::vector<std::unique_ptr<Replica::File>>& duplicateFiles);

    private:
        void*     address {};
        off64_t   offset  {};
        uintmax_t size    {};
        const int inFd    {};
        int       outFd   {};

        XXH128_hash_t completeHash   {};
        XXH128_hash_t firstHashBlock {};
        XXH128_hash_t lastHashBlock  {};

        const std::filesystem::path source;
        std::filesystem::path       output;

        void statFileSize();
        void mmapFile();
    };
}