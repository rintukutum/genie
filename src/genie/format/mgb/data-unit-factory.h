/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef GENIE_DATA_UNIT_FACTORY_H
#define GENIE_DATA_UNIT_FACTORY_H

// ---------------------------------------------------------------------------------------------------------------------

#include <genie/core/parameter/parameter_set.h>
#include <map>
#include "access_unit.h"

// ---------------------------------------------------------------------------------------------------------------------

namespace genie {
namespace format {
namespace mgb {

// ---------------------------------------------------------------------------------------------------------------------

class DataUnitFactory {
   private:
    std::map<size_t, std::unique_ptr<core::parameter::ParameterSet>> parameters;

   public:
    const core::parameter::ParameterSet& getParams(size_t id) const;
    std::unique_ptr<AccessUnit> read(util::BitReader& bitReader);
};

// ---------------------------------------------------------------------------------------------------------------------

}  // namespace mgb
}  // namespace format
}  // namespace genie

// ---------------------------------------------------------------------------------------------------------------------

#endif  // GENIE_DATA_UNIT_FACTORY_H

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------