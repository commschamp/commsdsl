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
    bool pluginBuildEnabledByDefault() const;
    bool testsBuildEnabledByDefault() const;

    std::string getFilesListFile() const;
    std::string getFilesListPrefix() const;
    std::vector<std::string> getFiles() const;
    std::string getOutputDirectory() const;
    std::vector<std::string> getCodeInputDirectories() const;
    bool hasNamespaceOverride() const;
    std::string getNamespace() const;
    bool hasForcedSchemaVersion() const;
    unsigned getForcedSchemaVersion() const;
    unsigned getMinRemoteVersion() const;
    std::string getCommsChampionTag() const;
    std::vector<std::string> getPlugins() const;
    std::string getCustomizationLevel() const;
private:
    boost::program_options::variables_map m_vm;
};

} // namespace commsdsl2comms
