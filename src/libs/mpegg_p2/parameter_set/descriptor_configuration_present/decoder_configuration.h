#ifndef GENIE_DECODER_CONFIGURATION_H
#define GENIE_DECODER_CONFIGURATION_H

// -----------------------------------------------------------------------------------------------------------------

#include <backbone/constants.h>
#include "mpegg_p2/parameter_set/descriptor_configuration_present/cabac/descriptor_subsequence_cfg.h"
#include "util/bitreader.h"
#include "util/bitwriter.h"

#include <memory>

// -----------------------------------------------------------------------------------------------------------------

namespace genie {
namespace mpegg_p2 {
namespace desc_conf_pres {

/**
 * ISO 23092-2 Section 3.3.2.1 table 8 lines 4 to 8
 */
class DecoderConfiguration {
   public:
    enum class EncodingModeId : uint8_t {
        CABAC = 0  //!< See Text in section
    };

    virtual void write(util::BitWriter& writer) const;

    virtual std::unique_ptr<DecoderConfiguration> clone() const = 0;

    explicit DecoderConfiguration(EncodingModeId _encoding_mode_id);

    virtual ~DecoderConfiguration() = default;

    static std::unique_ptr<DecoderConfiguration> factory(genie::GenDesc desc, util::BitReader& reader);

    EncodingModeId getEncodingMode() const;

   protected:
    EncodingModeId encoding_mode_ID;  //!< : 8; Line 4
};
}  // namespace desc_conf_pres
}  // namespace mpegg_p2
}  // namespace genie
// -----------------------------------------------------------------------------------------------------------------

#endif  // GENIE_DECODER_CONFIGURATION_H