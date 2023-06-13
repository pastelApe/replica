#include "replica/arguments/arguments.h"
#include "replica/copy/copy.h"
#include "replica/file/file.h"
#include "replica/unique/unique.h"
#include "replica/iou/ring.h"

int main(int argc, char* argv[]) {
    auto arguments = argparse::parse<Replica::Arguments>(argc, argv);
    arguments.welcome();

    auto ring { Replica::IOU::Ring(arguments.entries, arguments.blockSize, arguments.cqeMultiplier) };

    if (std::filesystem::is_directory(arguments.sourcePath)) {
        auto uniqueSizeFiles { std::vector<std::unique_ptr<Replica::File>>() };
        auto duplicateFiles  { std::vector<std::unique_ptr<Replica::File>>() };

        std::ranges::for_each(std::filesystem::directory_iterator(arguments.sourcePath), [&] (auto& directoryEntry) {
           if (!directoryEntry.is_directory()) {
               auto entry { std::make_unique<Replica::File>(directoryEntry.path()) };
               entry->SortFilesInto(uniqueSizeFiles, duplicateFiles);
           }
        });

        auto uniqueFiles { std::vector<std::filesystem::path>() };

        std::ranges::for_each(duplicateFiles, [&] (auto& duplicate) {
        });
        Replica::Copy(file, ring);

    } else {

       auto file { Replica::File(arguments.sourcePath, arguments.outputPath) };

       Replica::Copy(file, ring);
    }
    return 0;
}