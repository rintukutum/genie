#ifndef SPRING_COMBINE_AUS_H_
#define SPRING_COMBINE_AUS_H_

#include <string>
#include "util.h"
#include <util/perf-stats.h>

namespace spring {

void combine_aus(const std::string &temp_dir, compression_params &cp, const std::vector<std::vector<gabac::EncodingConfiguration>>& configs, const std::string &outputFilePath, util::PerfStats *stats);

}  // namespace spring

#endif  // SPRING_COMBINE_AUS_H_