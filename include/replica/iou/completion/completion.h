//
// Created by xavier on 6/16/2023.
//

#pragma once

#include "liburing.h"
#include "../ring/ring.h"

namespace Replica::IOU {
    class Completion {
        friend class Ring;
    public:
        explicit Completion(io_uring_cqe* cqe);
        ~Completion() = default;

        Replica::IOU::Data* Data();

        Completion Process();

        [[nodiscard]] bool Completed() const;
    private:
        io_uring_cqe* _cqe{};
        struct Data* _data {};
        bool _complete{};
    };
}