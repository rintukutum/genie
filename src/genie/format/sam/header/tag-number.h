/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef GENIE_TAG_NUMBER_H
#define GENIE_TAG_NUMBER_H

// ---------------------------------------------------------------------------------------------------------------------

#include <memory>
#include "tag.h"

// ---------------------------------------------------------------------------------------------------------------------

namespace genie {
namespace format {
namespace sam {
namespace header {

// ---------------------------------------------------------------------------------------------------------------------

class TagNumber : public TagBase {
   private:
    uint64_t data;

   public:
    TagNumber(std::string _name, uint64_t _data);

    std::string toString() const override;

    static std::unique_ptr<TagBase> fromString(std::string name, const std::string& string);
};

// ---------------------------------------------------------------------------------------------------------------------

}  // namespace header
}  // namespace sam
}  // namespace format
}  // namespace genie

// ---------------------------------------------------------------------------------------------------------------------

#endif  // GENIE_TAG_NUMBER_H

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------