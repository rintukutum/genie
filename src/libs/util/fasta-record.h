#ifndef UTIL_FASTA_RECORD_H_
#define UTIL_FASTA_RECORD_H_

#include <string>

namespace util {

struct FastaRecord {
   public:
    FastaRecord(std::string header, std::string sequence);
    ~FastaRecord();

   public:
    std::string header;
    std::string sequence;
};

}  // namespace util

#endif  // UTIL_FASTA_RECORD_H_
