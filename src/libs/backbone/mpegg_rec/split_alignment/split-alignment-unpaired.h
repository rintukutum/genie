/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef GENIE_SPLIT_ALIGNMENT_UNPAIRED_H
#define GENIE_SPLIT_ALIGNMENT_UNPAIRED_H

// ---------------------------------------------------------------------------------------------------------------------

#include <backbone/mpegg_rec/split-alignment.h>
#include <util/bitwriter.h>

// ---------------------------------------------------------------------------------------------------------------------

namespace genie {
namespace mpegg_rec {

/**
 *
 */
class SplitAlignmentUnpaired : public SplitAlignment {
   public:
    /**
     *
     */
    SplitAlignmentUnpaired();

    /**
     *
     * @param writer
     */
    void write(util::BitWriter &writer) const override;

    /**
     *
     * @return
     */
    std::unique_ptr<SplitAlignment> clone() const override;
};

// ---------------------------------------------------------------------------------------------------------------------

}  // namespace mpegg_rec
}  // namespace genie

// ---------------------------------------------------------------------------------------------------------------------

#endif  // GENIE_SPLIT_ALIGNMENT_UNPAIRED_H

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------