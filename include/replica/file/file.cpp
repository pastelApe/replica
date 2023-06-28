//
// Created by xavier on 6/12/2023.
//

#include "file.h"


#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <ranges>

#include "fmt/core.h"


/**********************************************************************************************************************/

namespace Replica {

/**********************************************************************************************************************/
    //Public Interface

    File::File(const std::filesystem::path &sourcePath, std::filesystem::path &outputPath)
    : source(sourcePath),
      output(outputPath),
      inFd(open(sourcePath.c_str(), O_RDONLY))
      {

        if (inFd == -1) {
            throw std::runtime_error(fmt::format("Failed to open sourcePath file {}. Error: {}",
                                                 source.string(),
                                                 strerror(errno)));
        }

          statFileSize();

        if (!output.has_root_directory()) {
                output = std::filesystem::absolute(source.parent_path()).string()
                        + '/'
                        + source.stem().string()
                        + "(Copy)"
                        + source.extension().string();

        }

        outFd = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);

        if (outFd == -1) {
            throw std::runtime_error(fmt::format("Failed to open outputPath file {}. Error: {}",
                                                     output.string(),
                                                     strerror(errno)));
        }
        mmapFile();
      }


   File::~File() {
        close(inFd);
        close(outFd);
        munmap(address, size);
    }

    void File::SetOffset(uintmax_t bytesRead) { offset += (off64_t)bytesRead; }

    void File::SortFilesInto(std::vector<std::unique_ptr<Replica::File>>& uniqueFiles,
                             std::vector<std::unique_ptr<Replica::File>>& duplicateFiles) {

        if (!uniqueFiles.empty()) {
            auto match { std::ranges::find_if(uniqueFiles, [&] (const auto& file) {
                return file->size == this->size;
            })};

            if (match != uniqueFiles.end()) {
                duplicateFiles.emplace_back(this);
            } else {
                uniqueFiles.emplace_back(this);
            }
        } else {
            uniqueFiles.emplace_back(this);
        }
    }



/**********************************************************************************************************************/
    //Private member functions

    void File::statFileSize()
    {
        struct stat st{};

        if (fstat(inFd, &st) == -1) {
            throw std::runtime_error(fmt::format("Could not stat size of files {}. Error: {}\n",
                                                 source.string(),
                                                 strerror(errno)));
        }

        if (S_ISREG(st.st_mode)) {
            size = st.st_size;
        } else if (S_ISBLK(st.st_mode)) {
            auto bytes { uint64_t(0) };

            if (ioctl(inFd, BLKGETSIZE64, &bytes) == -1) {
                throw std::runtime_error(fmt::format("Could not stat size of block device {}. Error: {}\n",
                                                     source.string(),
                                                     strerror(errno)));
            }
            size = bytes;
        }
    }

    void File::mmapFile() {
        if (ftruncate64(outFd, (off64_t ) size) == -1) {
            throw std::runtime_error(fmt::format("Failed to allocate memory for the output file. Error: {}",
                                                 output.string(),
                                                 strerror(errno)));
        }

        address = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, outFd, offset);
    }

    int File::FD() const { return inFd; }

    off64_t File::Offset() const { return offset; }

    void* File::Buffer() { return address; }

    bool File::Complete() const { return offset == size; }


/**********************************************************************************************************************/

} //namespace Replica

/**********************************************************************************************************************/