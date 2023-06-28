//
// Created by xavier on 6/7/2023.
//

#pragma once

#include "replica/file/file.h"
#include "replica/iou/ring/ring.h"

namespace Replica {
    void Copy(File& file, IOU::Ring& ring)
    {
        auto completionCount { uintmax_t(0)};
        //Ensure entire file is read and no completions remain.
        while (!file.Complete() || completionCount > 0) {
            //Process Completions.
            completionCount = ring.ProcessCompletions();
            //Prepare as many submissions as possible
            while(true) {

                auto data {ring.DataHandler() };

                data->fd        = file.FD();
                data->offset    = file.Offset();
                data->blockSize = ring.BlockSize();
                data->length    = ring.BlockSize();
                data->buffer    = (char*)file.Buffer() + file.Offset();

                if(!ring.PrepareSubmissions(data.get()), &file) {
                    break;
                }
                file.SetOffset(ring.BlockSize());
            }


        }
    }
}
