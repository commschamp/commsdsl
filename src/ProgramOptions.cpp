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

#include "ProgramOptions.h"
#include <iostream>
#include <cassert>
#include <vector>

namespace po = boost::program_options;
namespace commsdsl2comms
{

namespace
{

const std::string HelpStr("help");
const std::string FullHelpStr(HelpStr + ",h");
const std::string QuietStr("quiet");
const std::string FullQuietStr(QuietStr + ",q");
const std::string OutputDirStr("output-dir");
const std::string FullOutputDirStr(OutputDirStr + ",o");
const std::string InputFilesListStr("input-files-list");
const std::string FullInputFilesListStr(InputFilesListStr + ",i");
const std::string InputFilesPrefixStr("input-files-prefix");
const std::string FullInputFilesPrefixStr(InputFilesPrefixStr + ",p");
const std::string NamespaceStr("namespace");
const std::string FullNamespaceStr(NamespaceStr + ",n");
const std::string ForceVerStr("force-version");
const std::string FullForceVerStr(ForceVerStr + ",V");
const std::string MinRemoteVerStr("min-remote-version");
const std::string FullMinRemoteVerStr(MinRemoteVerStr + ",m");
const std::string InputFileStr("input-file");
const std::string CommsChampionTagStr("cc-tag");
const std::string WarnAsErrStr("warn-as-err");

po::options_description createDescription()
{
    po::options_description desc("Options");
    desc.add_options()
        (FullHelpStr.c_str(), "This help.")
        (FullQuietStr.c_str(), "Quiet, show only warnings and errors.")
        (FullOutputDirStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Output directory path. Empty means current.")
        (FullInputFilesListStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "File containing list of input files.")
        (FullInputFilesPrefixStr.c_str(), po::value<std::string>()->default_value(std::string()),
            "Prefix for the values from the list file.")
        (FullNamespaceStr.c_str(), po::value<std::string>(),
            "Force protocol namespace. Defaults to package name defined in the schema.")
        (FullForceVerStr.c_str(), po::value<unsigned>(),
            "Force schema version. Must not be greater than version specified in schema file.")
        (FullMinRemoteVerStr.c_str(), po::value<unsigned>()->default_value(0U),
            "Set minimal supported remote version. Defaults to 0.")
        (CommsChampionTagStr.c_str(), po::value<std::string>()->default_value("v0.25"),
            "Default tag/branch of the CommsChampion project.")
        (WarnAsErrStr.c_str(), "Treat warning as error.")
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

}

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

bool ProgramOptions::warnAsErrRequested() const
{
    return 0 < m_vm.count(WarnAsErrStr);
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

std::string ProgramOptions::getCommsChampionTag() const
{
    return m_vm[CommsChampionTagStr].as<std::string>();
}

// namespace

} // namespace commsdsl2comms

