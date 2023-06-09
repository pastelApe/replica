//
// Created by xavier on 6/5/2023.
//

#include "ring.h"

#include <stdexcept>
#include <fmt/core.h>
#include <algorithm>
namespace Replica
{
    IOU::Ring::Ring(uint storage, size_t blockSize, [[maybe_unused]]uint cqeMultiplier, uint flags)
    : _ring      (std::make_unique<io_uring>()),
      _entries   (storage),
      _blockSize (blockSize * 1024 * 1024)
    {
        if (!io_uring_register_ring_fd(_ring.get())) {
            throw std::runtime_error(fmt::format("Ring failed to register its FD. Error: {}\n", strerror(-errno)));
        }

        if (io_uring_queue_init(_entries, _ring.get(), flags)) {
            throw std::runtime_error(fmt::format("Ring failed to start. Error: {}\n", strerror(-errno)));
        }

        //Prepare initial data supply.
        _dataPool.reserve(_ring->cq.ring_entries);

        //Fill pool with data.
        std::ranges::generate_n(std::back_inserter(_dataPool), _ring->cq.ring_entries, [] { return Data().Build(); });
    }

    IOU::Ring::~Ring()
    {
        io_uring_unregister_ring_fd(_ring.get());
        io_uring_queue_exit(_ring.get());
    }

    std::unique_ptr<IOU::Data> IOU::Ring::ReserveData()
    {
        //Find an available request.
        auto openData {std::ranges::find_if(_dataPool, [] (const auto& data) { return !data->reserved; }) };

        auto newData { Data().Build() };

        //No reserved crates. Create one and add it to the supply.
        if (openData == _dataPool.end()) {
            _dataPool.emplace_back(std::make_unique<Data>());

            if (!_dataPool.empty()) {
                newData.swap(_dataPool.back());
            }
        } else {
            //Take the reserved newData and checkout.
            newData->Reserved();
            newData.swap(*openData);
        }

        return newData;
    }

    bool IOU::Ring::PrepareSubmission(IOU::Data* data)
    {
        std::unique_ptr<io_uring_sqe> sqe {io_uring_get_sqe(_ring.get()) };

        if (sqe == nullptr) {
            return false;
        } else {
            io_uring_prep_read(sqe.get(), data->fd, data->buffer, data->blockSize, data->offset);
            io_uring_sqe_set_data(sqe.get(), data);
        }

        return true;
    }

    std::optional<IOU::Completion> IOU::Ring::PeekCompletion()
    {
        std::unique_ptr<io_uring_cqe*> cqe {};

        int peek {io_uring_peek_cqe(_ring.get(), cqe.get()) };

        if (!peek) {
            peek = io_uring_wait_cqe(_ring.get(), cqe.get());

            if (!peek) {
                //return reserved Completion
                fmt::print("Completion Error: {}\n", strerror(-peek));
            }
        } else {
            Completion crate(*cqe);
            crate._data = (Data*)(io_uring_cqe_get_data(crate._cqe));
            return crate;
        }
        return std::nullopt;
    }

    void IOU::Ring::Submit()
    {
        if (io_uring_submit(_ring.get()) < 0) {
            throw std::runtime_error(fmt::format("Failed to submit. Error: {}", strerror(-errno)));
        }
    }

    void IOU::Ring::CompletionSeen(Completion& completion)
    {
        io_uring_cqe_seen(_ring.get(), completion._cqe);
    }

    size_t IOU::Ring::BlockSize() const
    {
        return _blockSize;
    }

    IOU::Completion::Completion(io_uring_cqe* cqe)
    : _cqe(cqe), _data((struct Data*)io_uring_cqe_get_data(cqe))
    {

    }

    IOU::Data *IOU::Completion::Data()
    {
        return _data;
    }

    bool IOU::Completion::Completed() const
    {
        return _complete;
    }

    IOU::Completion IOU::Completion::Process()
    {
        auto result {this->_cqe->res };

        if (result < 0 && result != -EAGAIN && result != -ECANCELED) {
            throw std::runtime_error(fmt::format("Failed on CQE. Error: {}\n Code: {}\n",
                                                 strerror(-result), -result));
        } else if (result != this->_data->length && result > 0) {
            //Short this-> Adjust and continue.
            this->_data->buffer = (char*)this->_data->buffer + result;
            this->_data->blockSize -= result;
            this->_data->offset += result;
            this->_complete = false;
        } else if (result == this->_data->length) {
            this->_data->reserved = false;
            this->_complete = true;
        } else {
            //EAGAIN or ECANCELED. Submit this completion again.
            this->_complete = false;
        }
        return *this;
    }

    IOU::Data &IOU::Data::Reserved()
    {
        reserved = true;
        return *this;
    }

    IOU::Data &IOU::Data::Returned()
    {
        reserved = false;
        return *this;
    }

    size_t IOU::Data::Length() const
    {
        return length;
    }
}





