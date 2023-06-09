//
// Created by xavier on 6/7/2023.
//

#pragma once

#include "replica/mappedFile/mapped.h"
#include "replica/iou/ring.h"

namespace Replica {
    void Copy(MappedFile& file, IOU::Ring& ring)
    {
        auto fileSize  { std::filesystem::file_size(file.Source()) };
        auto blockSize { ring.BlockSize() };
        auto offset    { off64_t (0) };
        auto remaining { fileSize };

        //Ensure entire files is read.
        while (!remaining) {
            //Process Completions.
            while(true) {
                //Find at least one completion.
                auto completion { ring.PeekCompletion() };

                if (completion.has_value()) {
                    //Check for completeness.
                    completion->Process();

                    if (completion.value().Completed()) {
                        //Release CQE and track length of the read() result.
                        ring.CompletionSeen(completion.value());
                        remaining -= completion.value().Data()->Length();
                    }

                    //Resubmit due to EAGAIN or ECANCELED error during Process().
                    if (!ring.PrepareSubmission(completion.value().Data())) {
                        break;
                    }

                    //Clear completion before CQE is recycled.
                    completion.reset();
                    completion.value().Data()->Returned();
                } else {
                    //no completion found.
                    break;
                }
            }

            ring.Submit();

            //Prepare as many shipments as possible
            while(true) {

                if (remaining < blockSize ) {
                    blockSize = remaining;
                }

                auto data { ring.ReserveData() };

                data->fd        = file.InFd();
                data->offset    = offset;
                data->blockSize = blockSize;
                data->length    = blockSize;
                data->buffer    = file.Buffer().get();

                //Fill Storage and Ship once full.
                if (!ring.PrepareSubmission(data.get())) {
                    break;
                }

                //Update tracking.
                offset += (off64_t) blockSize;
            }
        }
    }
}
