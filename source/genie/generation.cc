// Copyright 2018 The genie authors


/**
 *  @file generation.c
 *  @brief Main entry point for descriptor stream generation algorithms
 *  @author Jan Voges
 *  @bug No known bugs
 */


#include "genie/generation.h"

#include <iostream>
#include <string>
#include <vector>
#include <utils/MPEGGFileCreation/MPEGGFileCreator.h>
#include <fileio/gabac_file.h>
#include <boost/filesystem.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "genie/exceptions.h"
#include "fileio/fasta_file_reader.h"
#include "fileio/fasta_record.h"
#include "fileio/fastq_file_reader.h"
#include "fileio/fastq_record.h"
#include "fileio/sam_file_reader.h"
#include "fileio/sam_record.h"
#include "coding/spring/spring.h"
#include "genie/genie_file_format.h"
#include "genie/gabac_integration.h"

namespace spring {
void decompress(const std::string& temp_dir,
                const std::vector<std::map<uint8_t, std::map<uint8_t, std::string>>>& ref_descriptorFilesPerAU,
                uint32_t num_blocks,
                bool eru_abs_flag,
                bool paired_end
);
}

namespace dsg {

bool copyDir(
        boost::filesystem::path const & source,
        boost::filesystem::path const & destination
)
{
    namespace fs = boost::filesystem;
    try
    {
        // Check whether the function call is valid
        if(
                !fs::exists(source) ||
                !fs::is_directory(source)
                )
        {
            std::cerr << "Source directory " << source.string()
                      << " does not exist or is not a directory." << '\n'
                    ;
            return false;
        }
        if(fs::exists(destination))
        {
            std::cerr << "Destination directory " << destination.string()
                      << " already exists." << '\n'
                    ;
            return false;
        }
        // Create the destination directory
        if(!fs::create_directory(destination))
        {
            std::cerr << "Unable to create destination directory"
                      << destination.string() << '\n'
                    ;
            return false;
        }
    }
    catch(fs::filesystem_error const & e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    // Iterate through the source directory
    for(
            fs::directory_iterator file(source);
            file != fs::directory_iterator(); ++file
            )
    {
        try
        {
            fs::path current(file->path());
            if(fs::is_directory(current))
            {
                // Found directory: Recursion
                if(
                        !copyDir(
                                current,
                                destination / current.filename()
                        )
                        )
                {
                    return false;
                }
            }
            else
            {
                // Found file: Copy
                fs::copy_file(
                        current,
                        destination / current.filename()
                );
            }
        }
        catch(fs::filesystem_error const & e)
        {
            std:: cerr << e.what() << '\n';
        }
    }
    return true;
}

static void generationFromFasta(
        const ProgramOptions& programOptions
){
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
        const ProgramOptions& programOptions
){
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
    if (!programOptions.inputFilePairPath.empty()) {
        std::cout << "Paired file:\n";
        // Initialize a FASTQ file reader.
        input::fastq::FastqFileReader fastqFileReader1(programOptions.inputFilePairPath);

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

static generated_aus generationFromFastq_SPRING(
        const ProgramOptions& programOptions
){
    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Descriptor stream generation from FASTQ file" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    bool paired_end = false;
    // Initialize a FASTQ file reader.
    input::fastq::FastqFileReader fastqFileReader1(programOptions.inputFilePath);
    std::cout << "Calling SPRING" << std::endl;
    if (programOptions.inputFilePairPath.empty()) {
        return spring::generate_streams_SPRING(
                &fastqFileReader1,
                &fastqFileReader1,
                programOptions.numThreads,
                paired_end,
                programOptions.workingDirectory
        );
    } else {
        paired_end = true;
        input::fastq::FastqFileReader fastqFileReader2(programOptions.inputFilePairPath);
        return spring::generate_streams_SPRING(
                &fastqFileReader1,
                &fastqFileReader2,
                programOptions.numThreads,
                paired_end,
                programOptions.workingDirectory
        );
    }
}


static void generationFromSam(
        const ProgramOptions& programOptions
){
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

void decompression(
        const ProgramOptions& programOptions
){
    // Open file and create tmp directory with random name
    FILE *in = fopen(programOptions.inputFilePath.c_str(), "rb");
    if (!in) {
        throw std::runtime_error("Could not open input file");
    }
    std::string temp_dir;
    while (true) {
        std::string random_str = "tmp." + spring::random_string(10);
        temp_dir = "./" + random_str + '/';
        if (!boost::filesystem::exists(temp_dir)) {
            break;
        }
    }
    if (!boost::filesystem::create_directory(temp_dir)) {
        throw std::runtime_error("Cannot create temporary directory.");
    }
    std::cout << "Temporary directory: " << temp_dir << "\n";

    // Unpack
    std::cout << "Starting decompression...\n";
    auto flist =
            unpackFiles(temp_dir, in);
    fclose(in);

    //copyDir(temp_dir,temp_dir + "/../genie_comp");

    // Decompress
    run_gabac(flist, programOptions.configPath, true, programOptions.numThreads);

    //copyDir(temp_dir,temp_dir + "/../genie_uncomp");

    // Extract spring parameters
    spring::compression_params cp;
    FILE *cpfile = fopen((temp_dir + "cp.bin").c_str(), "rb");
    if (!cpfile) {
        throw std::runtime_error("Cannot open config file");
    }
    if (fread((uint8_t *) &cp, sizeof(spring::compression_params), 1, cpfile) != 1) {
        fclose(cpfile);
        throw std::runtime_error("Cannot read config");
    }
    fclose(cpfile);
    uint32_t num_blocks;
    if (!cp.paired_end) {
        num_blocks = 1 + (cp.num_reads - 1) / cp.num_reads_per_block;
    } else {
        num_blocks = 1 + (cp.num_reads / 2 - 1) / cp.num_reads_per_block;
    }

    // Decode spring streams
    spring::decompress(temp_dir, createMap(flist), num_blocks, cp.preserve_order, cp.paired_end);

    // Finish fastq
    std::string outname = programOptions.inputFilePath;
    outname = outname.substr(0, outname.find_last_of('.'));
    if(cp.paired_end) {
        outname += "_decompressed_1.fastq";
        std::rename((temp_dir + "decompressed_1.fastq").c_str(), outname.c_str());
        outname = programOptions.inputFilePath;
        outname = outname.substr(0, outname.find_last_of('.'));
        outname += "_decompressed_2.fastq";
        std::rename((temp_dir + "decompressed_2.fastq").c_str(), outname.c_str());
    } else {
        outname += "_decompressed.fastq";
        std::rename((temp_dir + "decompressed.fastq").c_str(), outname.c_str());
    }

    boost::filesystem::remove_all(temp_dir);
}

void generation(
        const ProgramOptions& programOptions
){
    if (programOptions.inputFileType == "FASTA") {
        generationFromFasta(programOptions);
    } else if (programOptions.inputFileType == "FASTQ") {
        if (programOptions.readAlgorithm == "HARC") {
            auto generated_aus = generationFromFastq_SPRING(programOptions);

            // Open output directory of spring
            std::vector<std::string> filelist;
            std::string path = generated_aus.getGeneratedAusRef().getRefAus().front().begin()->second.begin()->second;
            path = path.substr(0, path.find_last_of('/') + 1);
            boost::filesystem::path p(path);
            boost::filesystem::directory_iterator end_itr;

            // Remove temporary files
            std::remove((path + "read_order.bin").c_str());
            std::remove((path + "read_seq.txt").c_str());

            // Create list of all files
            for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr) {
                if (is_regular_file(itr->path())) {
                    std::string current_file = itr->path().string();
                    filelist.push_back(current_file);
                }
            }

           // copyDir(path,path + "/../genie_orig");

            // Compress
            run_gabac(filelist, programOptions.configPath, false, programOptions.numThreads);


            // Pack
            std::string outfile = (programOptions.inputFilePath.substr(0, programOptions.inputFilePath.find_last_of('.')) + ".genie");
            FILE *output = fopen(
                    outfile.c_str(), "wb"
            );
            if (!output) {
                throw std::runtime_error("Could not open output file");
            }
            packFiles(filelist, output);
            fclose(output);

            size_t orgSize = boost::filesystem::file_size(programOptions.inputFilePath);
            if(!programOptions.inputFilePairPath.empty()) {
                orgSize += boost::filesystem::file_size(programOptions.inputFilePairPath);
            }

            // Finish
            std::cout << "**** Finished ****" << std::endl;
            std::cout << "Compressed "
                    << orgSize
                    << " to "
                    << boost::filesystem::file_size(outfile)
                    << ". Compression rate "
                    << float(boost::filesystem::file_size(outfile)) /
                       boost::filesystem::file_size(programOptions.inputFilePath) *
                       100
                    << "%"
                    << std::endl;

            boost::filesystem::remove_all(path);

        } else {
            generationFromFastq(programOptions);
        }
    } else if (programOptions.inputFileType == "SAM") {
        generationFromSam(programOptions);
    } else {
        throwRuntimeError("wrong input file type");
    }
}

void fastqSpringResultToFile(generated_aus generatedAus){
    MPEGGFileCreator fileCreator;
    DatasetGroup *datasetGroup1 = fileCreator.addDatasetGroup();

    datasetGroup1->addInternalReference("ref1", "seq1", generatedAus.getGeneratedAusRef());

    std::vector<Class_type> existing_classes;
    existing_classes.push_back(CLASS_P);
    existing_classes.push_back(CLASS_M);

    std::map<Class_type, std::vector<uint8_t>> descriptorsIdPerClass;
    descriptorsIdPerClass[CLASS_P].push_back(0);
    descriptorsIdPerClass[CLASS_P].push_back(1);
    descriptorsIdPerClass[CLASS_M].push_back(0);
    descriptorsIdPerClass[CLASS_M].push_back(1);


    std::map<uint16_t, std::map<Class_type, std::vector<AccessUnit>>> accessUnitsAligned;
    std::vector<AccessUnit> accessUnitsUnaligned;

    for (const auto& accessUnitEntry : generatedAus.getEncodedFastqAus()) {
        accessUnitsUnaligned.emplace_back(
                accessUnitEntry,
                0,
                0,
                CLASS_U,
                0,
                0
        );
    }

    fileCreator.write("testOutput0");
}


}  // namespace dsg
