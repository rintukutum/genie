// Copyright 2018 The genie authors


/**
 *  @file ProgramOptions.h
 *  @brief Program options interface
 *  @author Jan Voges
 *  @bug No known bugs
 */


#ifndef DSG_PROGRAMOPTIONS_H_
#define DSG_PROGRAMOPTIONS_H_


#include <string>


namespace dsg {


class ProgramOptions {
 public:
    ProgramOptions(
        int argc,
        char *argv[]);

    ~ProgramOptions(void);

    void print(void);

 public:
    // Generic
    bool force;
    bool verbose;
    std::string workingDirectory;

    // Input
    std::string inputFilePath;
    std::string inputFileType;

    // Algorithm
    std::string idAlgorithm;
    std::string qvAlgorithm;
    std::string readAlgorithm;

 private:
    void processCommandLine(
        int argc,
        char *argv[]);

    void validate(void);

    void validateDependencies(void);
};


}  // namespace dsg


#endif  // DSG_PROGRAMOPTIONS_H_
