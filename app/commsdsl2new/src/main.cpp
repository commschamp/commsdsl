//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/version.h"
#include "commsdsl/gen/util.h"

#include "CommsProgramOptions.h"
#include "CommsGenerator.h"

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <fstream>

namespace commsdsl2new
{

std::vector<std::string> getFilesList(
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

        result = commsdsl::gen::util::strSplitByAnyChar(contents, "\r\n");
        if (prefix.empty()) {
            break;
        }

        for (auto& f : result) {
            f = commsdsl::gen::util::pathAddElem(prefix, f);
        }
    } while (false);
    return result;
}

} // namespace commsdsl2new
int main(int argc, const char* argv[])
{
    try {
        commsdsl2new::CommsProgramOptions options;
        options.parse(argc, argv);
        if (options.helpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n\n";
            std::cout << options.helpStr();
            return 0;
        }

        if (options.versionRequested()) {
            std::cout << 
                commsdsl::versionMajor() << '.' << 
                commsdsl::versionMinor() << '.' <<
                commsdsl::versionPatch() << std::endl;
            return 0;
        }        

        commsdsl2new::CommsGenerator generator;
        auto& logger = generator.logger();

        if (options.quietRequested()) {
            logger.setMinLevel(commsdsl::parse::ErrorLevel_Warning);
        }

        if (options.debugRequested()) {
            logger.setMinLevel(commsdsl::parse::ErrorLevel_Debug);
        }        

        if (options.warnAsErrRequested()) {
            logger.setWarnAsError();
        }

        if (options.hasNamespaceOverride()) {
            generator.setMainNamespaceOverride(options.getNamespace());
        }

        generator.setOutputDir(options.getOutputDirectory());
        generator.setVersionIndependentCodeForced(options.versionIndependentCodeRequested());
        generator.setCustomizationLevel(options.getCustomizationLevel());
        generator.setProtocolVersion(options.getProtocolVersion());
        generator.setCodeDir(options.getCodeInputDirectory());
        generator.setExtraInputBundles(options.getExtraInputBundles());

        auto files = commsdsl2new::getFilesList(options.getFilesListFile(), options.getFilesListPrefix());
        auto otherFiles = options.getFiles();
        files.insert(files.end(), otherFiles.begin(), otherFiles.end());

        if (files.empty()) {
            logger.error("No intput files are provided");
            return -1;
        }

        if (!generator.prepare(files)) {
            return -1;
        }

        if (!generator.write()) {
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
