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

    std::string getFilesListFile() const;
    std::string getFilesListPrefix() const;
    std::vector<std::string> getFiles() const;
    std::string getOutputDirectory() const;
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
