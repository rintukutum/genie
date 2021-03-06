#ifndef GENIE_DECODER_CONFIGURATION_H
#define GENIE_DECODER_CONFIGURATION_H

// -----------------------------------------------------------------------------------------------------------------

#include "cabac/descriptor_subsequence_cfg.h"
#include "util/bitwriter.h"


#include <memory>

// -----------------------------------------------------------------------------------------------------------------

namespace format {

namespace desc_conf_pres {

/**
 * ISO 23092-2 Section 3.3.2.1 table 8 lines 4 to 8
 */
class DecoderConfiguration {
   public:
    enum class EncodingModeId : uint8_t {
        CABAC = 0  //!< See Text in section
    };

    virtual void write(util::BitWriter *writer) const;

    virtual std::unique_ptr<DecoderConfiguration> clone() const = 0;

    explicit DecoderConfiguration(EncodingModeId _encoding_mode_id);

    virtual ~DecoderConfiguration() = default;

   protected:
    EncodingModeId encoding_mode_ID;  //!< : 8; Line 4
};
}  // namespace desc_conf_pres
}  // namespace format

// -----------------------------------------------------------------------------------------------------------------

#endif  // GENIE_DECODER_CONFIGURATION_H
