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

    File::File(const std::filesystem::path &sourcePath, std::optional<std::filesystem::path> &outputPath)
    : source(sourcePath),
      output(outputPath),
      inFd(open(sourcePath.c_str(), O_RDONLY)) {

        if (inFd == -1) {
            throw std::runtime_error(fmt::format("Failed to open sourcePath file {}. Error: {}",
                                                 source.string(),
                                                 strerror(errno)));
        }

        statFile();

        if (output.has_value()) {
            if (std::filesystem::is_directory(output.value())) {
                output = output.value().string() + '/' + source.filename().string();
            }

            open(output.value().c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);

            if (outFd == -1) {
                throw std::runtime_error(fmt::format("Failed to open outputPath file {}. Error: {}",
                                                     output.value().string(),
                                                     strerror(errno)));
            }

            mmapFile();
        }
    }

   File::~File() {
        close(inFd);
        close(outFd);
        munmap(address, size);
    }

    uintmax_t File::Size() const { return size; }

    bool File::Progress() const { return offset == size; }

    void File::Update_Offset(off64_t bytesRead) { offset += bytesRead; }

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

    void File::statFile() {

        if (fstat(inFd, metadata.get()) == -1) {
            throw std::runtime_error(fmt::format("Could not stat size of files {}. Error: {}\n",
                                                 source.string(),
                                                 strerror(errno)));
        }

        if (S_ISREG(metadata->st_mode)) {
            size = metadata->st_size;

        } else if (S_ISBLK(metadata->st_mode)) {

            if (ioctl(inFd, BLKGETSIZE64, &size) == -1) {
                throw std::runtime_error(fmt::format("Could not stat size of block device {}. Error: {}\n",
                                                     source.string(),
                                                     strerror(errno)));
            }
        }
    }

    void *File::mmapFile() {
        if (std::filesystem::file_size(output.value()) < size) {
            if (ftruncate64(outFd, (off64_t ) size) == -1) {
                throw std::runtime_error(fmt::format("Failed to allocate memory for the output file. Error: {}",
                                                     source.string(),
                                                     strerror(errno)));
            }
        }

        address = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, outFd, offset);
    }


/**********************************************************************************************************************/

} //namespace Replica

/**********************************************************************************************************************/