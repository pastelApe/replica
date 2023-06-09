//
// Created by xavier on 6/5/2023.
//

#pragma once

#include <liburing.h>
#include <memory>
#include <vector>
#include <optional>

namespace Replica::IOU {
        struct Data {
            bool    reserved;
            int     fd;
            off64_t offset;
            size_t  blockSize;
            size_t  length;
            void*   buffer;

            Data& Reserved();
            Data& Returned();

            std::unique_ptr<Data> Build() {
                return std::make_unique<Data>(*this);
            };

            [[nodiscard]] size_t Length() const;
        };

        class Completion {
            friend class Ring;
        public:
            explicit Completion(io_uring_cqe* cqe);
            ~Completion() = default;

            struct Data* Data();

            Completion Process();

            [[nodiscard]] bool Completed() const;
        private:
            io_uring_cqe* _cqe;
            struct Data* _data {};
            bool _complete{};
        };

        class Ring {
        public:
            explicit Ring(uint storage, size_t blockSize, uint cqeMultyplier, uint flags = 0);
            //Disable all copying or moving
            Ring(const Ring &)            = delete;
            Ring(Ring &&)                 = delete;
            Ring &operator=(const Ring &) = delete;
            Ring &operator=(Ring &&)      = delete;

            ~Ring();

            std::unique_ptr<Data> ReserveData();

            bool PrepareSubmission(IOU::Data* data);

            void Submit();

            std::optional<Completion> PeekCompletion();

            void CompletionSeen(Completion& completion);

            [[nodiscard]] size_t BlockSize() const;

        private:
            std::shared_ptr<io_uring> _ring;
            uint    _entries;
            size_t  _blockSize;
            std::vector<std::unique_ptr<Data>> _dataPool;
        };
    }
