/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef GABAC_DECODE_CABAC_H_
#define GABAC_DECODE_CABAC_H_

#include <cstdint>
#include <vector>

namespace genie {
namespace util {
class DataBlock;
}
}  // namespace genie

namespace genie {
namespace entropy {
namespace gabac {

enum class BinarizationId;
enum class ContextSelectionId;

void decode_cabac(const BinarizationId& binarizationId, const std::vector<uint32_t>& binarizationParameters,
                  const ContextSelectionId& contextSelectionId, uint8_t wordsize, util::DataBlock* bitstream);

}  // namespace gabac
}  // namespace entropy
}  // namespace genie
#endif  // GABAC_DECODE_CABAC_H_