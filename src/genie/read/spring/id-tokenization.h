/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#ifndef SPRING_ID_TOKENIZATION_H_
#define SPRING_ID_TOKENIZATION_H_

#include <genie/stream-saver.h>
#include <string>
#include <vector>

namespace genie {
namespace read {
namespace spring {

void generate_id_tokens(char *prev_ID, uint32_t *prev_tokens_ptr, std::string &current_id,
                        std::vector<util::DataBlock> &tokens);

void generate_id_tokens(char *prev_ID, uint32_t *prev_tokens_ptr, std::string &current_id,
                        std::vector<int64_t> tokens[128][6]);

std::string decode_id_tokens(std::string &prev_ID, uint32_t *prev_tokens_ptr, uint32_t *prev_tokens_len,
                             const std::vector<int64_t> tokens[128][6], uint32_t pos_in_tokens_array[128][6]);

void read_read_id_tokens_from_file(const std::string &infile_name, std::vector<int64_t> tokens[128][6],
                                   StreamSaver *ld);

void big_endian_push_uint32(uint32_t val, util::DataBlock *vec);

}  // namespace spring
}  // namespace read
}  // namespace genie

#endif  // SPRING_ID_TOKENIZATION_H_