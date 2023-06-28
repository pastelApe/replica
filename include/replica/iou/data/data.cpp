//
// Created by xavier on 6/16/2023.
//

#include "data.h"

namespace Replica::IOU {
    Data& Data::Reserved() {
        reserved = true;
        return *this;
    }

    Data& Data::Released() {
        reserved = false;
        return *this;
    }

    std::unique_ptr<Data> Data::Build() {
        return std::make_unique<Data>(*this);
    }

    bool Data::IsReserved() const {
        return reserved;
    }

    DataBuilder& DataBuilder::SetReserved(bool reserved) {
        data.reserved = reserved;
        return *this;
    }

    DataBuilder& DataBuilder::SetFileDescriptor(int fd) {
        data.fd = fd;
        return *this;
    }

    DataBuilder& DataBuilder::SetOffset(off64_t offset) {
        data.offset = offset;
        return *this;
    }

    DataBuilder& DataBuilder::SetBlockSize(size_t blockSize) {
        data.blockSize = blockSize;
        return *this;
    }

    DataBuilder& DataBuilder::SetLength(size_t length) {
        data.length = length;
        return *this;
    }

    DataBuilder& DataBuilder::SetBuffer(void* buffer) {
        data.buffer = buffer;
        return *this;
    }

    std::unique_ptr<Data> DataBuilder::Build() {
        return std::make_unique<Data>(data);
    }
}