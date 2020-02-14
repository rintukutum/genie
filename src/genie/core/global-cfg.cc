/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#include "global-cfg.h"

// ---------------------------------------------------------------------------------------------------------------------

namespace genie {
namespace core {

// ---------------------------------------------------------------------------------------------------------------------

util::IndustrialPark& GlobalCfg::getIndustrialPark() { return fpark; }

// ---------------------------------------------------------------------------------------------------------------------

GlobalCfg& GlobalCfg::getSingleton() {
    static GlobalCfg cfg;
    return cfg;
}

// ---------------------------------------------------------------------------------------------------------------------

}  // namespace core
}  // namespace genie

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------