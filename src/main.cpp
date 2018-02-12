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

//#include <boost/filesystem.hpp>

#include "bbmp/Protocol.h"

#include "ProgramOptions.h"

//namespace bf = boost::filesystem;

namespace bbmp2comms
{

} // namespace bbmp2comms

int main(int argc, const char* argv[])
{
    bbmp2comms::ProgramOptions options;
    options.parse(argc, argv);
    if (options.helpRequested()) {
        std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n";
        options.printHelp(std::cout);
        return 0;
    }

    auto files = options.getFiles();
    if (files.empty()) {
        std::cerr << "ERROR: No file is provided" << std::endl;
        return -1;
    }

    std::cout << "Files count: " << files.size() << std::endl;
    bbmp::Protocol protocol;
    for (auto& f : files) {
        std::cout << "\t" << f << '\n';
        if (!protocol.parse(f)) {
            std::cerr << "ERROR: Failed to parse " << f << std::endl;
            return -1;
        }
    }
    std::cout << std::endl;

    bool result = true;
    if (result) {
        std::cout << "SUCCESS" << std::endl;
        return 0;
    }

    return -1;
}
