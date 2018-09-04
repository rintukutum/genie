// Copyright 2018 The genie authors


/**
 *  @file generation.c
 *  @brief Main entry point for descriptor stream generation algorithms
 *  @author Jan Voges
 *  @bug No known bugs
 */


#include "generation.h"

#include <iostream>
#include <string>
#include <vector>

#include "common/exceptions.h"
#include "input/fasta/FastaFileReader.h"
#include "input/fasta/FastaRecord.h"
#include "input/fastq/FastqFileReader.h"
#include "input/fastq/FastqRecord.h"
#include "input/sam/SamFileReader.h"
#include "input/sam/SamRecord.h"
#include "algorithms/SPRING/spring.h"

namespace dsg {


static void generationFromFasta(
    const ProgramOptions& programOptions)
{
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Descriptor stream generation from FASTA file" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // Initialize a FASTA file reader.
    input::fasta::FastaFileReader fastaFileReader(programOptions.inputFilePath);

    // Parse the entire file.
    std::vector<input::fasta::FastaRecord> fastaRecords;
    fastaFileReader.parse(&fastaRecords);

    // Print information about the FASTA file.
    std::cout << "FASTA file: " << programOptions.inputFilePath << std::endl;
    std::cout << "Records: " << fastaRecords.size() << std::endl;

    // Iterate through all FASTA records.
    size_t i = 0;
    for (const auto& fastaRecord : fastaRecords) {
        std::cout << "  " << i++ << ": ";
        std::cout << fastaRecord.header << "\t";
        std::cout << fastaRecord.sequence << std::endl;
    }

    // Read FASTA lines.
    std::cout << "Lines: " << std::endl;
    while (true) {
        std::string line("");
        fastaFileReader.readLine(&line);

        if (line.empty() == true) {
            break;
        }

        std::cout << line << std::endl;
    }
}


static void generationFromFastq(
    const ProgramOptions& programOptions)
{
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Descriptor stream generation from FASTQ file" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // Initialize a FASTQ file reader.
    input::fastq::FastqFileReader fastqFileReader(programOptions.inputFilePath);

    // Second pass for HARC.


    // Read FASTQ records in blocks of 10 records.
    size_t blockSize = 10;

    while (true) {
        std::vector<input::fastq::FastqRecord> fastqRecords;
        size_t numRecords = fastqFileReader.readRecords(blockSize, &fastqRecords);
        if (numRecords != blockSize) {
            std::cout << "Read only " << numRecords << " records (" << blockSize << " requested)." << std::endl;
        }

        // Iterate through the records.
        for (const auto& fastqRecord : fastqRecords) {
            std::cout << fastqRecord.title << "\t";
            std::cout << fastqRecord.sequence << "\t";
            std::cout << fastqRecord.optional << "\t";
            std::cout << fastqRecord.qualityScores << std::endl;
        }

        if (numRecords != blockSize) {
            break;
        }
    }
    if (!programOptions.inputPairFileName.empty()) {
        std::cout << "Paired file:\n";
        // Initialize a FASTQ file reader.
        input::fastq::FastqFileReader fastqFileReader1(programOptions.inputPairFileName);

        // Read FASTQ records in blocks of 10 records.
        size_t blockSize = 10;

        while (true) {
            std::vector<input::fastq::FastqRecord> fastqRecords;
            size_t numRecords = fastqFileReader1.readRecords(blockSize, &fastqRecords);
            if (numRecords != blockSize) {
                std::cout << "Read only " << numRecords << " records (" << blockSize << " requested)." << std::endl;
            }

            // Iterate through the records.
            for (const auto& fastqRecord : fastqRecords) {
                std::cout << fastqRecord.title << "\t";
                std::cout << fastqRecord.sequence << "\t";
                std::cout << fastqRecord.optional << "\t";
                std::cout << fastqRecord.qualityScores << std::endl;
            }

            if (numRecords != blockSize) {
                break;
            }
        }
    }
}


static void generationFromFastq_SPRING(
    const ProgramOptions& programOptions)
{
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Descriptor stream generation from FASTQ file" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    bool paired_end = false;
    // Initialize a FASTQ file reader.
    input::fastq::FastqFileReader fastqFileReader1(programOptions.inputFileName);
    std::cout << "Calling SPRING" << std::endl;
    if (programOptions.inputPairFileName.empty()) {
        spring::generate_streams_SPRING(&fastqFileReader1, &fastqFileReader1, programOptions.numThr, paired_end);
    } else {
        paired_end = true;
        input::fastq::FastqFileReader fastqFileReader2(programOptions.inputPairFileName);
        spring::generate_streams_SPRING(&fastqFileReader1, &fastqFileReader2, programOptions.numThr, paired_end);
    }
}


static void generationFromSam(
    const ProgramOptions& programOptions)
{
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Descriptor stream generation from SAM file" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // Initialize a SAM file reader.
    input::sam::SamFileReader samFileReader(programOptions.inputFilePath);

    // Get SAM header.
    std::cout << "-- SAM header begin" << std::endl;
    std::cout << samFileReader.header;
    std::cout << "-- SAM header end" << std::endl;

    // Read SAM records in blocks of 10 records.
    size_t blockSize = 10;

    while (true) {
        std::vector<input::sam::SamRecord> samRecords;
        size_t numRecords = samFileReader.readRecords(blockSize, &samRecords);
        if (numRecords != blockSize) {
            std::cout << "Read only " << numRecords << " records (" << blockSize << " requested)." << std::endl;
        }

        // Iterate through the records.
        for (const auto& samRecord : samRecords) {
            std::cout << samRecord.qname << "\t";
            std::cout << samRecord.rname << "\t";
            std::cout << samRecord.pos << "\t";
            std::cout << samRecord.cigar << "\t";
            std::cout << samRecord.seq << "\t";
            std::cout << samRecord.qual << std::endl;
        }

        if (numRecords != blockSize) {
            break;
        }
    }
}


void generation(
    const ProgramOptions& programOptions)
{
    if (programOptions.inputFileType == "FASTA") {
        generationFromFasta(programOptions);
    } else if (programOptions.inputFileType == "FASTQ") {
    //    generationFromFastq(programOptions);
        generationFromFastq_SPRING(programOptions);
    } else if (programOptions.inputFileType == "SAM") {
        generationFromSam(programOptions);
    } else {
        throwRuntimeError("wrong input file type");
    }
}


}  // namespace dsg
