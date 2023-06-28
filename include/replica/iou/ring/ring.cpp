//
// Created by xavier on 6/5/2023.
//

#include "ring.h"

#include <stdexcept>
#include <fmt/core.h>
#include <algorithm>

/**********************************************************************************************************************/

namespace Replica
{

/**********************************************************************************************************************/

    IOU::Exception::Exception(const std::string &message) : runtime_error(message) { }

/**********************************************************************************************************************/

    IOU::Ring::Ring(uint storage, size_t blockSize, [[maybe_unused]]uint cqeMultiplier, uint flags)
    : _ring      (io_uring()),
      _entries   (storage),
      _blockSize (blockSize * 1024 * 1024)
    {
        if (!io_uring_register_ring_fd(&_ring)) {
            throw std::runtime_error(fmt::format("Ring failed to register its FD. Error: {}\n", strerror(-errno)));
        }

        if (io_uring_queue_init(_entries, &_ring, flags)) {
            throw std::runtime_error(fmt::format("Ring failed to start. Error: {}\n", strerror(-errno)));
        }

        //Prepare initial data supply.
        _dataPool.reserve(_ring.cq.ring_entries);

        //Fill pool with data.
        std::ranges::generate_n(std::back_inserter(_dataPool), _ring.cq.ring_entries, [] { return Data().Build(); });
    }


    IOU::Ring::~Ring()
    {
        io_uring_unregister_ring_fd(&_ring);
        io_uring_queue_exit(&_ring);
    }


    std::unique_ptr<IOU::Data> IOU::Ring::DataHandler()
    {
        //Check for available IOU::Data.
        auto openData {std::ranges::find_if(_dataPool, [] (const auto& data) { return !data->IsReserved(); }) };

        auto newData { Replica::IOU::DataBuilder().Build() };

        //No available IOU:Data. Create one and add it to the supply.
        if (openData == _dataPool.end()) {
            if (!_dataPool.empty()) {
                newData.swap(_dataPool.emplace_back(DataBuilder().Build()));
            }
        } else {
            //Reserve the IOU::Data.
            newData.swap(*openData);
            newData->Reserved();
        }

        return newData;
    }


    uintmax_t IOU::Ring::ProcessCompletions() {
        io_uring_cqe *cqe{nullptr};
        auto head{0};
        auto completions{0};


        io_uring_for_each_cqe(&_ring, head, cqe) {
            completions++;
            if (cqe != nullptr) {
                auto result{cqe->res};
                auto data{Replica::IOU::Data(cqe->user_data)};

                try {
                    if (result < 0
                        && (std::errc) (-result) != std::errc::resource_unavailable_try_again
                        && (std::errc) (-result) != std::errc::operation_canceled) {
                        throw Replica::IOU::Exception(fmt::format("Failed processing CQE. Error: {}\n",
                                                                  strerror(-result)));
                    }

                    // Successful completion.
                    if (result == data.length) {
                        data.Released();
                    }

                    // Short read. Adjust and resubmit.
                    if (result != data.length && result > 0) {
                        data.buffer = (char *) data.buffer + result;
                        data.blockSize -= result;
                        data.offset += result;
                    }

                    // Explicit handling of EAGAIN or ECANCELED.
                    PrepareSubmissions(&data);

                    completions++;
                }

                catch (const Replica::IOU::Exception &exception) {
                    fmt::print("IOUException occurred: {}.\n ", exception.what());
                }
            }
        }
        io_uring_cq_advance(&_ring, completions);
        return completions;
    }


    bool IOU::Ring::PrepareSubmissions(Replica::IOU::Data* data)
    {
        io_uring_sqe* sqe { io_uring_get_sqe(&_ring) };

        if (sqe == nullptr) {
            _submit();
            return false;
        }

        io_uring_prep_read(sqe, data->fd, data->buffer, data->blockSize, data->offset);
        io_uring_sqe_set_data(sqe, data);

        return true;
    }

    uintmax_t IOU::Ring::BlockSize() const { return _blockSize; }


/**********************************************************************************************************************/
// Private member functions


    uintmax_t IOU::Ring::_submit()
    {
        auto submissions { io_uring_submit(&_ring) };

        try {
            if (submissions < 0) {
                throw Replica::IOU::Exception(fmt::format("Failed to submit. Error: {}", strerror(-errno)));
            }
        }
        catch (Replica::IOU::Exception& exception) {
            fmt::print("IOUException occurred: {}.\n ", exception.what());
        }

        return submissions;
    }

/**********************************************************************************************************************/

} // namespace Replica

/**********************************************************************************************************************/



