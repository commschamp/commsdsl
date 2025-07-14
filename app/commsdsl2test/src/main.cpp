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

#include "TestGenerator.h"
#include "TestProgramOptions.h"

#include "commsdsl/version.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace commsdsl2test
{

std::vector<std::string> testGetFilesList(
    const std::string& fileName,
    const std::string& prefix)
{
    std::vector<std::string> result;
    do {
        if (fileName.empty()) {
            break;
        }
        
        std::ifstream stream(fileName);
        if (!stream) {
            break;
        }
        
        std::string contents(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));

        result = commsdsl::gen::util::genStrSplitByAnyChar(contents, "\r\n");
        if (prefix.empty()) {
            break;
        }

        for (auto& f : result) {
            f = commsdsl::gen::util::genPathAddElem(prefix, f);
        }
    } while (false);
    return result;
}

} // namespace commsdsl2test
int main(int argc, const char* argv[])
{
    try {
        commsdsl2test::TestProgramOptions options;
        options.genParse(argc, argv);
        if (options.genHelpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n\n";
            std::cout << options.genHelpStr();
            return 0;
        }

        if (options.testVersionRequested()) {
            std::cout << 
                commsdsl::versionMajor() << '.' << 
                commsdsl::versionMinor() << '.' <<
                commsdsl::versionPatch() << std::endl;
            return 0;
        }        

        commsdsl2test::TestGenerator generator;
        auto& logger = generator.genLogger();

        if (options.testQuietRequested()) {
            logger.genSetMinLevel(commsdsl::parse::ParseErrorLevel_Warning);
        }

        if (options.testWarnAsErrRequested()) {
            logger.genSetWarnAsError();
        }

        if (options.testHasNamespaceOverride()) {
            generator.genSetNamespaceOverride(options.testGetNamespace());
        }

        generator.genSetOutputDir(options.testGetOutputDirectory());
        generator.genSetCodeDir(options.testGetGetCodeInputDirectory());
        generator.genSetMultipleSchemasEnabled(options.testMultipleSchemasEnabled());

        auto files = commsdsl2test::testGetFilesList(options.testGetFilesListFile(), options.testGetFilesListPrefix());
        auto otherFiles = options.testGetFiles();
        files.insert(files.end(), otherFiles.begin(), otherFiles.end());

        if (files.empty()) {
            logger.genError("No input files are provided");
            return -1;
        }

        if (!generator.genPrepare(files)) {
            return -1;
        }

        if (!generator.genWrite()) {
            return -1;
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        assert(false);
    }

    return -1;
}
