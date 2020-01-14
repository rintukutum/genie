#ifndef GENIE_MM_CFG_H
#define GENIE_MM_CFG_H

// -----------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include "util/bitreader.h"
#include "util/bitwriter.h"

// -----------------------------------------------------------------------------------------------------------------
namespace genie {
namespace mpegg_p2 {

/**
 * ISO 23092-2 Section 3.4.1.1 table 19 lines 7 to 10
 */
class MmCfg {
   private:
    uint16_t mm_threshold;  //!< Line 8
    uint32_t mm_count;      //!< Line 9

   public:
    MmCfg(uint16_t _mm_threshold, uint32_t _mm_count);
    MmCfg();
    explicit MmCfg(util::BitReader &reader);
    virtual ~MmCfg() = default;

    virtual void write(util::BitWriter &writer) const;
};
}  // namespace mpegg_p2
}  // namespace genie
// -----------------------------------------------------------------------------------------------------------------

#endif  // GENIE_MM_CFG_H