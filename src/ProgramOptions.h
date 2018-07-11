#pragma once

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/program_options.hpp>

namespace commsdsl2comms
{

class ProgramOptions
{
public:
    void parse(int argc, const char* argv[]);
    static void printHelp(std::ostream& out);

    bool helpRequested() const;
    bool quietRequested() const;
    bool warnAsErrRequested() const;
    bool versionIndependentCodeRequested() const;

    std::string getFilesListFile() const;
    std::string getFilesListPrefix() const;
    std::vector<std::string> getFiles() const;
    std::string getOutputDirectory() const;
    std::string getCodeInputDirectory() const;
    bool hasNamespaceOverride() const;
    std::string getNamespace() const;
    bool hasForcedSchemaVersion() const;
    unsigned getForcedSchemaVersion() const;
    unsigned getMinRemoteVersion() const;
    std::string getCommsChampionTag() const;
private:
    boost::program_options::variables_map m_vm;
};

} // namespace commsdsl2comms
