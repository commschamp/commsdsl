#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "commsdsl/Protocol.h"

#include "ProgramOptions.h"
#include "Logger.h"
#include "Generator.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

std::vector<std::string> getFilesList(
    const std::string& fileName,
    const std::string& prefix)
{
    std::vector<std::string> result;
    do {
        if (fileName.empty()) {
            break;
        }

        ba::split(result, fileName, ba::is_any_of("\r\n"), ba::token_compress_on);
        if (prefix.empty()) {
            break;
        }

        bf::path prefixPath(prefix);
        for (auto& f : result) {
            f = (prefixPath / f).string();
        }
    } while (false);
    return result;
}

} // namespace commsdsl2comms

int main(int argc, const char* argv[])
{
    try {
        commsdsl2comms::ProgramOptions options;
        commsdsl2comms::Logger logger;

        options.parse(argc, argv);
        if (options.helpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n";
            options.printHelp(std::cout);
            return 0;
        }

        if (options.quietRequested()) {
            logger.setMinLevel(commsdsl::ErrorLevel_Warning);
        }

        if (options.warnAsErrRequested()) {
            logger.setWarnAsError();
        }

        auto files = commsdsl2comms::getFilesList(options.getFilesListFile(), options.getFilesListPrefix());
        auto otherFiles = options.getFiles();
        files.insert(files.end(), otherFiles.begin(), otherFiles.end());

        if (files.empty()) {
            logger.log(commsdsl::ErrorLevel_Error, "No intput files are provided");
            return -1;
        }

        commsdsl2comms::Generator generator(logger);
        if (!generator.generate(files)) {
            return -1;
        }

        return 0;
    }
    catch (...) {
        assert(!"Unhandled exception should not happen");
        // Ignore exception
    }

    return -1;
}
