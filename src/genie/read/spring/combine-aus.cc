/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#include <genie/util/bitwriter.h>
#include <genie/util/perf-stats.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include "spring-gabac.h"
#include "util.h"

namespace genie {
namespace read {
namespace spring {

/*void combine_aus(const std::string &temp_dir, compression_params &cp, const
std::vector<std::vector<gabac::EncodingConfiguration>>& configs, const std::string &outputFilePath, util::FastqStats
*stats) { using namespace format; std::ofstream ofstr(outputFilePath); util::BitWriter bw(&ofstr);

  // write parameter set
  const uint8_t PARAMETER_SET_ID = 0;
  const uint32_t READ_LENGTH = 0;
  const bool PAIRED_END = cp.paired_end;
  const bool QV_PRESENT = cp.preserve_quality;
  ParameterSet ps = createQuickParameterSet(PARAMETER_SET_ID, READ_LENGTH, PAIRED_END, QV_PRESENT,
                                            .core::mgrec::ClassType::CLASS_U, configs, true);
  auto crps = util::make_unique<ParameterSetCrps>(ParameterSetCrps::CrAlgId::GLOBAL_ASSEMBLY);
  ps.setCrps(std::move(crps));
  // FIXME add in size written to stats->cmprs_total_sz
  ps.write(&bw);

  // read info about number of blocks (AUs) and the number of reads and records in those
  const std::string block_info_file = temp_dir + "/block_info.bin";
  std::ifstream f_block_info(block_info_file, std::ios::binary);
  uint32_t num_AUs;
  f_block_info.read((char*)&num_AUs, sizeof(uint32_t));
  std::vector<uint32_t> num_reads_per_AU(num_AUs);
  std::vector<uint32_t> num_records_per_AU(num_AUs);
  f_block_info.read((char*)&num_reads_per_AU[0], num_AUs*sizeof(uint32_t));
  if (!cp.paired_end) // num reads same as num records per AU
    num_records_per_AU = num_reads_per_AU;
  else
    f_block_info.read((char*)&num_records_per_AU[0], num_AUs*sizeof(uint32_t));

  // define descriptors corresponding to reads, ids and quality (so as to read them from file)
  const std::string read_desc_prefix = temp_dir + "/read_streams.";
  const std::string id_desc_prefix = temp_dir + "/id_streams.";
  const std::string quality_desc_prefix = temp_dir + "/quality_streams.";

  // now go over each block
  for (uint32_t auId = 0; auId < num_AUs; auId++) {
    auto generated_streams = create_default_streams();
    // load read streams
    std::string filename = read_desc_prefix + std::to_string(auId);
    read_streams_from_file(generated_streams, filename, read_descriptors);
    // load id streams
    if (cp.preserve_id) {
      filename = id_desc_prefix + std::to_string(auId);
      read_streams_from_file(generated_streams, filename, id_descriptors);
    }
    // load quality streams
    if (cp.preserve_quality) {
      filename = quality_desc_prefix + std::to_string(auId);
      read_streams_from_file(generated_streams, filename, quality_descriptors);
    }
    // create and write AU
    AccessUnit au = createQuickAccessUnit(
        auId, PARAMETER_SET_ID, num_reads_per_AU[auId], .core::mgrec::ClassType::CLASS_U,
        DataUnit::DatasetType::NON_ALIGNED, &generated_streams, num_records_per_AU[auId]);
    // FIXME add in size written to stats
    au.write(&bw);
  }
}*/
}
}  // namespace read
}  // namespace genie