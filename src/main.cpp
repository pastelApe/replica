#include "replica/arguments/arguments.h"
#include "replica/copy/copy.h"
#include "replica/mappedFile/mapped.h"
#include "replica/unique/unique.h"
#include "replica/iou/ring.h"

int main(int argc, char* argv[]) {
    auto arguments = argparse::parse<Replica::Arguments>(argc, argv);
    arguments.ValidateUserInput();
    arguments.welcome();

    auto ring { Replica::IOU::Ring(arguments.entries, arguments.blockSize, arguments.cqeMultiplier) };

    if (std::filesystem::is_directory(arguments.sourcePath)) {
        auto uniqueFileSet { Replica::Unique(arguments.sourcePath) };

        std::ranges::for_each(uniqueFileSet.Get(), [&] (auto uniqueFile) {
            auto destinationPath {
                    std::filesystem::path(arguments.outputPath.string() + "/" + uniqueFile.filename().string())
            };

            Replica::MappedFile mappedFile(uniqueFile, destinationPath);

            Replica::Copy((Replica::MappedFile &)(mappedFile), ring);
        });
    } else {
        auto destinationPath {
                std::filesystem::path(arguments.outputPath.string() + "/" + arguments.sourcePath.filename().string())
        };

        Replica::MappedFile mappedFile(arguments.sourcePath, destinationPath);

        Replica::Copy((Replica::MappedFile &)(mappedFile), ring);
    }
    return 0;
}