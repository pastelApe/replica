//
// Created by xavier on 6/5/2023.
//

#pragma once

#include <liburing.h>
#include <memory>
#include <vector>
#include <optional>

#include "replica/iou/data/data.h"
#include "replica/iou/completion/completion.h"
#include "replica/file/file.h"

namespace Replica::IOU {
    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string& message);
};

    class Ring {
    public:
        explicit Ring(uint storage, size_t blockSize, uint cqeMultiplier, uint flags = 0);
        //Disable all copying or moving
        Ring(const Ring &)            = delete;
        Ring(Ring &&)                 = delete;
        Ring &operator=(const Ring &) = delete;
        Ring &operator=(Ring &&)      = delete;

        ~Ring();

        std::unique_ptr<Data> DataHandler();

        uintmax_t ProcessCompletions();

        bool PrepareSubmissions(Replica::IOU::Data* data);

        [[nodiscard]] uintmax_t BlockSize() const;

    private:
        io_uring _ring;
        uint    _entries;
        size_t  _blockSize;
        std::vector<std::unique_ptr<Data>> _dataPool;

        uintmax_t _submit();
    };
}
