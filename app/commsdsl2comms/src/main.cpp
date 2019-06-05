//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iterator>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "commsdsl/Protocol.h"
#include "commsdsl/version.h"

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
        
        std::ifstream stream(fileName);
        if (!stream) {
            break;
        }
        
        std::string contents(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));

        ba::split(result, contents, ba::is_any_of("\r\n"), ba::token_compress_on);
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

        if (options.versionRequested()) {
            std::cout << 
                commsdsl::versionMajor() << '.' << 
                commsdsl::versionMinor() << '.' <<
                commsdsl::versionPatch() << std::endl;
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

        commsdsl2comms::Generator generator(options, logger);
        if (!generator.generate(files)) {
            return -1;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        assert(!"Unhandled exception should not happen");
        // Ignore exception
    }

    return -1;
}
