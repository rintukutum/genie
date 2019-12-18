/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef GENIE_MPEGG_DECOMPRESSOR_H
#define GENIE_MPEGG_DECOMPRESSOR_H

#include "mpegg-raw-au.h"

#include <format/block-payload.h>
#include <util/source.h>
#include <memory>

class MpeggDecompressor : public Drain<std::unique_ptr<BlockPayloadSet>>, public Source<std::unique_ptr<MpeggRawAu>> {
   public:
    void flowIn(std::unique_ptr<BlockPayloadSet> t, size_t id) override = 0;
    void dryIn() override = 0;
    ~MpeggDecompressor() override = default;
};

#endif  // GENIE_MPEGG_DECOMPRESSOR_H