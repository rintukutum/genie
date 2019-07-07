#include "encoding.h"

#include <fio/sam-file-reader.h>
#include <fio/sam-record.h>
#include <utils/log.h>

namespace lae {

    void encode(const ProgramOptions &programOptions) {
        LOG_DEBUG << "Encoding";
        LOG_INFO << "Input file: " << programOptions.inputFilePath;

        fio::SamFileReader samFileReader(programOptions.inputFilePath);

        size_t blockSize = 10000;

        while (true) {
            // Read a block of SAM records
            std::list<fio::SamRecord> samRecords;
            samFileReader.readRecords(blockSize, &samRecords);
            std::list<fio::SamRecord> samRecordsCopy(samRecords);
            LOG_TRACE << "Read " << samRecords.size() << " SAM record(s)";

            // Set up the decoded descriptor streams
            std::vector<int64_t> decodedPos;
            int64_t prevPos = 0;

            for (const auto &samRecord : samRecords) {
                // LOG_TRACE << "Encoding SAM record";
                // LOG_TRACE << samRecord.str();

                // LOG_TRACE << "Searching mate";
                // LOG_TRACE << "RNAME:POS = " << samRecord.rname << ":" << samRecord.pos;
                // LOG_TRACE << "RNEXT:PNEXT = " << samRecord.rnext << ":" << samRecord.pnext;

                // Search for mate
                std::string rnameSearchString;
                if (samRecord.rnext == "=") {
                    rnameSearchString = samRecord.rname;
                } else {
                    rnameSearchString = samRecord.rnext;
                }

                samRecordsCopy.erase(samRecordsCopy.begin()); // delete current record from the search space
                bool foundMate = false;
                for (auto it = samRecordsCopy.begin(); it != samRecordsCopy.end(); ++it) {
                    // LOG_TRACE << "Searching for RNAME:POS = " << rnameSearchString << ":" << samRecord.pnext;
                    // LOG_TRACE << "Current:      RNAME:POS = " << it->rname << ":" << it->pos;

                    if (it->rname == rnameSearchString && it->pos == samRecord.pnext) {
                        LOG_TRACE << "Found mate";
                        foundMate = true;
                        samRecordsCopy.erase(it);
                        break;
                    }

                }
                if (!foundMate) {
                    LOG_TRACE << "Did not find mate";
                }

                // pos
                decodedPos.push_back(samRecord.pos - prevPos);
                prevPos = samRecord.pos;

                // Break if everything was processed
                if (samRecordsCopy.empty()) {
                    break;
                }
            }

            // Break if less than blockSize records were read from the SAM file
            if (samRecords.size() < blockSize) {
                break;
            }
        }
    }

}  // namespace lae
