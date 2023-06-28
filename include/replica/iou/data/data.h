//
// Created by xavier on 6/16/2023.
//

#pragma once

#include <cstdio>
#include <memory>

namespace Replica::IOU {
    struct Data {
        bool    reserved;
        int     fd;
        off64_t offset;
        size_t  blockSize;
        size_t  length;
        void*   buffer;

        Data& Reserved();
        Data& Released();
        std::unique_ptr<Data> Build();
        [[nodiscard]] bool IsReserved() const; // Function to check if reserved is true
    };

    class DataBuilder {
    public:
        DataBuilder& SetReserved(bool reserved);
        DataBuilder& SetFileDescriptor(int fd);
        DataBuilder& SetOffset(off64_t offset);
        DataBuilder& SetBlockSize(size_t blockSize);
        DataBuilder& SetLength(size_t length);
        DataBuilder& SetBuffer(void* buffer);
        std::unique_ptr<Data> Build();

    private:
        Data data;
    };
}