//
// Created by xavier on 6/8/2023.
//

#include <csignal>      // ftruncate
#include <fcntl.h>      // O_ flags
#include <linux/fs.h>   // BLKGETSIZE64
#include <sys/ioctl.h>
#include <sys/mman.h>   // PROT flags
#include <sys/stat.h>   // fstat, S_ flags

#include <fmt/core.h>

#include "mapped.h"

namespace Replica {
    MappedFile::MappedFile(std::filesystem::path& source, std::filesystem::path &output)
    : _source(source),
      _output(output),
      _infd(open(_source.c_str(), O_RDONLY)),
      _outfd(open(_output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644))
    {
        if (_infd == -1) {
            throw std::runtime_error(fmt::format("Failed to open source file {}. Error: {}",
                                                 _source.string(),
                                                 strerror(errno)));
        }

        if (_outfd == -1) {
            throw std::runtime_error(fmt::format("Failed to open output file {}. Error: {}",
                                                 _source.string(),
                                                 strerror(errno)));
        }
        _statFileSize() ;

        //Set the output file to the size of the source.
        if (ftruncate64(_outfd, (off64_t)_size) == -1) {
            throw std::runtime_error(fmt::format("Failed to set the size of the output file. Error: {}",
                                                 _source.string(),
                                                 strerror(errno)));
        }

        _buffer = std::make_shared<void*>(mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _outfd, 0));
    }

    MappedFile::~MappedFile()
    {
        close(_infd);
        close(_outfd);
        munmap(_buffer.get(), _size);
    }

    void MappedFile::_statFileSize()
    {
        struct stat st{};

        if (fstat(_infd, &st) == -1) {
            throw std::runtime_error(fmt::format("Could not stat size of files {}. Error: {}\n",
                                                 _source.string(),
                                                 strerror(errno)));
        }

        if (S_ISREG(st.st_mode)) {
            _size = st.st_size;
        } else if (S_ISBLK(st.st_mode)) {
            auto bytes { uint64_t(0) };

            if (ioctl(_infd, BLKGETSIZE64, &bytes) == -1) {
                throw std::runtime_error(fmt::format("Could not stat size of block device {}. Error: {}\n",
                                                     _source.string(),
                                                     strerror(errno)));
            }
            _size = bytes;
        }
    }

    std::filesystem::path& MappedFile::Source()
    {
        return _source;
    }

    int MappedFile::InFd() const
    {
        return _infd;
    }

    std::shared_ptr<void *> MappedFile::Buffer()
    {
        return _buffer;
    }
}

