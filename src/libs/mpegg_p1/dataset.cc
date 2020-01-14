#include "dataset.h"

namespace genie {
namespace mpegg_p1 {

std::unique_ptr<mpegg_p1::Dataset> Dataset::createFromDataUnits(
    const std::vector<std::unique_ptr<mpegg_p2::DataUnit>>& dataUnits) {
    return std::unique_ptr<Dataset>(new Dataset(dataUnits));  // TODO: direct call of constructor instead of factory?
}

// TODO: implement constructor
Dataset::Dataset(const std::vector<std::unique_ptr<mpegg_p2::DataUnit>>& dataUnits) {
    (void)dataUnits;  // silence compiler warning
    // TODO: add dt_metadata and dt_protection, dataset_header, dataset_parametersets, accessunits(p1)
}
}  // namespace mpegg_p1
}  // namespace genie