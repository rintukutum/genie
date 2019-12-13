#include "parameter_set_crps.h"
#include "util/bitwriter.h"
#include "util/exceptions.h"
#include "util/make_unique.h"

// -----------------------------------------------------------------------------------------------------------------

namespace format {
namespace mpegg_p2 {
ParameterSetCrps::ParameterSetCrps(CrAlgId _cr_alg_ID) : cr_alg_ID(_cr_alg_ID) {
    if (cr_alg_ID == CrAlgId::PUSH_IN || cr_alg_ID == CrAlgId::LOCAL_ASSEMBLY) {
        crps_info = util::make_unique<CrpsInfo>();
    }
}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSetCrps::setCrpsInfo(std::unique_ptr<CrpsInfo> _crps_info) {
    if (!crps_info) {
        UTILS_THROW_RUNTIME_EXCEPTION("Invalid crps mode for crps info");
    }
    crps_info = std::move(_crps_info);
}
// -----------------------------------------------------------------------------------------------------------------

void ParameterSetCrps::write(util::BitWriter *writer) const {
    writer->write(uint8_t(cr_alg_ID), 8);
    if (crps_info) {
        crps_info->write(writer);
    }
}
}  // namespace mpegg_p2
}  // namespace format