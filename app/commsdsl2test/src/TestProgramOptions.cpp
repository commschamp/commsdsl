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

const std::string TestQuietStr("quiet");
const std::string TestFullQuietStr("q," + TestQuietStr);
const std::string FullVersionStr("version");
const std::string TestOutputDirStr("output-dir");
const std::string TestFullOutputDirStr("o," + TestOutputDirStr);
const std::string TestInputFilesListStr("input-files-list");
const std::string TestFullInputFilesListStr("i," + TestInputFilesListStr);
const std::string TestInputFilesPrefixStr("input-files-prefix");
const std::string TestFullInputFilesPrefixStr("p," + TestInputFilesPrefixStr);
const std::string TestNamespaceStr("namespace");
const std::string TestFullNamespaceStr("n," + TestNamespaceStr);
const std::string TestWarnAsErrStr("warn-as-err");
const std::string TestCodeInputDirStr("code-input-dir");
const std::string TestFullCodeInputDirStr("c," + TestCodeInputDirStr);
const std::string TestMultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string TestFullMultipleSchemasEnabledStr("s," + TestMultipleSchemasEnabledStr);

} // namespace

TestProgramOptions::TestProgramOptions()
{
    genAddHelpOption()
    (FullVersionStr, "Print version string and exit.")
    (TestFullQuietStr.c_str(), "Quiet, show only warnings and errors.")
    (TestFullOutputDirStr.c_str(), "Output directory path. When not provided current is used.", true)        
    (TestFullInputFilesListStr.c_str(), "File containing list of input files.", true)        
    (TestFullInputFilesPrefixStr.c_str(), "Prefix for the values from the list file.", true)
    (TestFullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (TestWarnAsErrStr.c_str(), "Treat warning as error.")
    (TestFullCodeInputDirStr, "Directory with code updates.", true)
    (TestFullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")    
    ;
}

bool TestProgramOptions::testQuietRequested() const
{
    return genIsOptUsed(TestQuietStr);
}

bool TestProgramOptions::testVersionRequested() const
{
    return genIsOptUsed(FullVersionStr);
}

bool TestProgramOptions::testWarnAsErrRequested() const
{
    return genIsOptUsed(TestWarnAsErrStr);
}

const std::string& TestProgramOptions::testGetFilesListFile() const
{
    return genValue(TestInputFilesListStr);
}

const std::string& TestProgramOptions::testGetFilesListPrefix() const
{
    return genValue(TestInputFilesPrefixStr);
}

const TestProgramOptions::GenArgsList& TestProgramOptions::testGetFiles() const
{
    return genArgs();
}

const std::string& TestProgramOptions::testGetOutputDirectory() const
{
    return genValue(TestOutputDirStr);
}

const std::string& TestProgramOptions::testGetGetCodeInputDirectory() const
{
    return genValue(TestCodeInputDirStr);
}

bool TestProgramOptions::testHasNamespaceOverride() const
{
    return genIsOptUsed(TestNamespaceStr);
}

const std::string& TestProgramOptions::testGetNamespace() const
{
    return genValue(TestNamespaceStr);
}

bool TestProgramOptions::testMultipleSchemasEnabled() const
{
    return genIsOptUsed(TestMultipleSchemasEnabledStr);
}

} // namespace commsdsl2test
