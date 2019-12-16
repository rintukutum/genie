/**
 * @file
 * @copyright This file is part of GENIE. See LICENSE and/or
 * https://github.com/mitogen/genie for more details.
 */

#include "gabac-compressor.h"
#include <format/mpegg_p2/parameter_set/descriptor_configuration_present/cabac/decoder_configuration_cabac.h>
#include <format/mpegg_p2/parameter_set/descriptor_configuration_present/descriptor_configuration_present.h>
#include <util/make_unique.h>

// ---------------------------------------------------------------------------------------------------------------------

void GabacCompressor::compress(const gabac::EncodingConfiguration &conf, gabac::DataBlock *in,
                               std::vector<gabac::DataBlock> *out) {
    // Interface to GABAC library
    gabac::IBufferStream bufferInputStream(in);
    lae::GenieGabacOutputStream bufferOutputStream;

    // Setup
    const size_t GABAC_BLOCK_SIZE = 0;  // 0 means single block (block size is equal to input size)
    std::ostream *const GABC_LOG_OUTPUT_STREAM = &std::cout;
    const gabac::IOConfiguration GABAC_IO_SETUP = {&bufferInputStream, &bufferOutputStream, GABAC_BLOCK_SIZE,
                                                   GABC_LOG_OUTPUT_STREAM, gabac::IOConfiguration::LogLevel::TRACE};
    const bool GABAC_DECODING_MODE = false;

    // Run
    gabac::run(GABAC_IO_SETUP, conf, GABAC_DECODING_MODE);

    // Translate to GENIE-Payload TODO: change gabac to directly output the right format
    bufferOutputStream.flush_blocks(out);
}

// ---------------------------------------------------------------------------------------------------------------------

void GabacCompressor::flowIn(std::unique_ptr<MpeggRawAu> raw_aus, size_t id) {
    auto payload = util::make_unique<BlockPayloadSet>(raw_aus->moveParameters());

    for (size_t desc = 0; desc < MpeggRawAu::NUM_DESCRIPTORS; ++desc) {
        auto descriptor_payload = util::make_unique<DescriptorPayload>();
        const auto NUM_SUBSEQS = MpeggRawAu::getDescriptorProperties()[desc].number_subsequences;
        for (size_t sub_desc = 0; sub_desc < NUM_SUBSEQS; ++sub_desc) {
            // Do the compression
            auto &input = raw_aus->get(MpeggRawAu::GenomicDescriptor(desc), sub_desc);
            gabac::DataBlock input_block((const uint8_t *)input.getData(), input.getWordSize(), input.rawSize());
            std::vector<gabac::DataBlock> out;
            compress(configSet.getConfAsGabac(MpeggRawAu::GenomicDescriptor(desc), sub_desc), &input_block, &out);

            // Add to payload
            auto subsequence_payload = util::make_unique<SubDescriptorPayload>();
            for (auto &o : out) {
                subsequence_payload->add(util::make_unique<TransformedPayload>(&o));
            }
            descriptor_payload->add(std::move(subsequence_payload));
        }
        if (!descriptor_payload->isEmpty()) {
            payload->setPayload(desc, std::move(descriptor_payload));
        }
    }

    configSet.storeParameters(payload->getParameters());

    flowOut(std::move(payload), id);
}

// ---------------------------------------------------------------------------------------------------------------------

void GabacCompressor::dryIn() { dryOut(); }

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
