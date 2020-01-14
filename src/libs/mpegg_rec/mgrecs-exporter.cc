/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#include "mgrecs-exporter.h"
#include <util/ordered-section.h>

namespace genie {
namespace mpegg_rec {
// ---------------------------------------------------------------------------------------------------------------------

MgrecsExporter::MgrecsExporter(std::ostream &_file_1) : writer(&_file_1) {}

// ---------------------------------------------------------------------------------------------------------------------

void MgrecsExporter::flowIn(genie::mpegg_rec::MpeggChunk &&t, size_t id) {
    genie::mpegg_rec::MpeggChunk data = std::move(t);
    util::OrderedSection section(&lock, id);
    for (auto &i : data) {
        i.write(writer);
    }
}

// ---------------------------------------------------------------------------------------------------------------------

void MgrecsExporter::dryIn() {}
}
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------