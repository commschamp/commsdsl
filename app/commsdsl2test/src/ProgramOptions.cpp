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

namespace commsdsl2test
{

namespace
{

const std::string QuietStr("quiet");
const std::string FullQuietStr("q," + QuietStr);
const std::string VersionStr("version");
const std::string OutputDirStr("output-dir");
const std::string FullOutputDirStr("o," + OutputDirStr);
const std::string InputFilesListStr("input-files-list");
const std::string FullInputFilesListStr("i," + InputFilesListStr);
const std::string InputFilesPrefixStr("input-files-prefix");
const std::string FullInputFilesPrefixStr("p," + InputFilesPrefixStr);
const std::string NamespaceStr("namespace");
const std::string FullNamespaceStr("n," + NamespaceStr);
const std::string InputFileStr("input-file");
const std::string WarnAsErrStr("warn-as-err");

} // namespace

ProgramOptions::ProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr.c_str(), "Quiet, show only warnings and errors.")
    (FullOutputDirStr.c_str(), "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr.c_str(), "File containing list of input files.", true)        
    (FullInputFilesPrefixStr.c_str(), "Prefix for the values from the list file.", true)
    (FullNamespaceStr.c_str(), "Force protocol namespace. Defaults to schema name.", true) 
    (WarnAsErrStr.c_str(), "Treat warning as error.")
    ;
}

bool ProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool ProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool ProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& ProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& ProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const ProgramOptions::ArgsList& ProgramOptions::getFiles() const
{
    return args();
}

const std::string& ProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

bool ProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& ProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

} // namespace commsdsl2test
