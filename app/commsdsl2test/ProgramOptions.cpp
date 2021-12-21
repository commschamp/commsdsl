//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "ProgramOptions.h"
#include <iostream>
#include <cassert>
#include <vector>

namespace po = boost::program_options;
namespace commsdsl2test
{

namespace
{

const std::string HelpStr("help");
const std::string FullHelpStr(HelpStr + ",h");
const std::string QuietStr("quiet");
const std::string FullQuietStr(QuietStr + ",q");
const std::string VersionStr("version");
const std::string OutputDirStr("output-dir");
const std::string FullOutputDirStr(OutputDirStr + ",o");
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr(CodeInputDirStr + ",c");
const std::string InputFilesListStr("input-files-list");
const std::string FullInputFilesListStr(InputFilesListStr + ",i");
const std::string InputFilesPrefixStr("input-files-prefix");
const std::string FullInputFilesPrefixStr(InputFilesPrefixStr + ",p");
const std::string NamespaceStr("namespace");
const std::string FullNamespaceStr(NamespaceStr + ",n");
const std::string ForceVerStr("force-schema-version");
const std::string ProtocolVerStr("protocol-version");
const std::string FullProtocolVerStr(ProtocolVerStr + ",V");
const std::string MinRemoteVerStr("min-remote-version");
const std::string FullMinRemoteVerStr(MinRemoteVerStr + ",m");
const std::string InputFileStr("input-file");
const std::string WarnAsErrStr("warn-as-err");
const std::string VersionIndependentCodeStr("version-independent-code");

po::options_description createDescription()
{
    po::options_description desc("Options");
    desc.add_options()
        (FullHelpStr.c_str(), "This help.")
        (VersionStr.c_str(), "Print version string and exit.")        
        (FullQuietStr.c_str(), "Quiet, show only warnings and errors.")
        (FullOutputDirStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Output directory path. Empty means current.")
        (FullCodeInputDirStr.c_str(), po::value<std::vector<std::string> >(),
            "Directory with code updates. Multiple directories are supported, later one takes priority.")
        (FullInputFilesListStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "File containing list of input files.")
        (FullInputFilesPrefixStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Prefix for the values from the list file.")
        (FullNamespaceStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Force protocol namespace. Defaults to schema name.")
        (ForceVerStr.c_str(), po::value<unsigned>(),
            "Force schema version. Must not be greater than version specified in schema file.")
        (FullProtocolVerStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Specify semantic version of the generated protocol code using <major>.<minor>.<patch> format to "
            "make this information available in the generated code")
        (FullMinRemoteVerStr.c_str(), po::value<unsigned>()->default_value(0U),
            "Set minimal supported remote version. Defaults to 0.")
        (WarnAsErrStr.c_str(), "Treat warning as error.")
        (VersionIndependentCodeStr.c_str(),
            "By default the generated code is version dependent if at least one defined "
            "interface has \"version\" field. Use this switch to forcefully disable generation "
            "of version denendent code.")
    ;
    return desc;
}

const po::options_description& getDescription()
{
    static const auto Desc = createDescription();
    return Desc;
}

po::options_description createHidden()
{
    po::options_description desc("Hidden");
    desc.add_options()
        (InputFileStr.c_str(), po::value< std::vector<std::string> >(), "input file")
    ;
    return desc;
}

const po::options_description& getHidden()
{
    static const auto Desc = createHidden();
    return Desc;
}

po::positional_options_description createPositional()
{
    po::positional_options_description desc;
    desc.add(InputFileStr.c_str(), -1);
    return desc;
}

const po::positional_options_description& getPositional()
{
    static const auto Desc = createPositional();
    return Desc;
}

} // namespace

void ProgramOptions::parse(int argc, const char* argv[])
{
    po::options_description allOptions;
    allOptions.add(getDescription()).add(getHidden());
    auto parseResult =
        po::command_line_parser(argc, argv)
            .options(allOptions)
            .positional(getPositional())
            .run();
    po::store(parseResult, m_vm);
    po::notify(m_vm);
}

void ProgramOptions::printHelp(std::ostream& out)
{
    out << getDescription() << std::endl;
}

bool ProgramOptions::helpRequested() const
{
    return 0 < m_vm.count(HelpStr);
}

bool ProgramOptions::quietRequested() const
{
    return 0 < m_vm.count(QuietStr);
}

bool ProgramOptions::versionRequested() const
{
    return 0 < m_vm.count(VersionStr);
}

bool ProgramOptions::warnAsErrRequested() const
{
    return 0 < m_vm.count(WarnAsErrStr);
}

bool ProgramOptions::versionIndependentCodeRequested() const
{
    return 0 < m_vm.count(VersionIndependentCodeStr);
}

std::string ProgramOptions::getFilesListFile() const
{
    return m_vm[InputFilesListStr].as<std::string>();
}

std::string ProgramOptions::getFilesListPrefix() const
{
    return m_vm[InputFilesPrefixStr].as<std::string>();
}

std::vector<std::string> ProgramOptions::getFiles() const
{
    if (m_vm.count(InputFileStr) == 0U) {
        return std::vector<std::string>();
    }

    auto inputs = m_vm[InputFileStr].as<std::vector<std::string> >();
    assert(!inputs.empty());
    return inputs;
}

std::string ProgramOptions::getOutputDirectory() const
{
    return m_vm[OutputDirStr].as<std::string>();
}

std::vector<std::string> ProgramOptions::getCodeInputDirectories() const
{
    if (m_vm.count(CodeInputDirStr) == 0U) {
        return std::vector<std::string>();
    }

    return m_vm[CodeInputDirStr].as<std::vector<std::string> >();
}

bool ProgramOptions::hasNamespaceOverride() const
{
    return 0U < m_vm.count(NamespaceStr);
}

std::string ProgramOptions::getNamespace() const
{
    return m_vm[NamespaceStr].as<std::string>();
}

bool ProgramOptions::hasForcedSchemaVersion() const
{
    return 0U < m_vm.count(ForceVerStr);
}

unsigned ProgramOptions::getForcedSchemaVersion() const
{
    return m_vm[ForceVerStr].as<unsigned>();
}

unsigned ProgramOptions::getMinRemoteVersion() const
{
    return m_vm[MinRemoteVerStr].as<unsigned>();
}

std::string ProgramOptions::getProtocolVersion() const
{
    return m_vm[ProtocolVerStr].as<std::string>();
}

} // namespace commsdsl2test
