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

#include "EmscriptenGenerator.h"
#include "EmscriptenProgramOptions.h"

#include "commsdsl/version.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace commsdsl2emscripten
{

std::vector<std::string> emscriptenGetFilesList(
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

} // namespace commsdsl2emscripten

int main(int argc, const char* argv[])
{
    try {
        commsdsl2emscripten::EmscriptenProgramOptions options;
        options.genParse(argc, argv);
        if (options.genHelpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n\n";
            std::cout << options.genHelpStr();
            return 0;
        }

        if (options.emscriptenVersionRequested()) {
            std::cout << 
                commsdsl::versionMajor() << '.' << 
                commsdsl::versionMinor() << '.' <<
                commsdsl::versionPatch() << std::endl;
            return 0;
        }        

        commsdsl2emscripten::EmscriptenGenerator generator;
        auto& logger = generator.genLogger();

        if (options.emscriptenQuietRequested()) {
            logger.genSetMinLevel(commsdsl::parse::ParseErrorLevel_Warning);
        }

        if (options.emscriptenWarnAsErrRequested()) {
            logger.genSetWarnAsError();
        }

        if (options.emscriptenHasNamespaceOverride()) {
            generator.genSetNamespaceOverride(options.emscriptenGetNamespace());
        }

        if (options.emscriptenHasForcedInterface()) {
            generator.emscriptenSetForcedInterface(options.emscriptenGetForcedInterface());
        }

        generator.genSetOutputDir(options.emscriptenGetOutputDirectory());
        generator.genSetCodeDir(options.emscriptenGetCodeInputDirectory());
        generator.genSetMultipleSchemasEnabled(options.emscriptenMultipleSchemasEnabled());
        generator.genSetMinRemoteVersion(options.emscriptenGetMinRemoteVersion());
        generator.emscriptenSetMainNamespaceInNamesForced(options.emscriptenIsMainNamespaceInNamesForced());
        generator.emscriptenSetHasProtocolVersion(options.emscriptenHasProtocolVersion());
        generator.emscriptenSetMessagesListFile(options.emscriptenMessagesListFile());
        generator.emscriptenSetForcedPlatform(options.emscriptenForcedPlatform());
        generator.genSetTopNamespace("cc_emscripten");

        auto files = commsdsl2emscripten::emscriptenGetFilesList(options.emscriptenGetFilesListFile(), options.emscriptenGetFilesListPrefix());
        auto otherFiles = options.emscriptenGetFiles();
        files.insert(files.end(), otherFiles.begin(), otherFiles.end());

        if (files.empty()) {
            logger.genError("No input files are provided");
            return -1;
        }

        if (!generator.genPrepare(files)) {
            logger.genError("Failed to prepare");
            return -1;
        }

        if (!generator.genWrite()) {
            logger.genError("Failed to write");
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
