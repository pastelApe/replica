//
// Created by xavier on 6/5/2023.
//

#include "arguments.h"

namespace Replica {
    void Arguments::welcome()
    {
        fmt::print("{:^{}}\n", "_____            _ _                 \n"
                               "|  __ \\          | (_)              \n"
                               "| |__) |___ _ __ | |_  ___ __ _      \n"
                               "|  _  // _ \\ '_ \\| | |/ __/ _` |   \n"
                               "| | \\ \\  __/ |_) | | | (_| (_| |   \n"
                               "|_|  \\_\\___| .__/|_|_|\\___\\__,_| \n"
                               "| |                                  \n"
                               "|_|                              \n\n\n",
                   100);
    }

    void Arguments::ValidateUserInput()
    {
        //Try until a valid path is given.
        try {
            while (true) {
                if (sourcePath.string().front() == '~') {
                    fmt::print("Formatting ~");
                    _prependHomeDir(sourcePath);
                }

                if (!exists(sourcePath)) {
                    fmt::print("{} does not exist. Try again: ", sourcePath.string());
                    std::cin >> sourcePath;
                } else {
                    break;
                }
            }

            auto currentDirectory { std::filesystem::current_path() };

            sourcePath = std::filesystem::canonical(sourcePath);

            if (currentDirectory != sourcePath.parent_path()) {
                //Change CWD.
                std::filesystem::current_path(sourcePath);
            }

            fmt::print("Set working directory to given path: {}\n", sourcePath.string());
        }
        catch (std::filesystem::filesystem_error const& error) {
            throw std::runtime_error(fmt::format(fmt::runtime(
                                                         "what(): {}\n "
                                                         "path(): {}\n "
                                                         "code().value(): {}\n"
                                                         "code().message(): {}\n "
                                                         "code().category(): {}\n"),
                                                 error.what(),
                                                 error.path1().string(),
                                                 error.code().message(),
                                                 error.code().category().name()));
        }
    }

    void Arguments::_prependHomeDir(std::filesystem::path &path)
    {
        std::string homeDir = getenv("HOME");
        std::string stringPath {path.string()};
        stringPath.replace(stringPath.begin(), stringPath.begin() + 1, homeDir);
        sourcePath = stringPath;
        fmt::print("Path: {}", sourcePath.string());
    }


}
