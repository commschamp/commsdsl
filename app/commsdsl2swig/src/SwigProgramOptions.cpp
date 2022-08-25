//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigProgramOptions.h"

#include <iostream>
#include <cassert>
#include <vector>

namespace commsdsl2swig
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
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr("c," + CodeInputDirStr);
const std::string MultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string FullMultipleSchemasEnabledStr("s," + MultipleSchemasEnabledStr);

} // namespace

SwigProgramOptions::SwigProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr.c_str(), "Quiet, show only warnings and errors.")
    (FullOutputDirStr.c_str(), "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr.c_str(), "File containing list of input files.", true)        
    (FullInputFilesPrefixStr.c_str(), "Prefix for the values from the list file.", true)
    (FullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (WarnAsErrStr.c_str(), "Treat warning as error.")
    (FullCodeInputDirStr, "Directory with code updates.", true)
    (FullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")    
    ;
}

bool SwigProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool SwigProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool SwigProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& SwigProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& SwigProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const SwigProgramOptions::ArgsList& SwigProgramOptions::getFiles() const
{
    return args();
}

const std::string& SwigProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

const std::string& SwigProgramOptions::getCodeInputDirectory() const
{
    return value(CodeInputDirStr);
}

bool SwigProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& SwigProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

bool SwigProgramOptions::multipleSchemasEnabled() const
{
    return isOptUsed(MultipleSchemasEnabledStr);
}

} // namespace commsdsl2swig
