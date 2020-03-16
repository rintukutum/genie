/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#include "context-tables.h"
#include<iostream>

namespace genie {
namespace entropy {
namespace gabac {
namespace contexttables {

std::vector<ContextModel> buildContextTable() {
    std::vector<ContextModel> contextModels;

    for (const auto& contextSet : INIT_TRUNCATED_UNARY_CTX) {
        for (const auto j : contextSet) {
            ContextModel contextModel(j);
            contextModels.push_back(contextModel);
        }
    }

    for (const auto& contextSet : INIT_EXPONENTIAL_GOLOMB_CTX) {
        for (const auto j : contextSet) {
            ContextModel contextModel(j);
            contextModels.push_back(contextModel);
        }
    }

    for (const auto& contextSet : INIT_BINARY_CTX) {
        for (const auto j : contextSet) {
            ContextModel contextModel(j);
            contextModels.push_back(contextModel);
        }
    }

    std::cout << "Size of contextModels: " << contextModels.size() << std::endl;
    return contextModels;
}


std::vector<ContextModel> buildContextTable(unsigned int numContexts) {
    std::vector<ContextModel> contextModels;
    
    // TODO 
    
    return contextModels;
}

}  // namespace contexttables
}  // namespace gabac
}  // namespace entropy
}  // namespace genie