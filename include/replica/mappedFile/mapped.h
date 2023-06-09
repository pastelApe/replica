//
// Created by xavier on 6/8/2023.
//

#pragma once

#include <filesystem>

namespace Replica {
    class MappedFile {
    public:
        MappedFile(std::filesystem::path& source, std::filesystem::path &output);
        ~MappedFile();

        std::filesystem::path& Source();
        [[nodiscard]] int InFd() const;
        std::shared_ptr<void*> Buffer();

    private:
        std::filesystem::path& _source;
        std::filesystem::path& _output;
        int _infd;
        int _outfd;
        size_t _size {};
        std::shared_ptr<void*> _buffer {};

        void _statFileSize();

    };

}
