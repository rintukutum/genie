project("util")

set(source_files
        bitwriter.cc
        exceptions.cc
        fasta-file-reader.cc
        fasta-record.cc
        fastq-file-reader.cc
        fastq-record.cc
        file-reader.cc
        log.cc
        sam-file-reader.cc
        sam-record.cc
        string-helpers.cc
)

add_library(util ${source_files})
