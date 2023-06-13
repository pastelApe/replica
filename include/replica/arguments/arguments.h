//
// Created by xavier on 6/5/2023.
//

#pragma once

#include <fmt/core.h>
#include "../submodule/argparse/include/argparse/argparse.hpp"

namespace Replica {
    class Arguments :  public argparse::Args {
    public:
        std::filesystem::path &sourcePath = arg("Source path");
        std::optional<std::filesystem::path> &outputPath =
                kwarg("o,outputPath", "Destination Path.").set_default(std::filesystem::current_path());
        int& entries       = kwarg("s,entries", "Number of SQE entries for the Ring instance.").set_default(16);
        int& blockSize     = kwarg("p,blockSize", "Size of read() operation in MBs.").set_default(32);
        int& cqeMultiplier = kwarg("i,cqeMultiplier", "Multiplier for CQ. Default is twice the SQEs. ").set_default(2);
        bool& verbose      = flag("v,verbose", "A flag to toggle verbose");

        /**
         * Display ascii art for the application.
         */
        void welcome() override;

        /**
         * Ensure the given path is a valid directory..
         */
        void ValidateUserInput();
    private:
        void _prependHomeDir(std::filesystem::path& path);

    };
}

