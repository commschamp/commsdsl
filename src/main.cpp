//
// Copyright 2017 (C). Alex Robenko. All rights reserved.
//

// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "commsdsl/Protocol.h"

#include "ProgramOptions.h"

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
        options.parse(argc, argv);
        if (options.helpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n";
            options.printHelp(std::cout);
            return 0;
        }

        auto files = commsdsl2comms::getFilesList(options.getFilesListFile(), options.getFilesListPrefix());
        auto otherFiles = options.getFiles();
        files.insert(files.end(), otherFiles.begin(), otherFiles.end());

        if (files.empty()) {
            std::cerr << "ERROR: No intput files are provided" << std::endl;
            return -1;
        }

        std::cout << "Files count: " << files.size() << std::endl;
        commsdsl::Protocol protocol;
        for (auto& f : files) {
            std::cout << "\tParsing " << f << '\n';
            if (!protocol.parse(f)) {
                std::cerr << "ERROR: Failed to parse " << f << std::endl;
                return -1;
            }
        }
        std::cout << std::endl;

        if (!protocol.validate()) {
            std::cerr << "ERROR: Validation failed" << std::endl;
            return -1;
        }

        auto schema = protocol.schema();
        std::cout << "NAME = " << schema.name() << std::endl;
        std::cout << "ID = " << schema.id() << std::endl;


        bool result = true;
        if (result) {
            std::cout << "SUCCESS" << std::endl;
            return 0;
        }
    }
    catch (...) {
        // Ignore exception
    }

    return -1;
}
