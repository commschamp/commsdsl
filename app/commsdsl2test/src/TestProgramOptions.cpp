//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "TestProgramOptions.h"

#include <cassert>
#include <iostream>
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
const std::string WarnAsErrStr("warn-as-err");
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr("c," + CodeInputDirStr);
const std::string MultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string FullMultipleSchemasEnabledStr("s," + MultipleSchemasEnabledStr);

} // namespace

TestProgramOptions::TestProgramOptions()
{
    genAddHelpOption()
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

bool TestProgramOptions::testQuietRequested() const
{
    return genIsOptUsed(QuietStr);
}

bool TestProgramOptions::testVersionRequested() const
{
    return genIsOptUsed(VersionStr);
}

bool TestProgramOptions::testWarnAsErrRequested() const
{
    return genIsOptUsed(WarnAsErrStr);
}

const std::string& TestProgramOptions::gestGetFilesListFile() const
{
    return genValue(InputFilesListStr);
}

const std::string& TestProgramOptions::testGetFilesListPrefix() const
{
    return genValue(InputFilesPrefixStr);
}

const TestProgramOptions::GenArgsList& TestProgramOptions::testGetFiles() const
{
    return genArgs();
}

const std::string& TestProgramOptions::testGetOutputDirectory() const
{
    return genValue(OutputDirStr);
}

const std::string& TestProgramOptions::getGetCodeInputDirectory() const
{
    return genValue(CodeInputDirStr);
}

bool TestProgramOptions::testHasNamespaceOverride() const
{
    return genIsOptUsed(NamespaceStr);
}

const std::string& TestProgramOptions::testGetNamespace() const
{
    return genValue(NamespaceStr);
}

bool TestProgramOptions::testMultipleSchemasEnabled() const
{
    return genIsOptUsed(MultipleSchemasEnabledStr);
}

} // namespace commsdsl2test
