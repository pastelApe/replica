//
// Created by xavier on 6/16/2023.
//

#include <fmt/core.h>
#include "completion.h"

namespace Replica::IOU {
    Completion::Completion(io_uring_cqe *cqe) {

    }

    Replica::IOU::Data* Completion::Data() { return _data; }

    Completion Completion::Process()
    {
        auto result { this->_cqe->res };

        try {
            if (result < 0 &&
                (std::errc)(-result) != std::errc::resource_unavailable_try_again &&
                (std::errc)(-result) != std::errc::operation_canceled) {
                throw Replica::IOU::Exception(fmt::format("Failed on CQE. Error: {}\n", std::to_string(-result)));

            } else if (result != this->_data->length && result > 0) {
                //Short read.
                _adjust_for_submission(result);
            } else if (result == this->_data->length) {
                this->_data->reserved = false;
                this->_complete = true;
            } else {
                //EAGAIN or ECANCELED. _submit this completion again.
                this->_complete = false;
            }
            return *this;
        }
        catch (const Replica::IOU::Exception& exception) {
            fmt::print("IOUException occurred: {}.\n ", exception.what());
        }

    }

    bool Completion::Completed() const { return _complete; }

    void Completion::_adjust_for_submission(int bytesRead)

}

