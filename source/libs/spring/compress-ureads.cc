#include "spring/preprocess.h"

#ifdef GENIE_USE_OPENMP
#include <omp.h>
#endif

#include <array>
#include <cmath>  // abs
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <numeric>
#include <algorithm>

#include "generate-read-streams.h"
#include "util.h"
#include "params.h"
#include "util.h"

#include <utils/fastq-file-reader.h>

namespace spring {

  std::vector<std::map<uint8_t, std::map<uint8_t, std::string>>>
  compress_ureads(utils::FastqFileReader *fastqFileReader1,
                  utils::FastqFileReader *fastqFileReader2,
                  const std::string &temp_dir, compression_params &cp,
                  dsg::StreamSaver& st) {
        utils::FastqFileReader *fastqFileReader[2] = {fastqFileReader1, fastqFileReader2};
        std::string outfileclean[2];
        std::string outfileN[2];
        std::string outfileorderN[2];
        std::string outfileid[2];
        std::string outfilequality[2];
//  std::string outfileread[2];
        std::string outfilereadlength[2];
        std::string basedir = temp_dir;
        outfileclean[0] = basedir + "/input_clean_1.dna";
        outfileclean[1] = basedir + "/input_clean_2.dna";
        outfileN[0] = basedir + "/input_N.dna";
        outfileN[1] = basedir + "/input_N.dna.2";
        outfileorderN[0] = basedir + "/read_order_N.bin";
        outfileorderN[1] = basedir + "/read_order_N.bin.2";
        outfileid[0] = basedir + "/id_1";
        outfileid[1] = basedir + "/id_2";
        outfilequality[0] = basedir + "/quality_1";
        outfilequality[1] = basedir + "/quality_2";
//  outfileread[0] = basedir + "/read_1";
//  outfileread[1] = basedir + "/read_2";
//  outfilereadlength[0] = basedir + "/readlength_1";
//  outfilereadlength[1] = basedir + "/readlength_2";

        std::ofstream fout_clean[2];
        std::ofstream fout_N[2];
        std::ofstream fout_order_N[2];
        std::ofstream fout_id[2];
        std::ofstream fout_quality[2];

        std::vector<utils::FastqRecord> fastqRecords;
//  if (cp.long_flag) {
//    fin[0].open(infile_1);
//    if (cp.paired_end) fin[1].open(infile_2);
//  } else {
        for (int j = 0; j < 2; j++) {
            if (j == 1 && !cp.paired_end) continue;
            //fout_clean[j].open(outfileclean[j]);
            fout_clean[j].open(outfileclean[j]);
            fout_N[j].open(outfileN[j]);
            fout_order_N[j].open(outfileorderN[j], std::ios::binary);
//      if (!cp.preserve_order) {
            if (cp.preserve_id) fout_id[j].open(outfileid[j]);
            if (cp.preserve_quality) fout_quality[j].open(outfilequality[j]);
//      }
        }
//  }

        uint32_t max_readlen = 0;
        uint64_t num_reads[2] = {0, 0};
        uint64_t num_reads_clean[2] = {0, 0};
        uint32_t num_reads_per_block;
//  if (!cp.long_flag)
        num_reads_per_block = cp.num_reads_per_block;
//  else
//    num_reads_per_block = cp.num_reads_per_block_long;
        uint8_t paired_id_code = 0;
        bool paired_id_match = false;

//  char *quality_binning_table = new char[128];
//  if (cp.ill_bin_flag) generate_illumina_binning_table(quality_binning_table);
//  if (cp.bin_thr_flag) generate_binary_binning_table(quality_binning_table, cp.bin_thr_thr, cp.bin_thr_high, cp.bin_thr_low);

        // Check that we were able to open the input files and also look for
        // paired end matching ids if relevant
        if (cp.paired_end) {
            if (cp.preserve_id) {
                std::string id_1, id_2;
                fastqFileReader[0]->readRecords(1, &fastqRecords);
                id_1 = fastqRecords[0].title;
                fastqFileReader[1]->readRecords(1, &fastqRecords);
                id_2 = fastqRecords[0].title;
                paired_id_code = find_id_pattern(id_1, id_2);
                if (paired_id_code != 0) paired_id_match = true;
                fastqFileReader[0]->seekFromSet(0);
                fastqFileReader[1]->seekFromSet(0);
            }
        }

        //
        // Disable parallelism for this loop.
        // All real work is done by the master thread, and the worker threads
        // just sit at an OpenMP barrier.  Running on a single thread speeds
        // things up slightly.
        //
        // int num_threads = cp.num_thr;
        int num_threads = 1;
#undef GENIE_USE_OPENMP

        uint64_t num_reads_per_step = (uint64_t) num_threads * num_reads_per_block;
        std::string *id_array_1 = new std::string[num_reads_per_step];
        bool *read_contains_N_array = new bool[num_reads_per_step];
        uint32_t *read_lengths_array = new uint32_t[num_reads_per_step];
        bool *paired_id_match_array = new bool[num_threads];
        uint32_t num_blocks_done = 0;

        while (true) {
            bool done[2] = {true, true};
            for (int j = 0; j < 2; j++) {
                if (j == 1 && !cp.paired_end) continue;
                done[j] = false;
                uint32_t num_reads_read = fastqFileReader[j]->readRecords(num_reads_per_step, &fastqRecords);
                if (num_reads_read < num_reads_per_step) done[j] = true;
                if (num_reads_read == 0) continue;
                if (num_reads[0] + num_reads[1] + num_reads_read > MAX_NUM_READS) {
                    std::cerr << "Max number of reads allowed is " << MAX_NUM_READS << "\n";
                    throw std::runtime_error("Too many reads.");
                }
#ifdef GENIE_USE_OPENMP
#pragma omp parallel num_threads(cp.num_thr)
#endif
                {
                    bool done_loop = false;
#ifdef GENIE_USE_OPENMP
                    uint64_t tid = omp_get_thread_num();
#else
                    uint64_t tid = 0;
#endif
                    if (j == 1) paired_id_match_array[tid] = paired_id_match;
                    if (tid * num_reads_per_block >= num_reads_read) done_loop = true;
                    uint32_t num_reads_thr = std::min((uint64_t) num_reads_read,
                                                      (tid + 1) * num_reads_per_block) -
                                             tid * num_reads_per_block;
//        std::ofstream fout_readlength;
                    if (!done_loop) {
//          if (cp.long_flag)
//            fout_readlength.open(outfilereadlength[j] + "." +
//                                     std::to_string(num_blocks_done + tid),
//                                 std::ios::binary);
                        // check if reads and qualities have equal lengths
                        for (uint32_t i = tid * num_reads_per_block;
                             i < tid * num_reads_per_block + num_reads_thr; i++) {
                            size_t len = fastqRecords[i].sequence.length();
                            if (len == 0)
                                throw std::runtime_error("Read of length 0 detected.");
//            if (cp.long_flag && len > MAX_READ_LEN_LONG) {
//              std::cerr << "Max read length for long mode is "
//                        << MAX_READ_LEN_LONG << ", but found read of length "
//                        << len << "\n";
//              throw std::runtime_error("Too long read length");
//            }
//            if (!cp.long_flag && len > MAX_READ_LEN) {
                            if (len > MAX_READ_LEN) {
                                std::cerr << "Max read length without long mode is "
                                          << MAX_READ_LEN << ", but found read of length " << len
                                          << "\n";
                                throw std::runtime_error(
                                        "Too long read length");// (please try --long/-l flag).");
                            }
                            if (cp.preserve_quality && (fastqRecords[i].qualityScores.length() != len))
                                throw std::runtime_error(
                                        "Read length does not match quality length.");
                            if (cp.preserve_id && (fastqRecords[i].title.length() == 0))
                                throw std::runtime_error("Identifier of length 0 detected.");
                            read_lengths_array[i] = (uint32_t) len;

                            // mark reads with N
//            if (!cp.long_flag)
                            read_contains_N_array[i] =
                                    (fastqRecords[i].sequence.find('N') != std::string::npos);

                            // Write read length to a file (for long mode)
//            if (cp.long_flag)
//              fout_readlength.write((char *)&read_lengths_array[i],
//                                    sizeof(uint32_t));
                            if (j == 0)
                                id_array_1[i] = fastqRecords[i].title; // store for later comparison with id_2
                            if (j == 1 && paired_id_match_array[tid])
                                paired_id_match_array[tid] = check_id_pattern(
                                        id_array_1[i], fastqRecords[i].title, paired_id_code);
                        }
//          if (cp.long_flag) fout_readlength.close();
                        // apply binning (if asked to do so)
//          if (cp.preserve_quality && (cp.ill_bin_flag || cp.bin_thr_flag))
//            quantize_quality(quality_array + tid * num_reads_per_block,
//                             num_reads_thr, quality_binning_table);

                        // apply qvz quantization (if flag specified and order preserving)
//	  if(cp.preserve_quality && cp.qvz_flag && cp.preserve_order)
//            quantize_quality_qvz(quality_array + tid * num_reads_per_block, num_reads_thr, read_lengths_array + tid * num_reads_per_block, cp.qvz_ratio);
//          if (!cp.long_flag) {
//            if (cp.preserve_order) {
//              // Compress ids
//              if (cp.preserve_id) {
//                std::string outfile_name =
//                    outfileid[j] + "." + std::to_string(num_blocks_done + tid);
//                compress_id_block(outfile_name.c_str(),
//                                  id_array + tid * num_reads_per_block,
//                                  num_reads_thr);
//              }
//              // Compress qualities
//              if (cp.preserve_quality) {
//                std::string outfile_name =
//                    outfilequality[j] + "." +
//                    std::to_string(num_blocks_done + tid);
//                bsc::BSC_str_array_compress(
//                    outfile_name.c_str(),
//                    quality_array + tid * num_reads_per_block, num_reads_thr,
//                    read_lengths_array + tid * num_reads_per_block);
//              }
//            }
//          } else {
//            // Compress read lengths file
//            std::string infile_name = outfilereadlength[j] + "." +
//                                      std::to_string(num_blocks_done + tid);
//            std::string outfile_name = outfilereadlength[j] + "." +
//                                       std::to_string(num_blocks_done + tid) +
//                                       ".bsc";
//            bsc::BSC_compress(infile_name.c_str(), outfile_name.c_str());
//            remove(infile_name.c_str());
//            // Compress ids
//            if (cp.preserve_id) {
//              std::string outfile_name =
//                  outfileid[j] + "." + std::to_string(num_blocks_done + tid);
//              compress_id_block(outfile_name.c_str(),
//                                id_array + tid * num_reads_per_block,
//                                num_reads_thr);
//            }
//            // Compress qualities
//            if (cp.preserve_quality) {
//              std::string outfile_name = outfilequality[j] + "." +
//                                         std::to_string(num_blocks_done + tid);
//              bsc::BSC_str_array_compress(
//                  outfile_name.c_str(),
//                  quality_array + tid * num_reads_per_block, num_reads_thr,
//                  read_lengths_array + tid * num_reads_per_block);
//            }
//            // Compress reads
//            outfile_name =
//                outfileread[j] + "." + std::to_string(num_blocks_done + tid);
//            bsc::BSC_str_array_compress(
//                outfile_name.c_str(), read_array + tid * num_reads_per_block,
//                num_reads_thr, read_lengths_array + tid * num_reads_per_block);
//          }
                    }  // if(!done)
                }    // omp parallel
                // if id match not found in any thread, set to false
                if (cp.paired_end && (j == 1)) {
                    if (paired_id_match)
                        for (int tid = 0; tid < num_threads; tid++)
                            paired_id_match &= paired_id_match_array[tid];
                    if (!paired_id_match) paired_id_code = 0;
                }
//      if (!cp.long_flag) {
                // write reads and read_order_N to respective files
                for (uint32_t i = 0; i < num_reads_read; i++) {
                    if (!read_contains_N_array[i]) {
                        fout_clean[j] << fastqRecords[i].sequence << "\n";
                        num_reads_clean[j]++;
                    } else {
                        uint32_t pos_N = num_reads[j] + i;
                        fout_order_N[j].write((char *) &pos_N, sizeof(uint32_t));
                        fout_N[j] << fastqRecords[i].sequence << "\n";
                    }
                }
//        if (!cp.preserve_order) {
//          // write qualities to file
                for (uint32_t i = 0; i < num_reads_read; i++)
                    fout_quality[j] << fastqRecords[i].qualityScores << "\n";
                // write ids to file
                for (uint32_t i = 0; i < num_reads_read; i++)
                    fout_id[j] << fastqRecords[i].title << "\n";
//        }
//      }
                num_reads[j] += num_reads_read;
                max_readlen =
                        std::max(max_readlen,
                                 *(std::max_element(read_lengths_array,
                                                    read_lengths_array + num_reads_read)));
            }
            if (cp.paired_end)
                if (num_reads[0] != num_reads[1])
                    throw std::runtime_error(
                            "Number of reads in paired files do not match.");
            if (done[0] && done[1]) break;
            num_blocks_done += num_threads;
        }

        delete[] id_array_1;
        delete[] read_contains_N_array;
        delete[] read_lengths_array;
//  delete[] quality_binning_table;
        delete[] paired_id_match_array;
        // close files
//  if (cp.long_flag) {
//    fin[0].close();
//    if (cp.paired_end) fin[1].close();
//  } else {
        for (int j = 0; j < 2; j++) {
            if (j == 1 && !cp.paired_end) continue;
            fout_clean[j].close();
            fout_N[j].close();
            fout_order_N[j].close();
//      if (!cp.preserve_order) {
            if (cp.preserve_id) fout_id[j].close();
            if (cp.preserve_quality) fout_quality[j].close();
//      }
        }
//  }
        if (num_reads[0] == 0) throw std::runtime_error("No reads found.");

        if (cp.paired_end) {
            // merge input_N and input_order_N for the two files
            std::ofstream fout_N_PE(outfileN[0], std::ios::app);
            std::ifstream fin_N_PE(outfileN[1]);
            fout_N_PE << fin_N_PE.rdbuf();
            fout_N_PE.close();
            fin_N_PE.close();
            remove(outfileN[1].c_str());
            std::ofstream fout_order_N_PE(outfileorderN[0],
                                       std::ios::app | std::ios::binary);
            std::ifstream fin_order_N(outfileorderN[1], std::ios::binary);
            uint32_t num_N_file_2 = num_reads[1] - num_reads_clean[1];
            uint32_t order_N;
            for (uint32_t i = 0; i < num_N_file_2; i++) {
                fin_order_N.read((char *) &order_N, sizeof(uint32_t));
                order_N += num_reads[0];
                fout_order_N_PE.write((char *) &order_N, sizeof(uint32_t));
            }
            fin_order_N.close();
            fout_order_N_PE.close();
            remove(outfileorderN[1].c_str());
        }

        if (cp.paired_end && paired_id_match) {
            // delete id files for second file since we found a pattern
//    if (!cp.long_flag && !cp.preserve_order) {
            remove(outfileid[1].c_str());
//    } else {
//      uint32_t num_blocks =
//          1 +
//          (num_reads[0] - 1) /
//              num_reads_per_block;  // ceil of num_reads[0]/num_reads_per_block
//      for (uint32_t i = 0; i < num_blocks; i++)
//        remove((outfileid[1] + "." + std::to_string(i)).c_str());
//    }
        }
        cp.paired_id_code = paired_id_code;
        cp.paired_id_match = paired_id_match;
        cp.num_reads = num_reads[0] + num_reads[1];
        cp.num_reads_clean[0] = num_reads_clean[0];
        cp.num_reads_clean[1] = num_reads_clean[1];
        cp.max_readlen = max_readlen;

        std::cout << "Max Read length: " << cp.max_readlen << "\n";
        std::cout << "Total number of reads: " << cp.num_reads << "\n";

//  if (!cp.long_flag)
        std::cout << "Total number of reads without N: "
                  << cp.num_reads_clean[0] + cp.num_reads_clean[1] << "\n";
        if (cp.preserve_id && cp.paired_end)
            std::cout << "Paired id match code: " << (int) cp.paired_id_code << "\n";
    }

}  // namespace spring
