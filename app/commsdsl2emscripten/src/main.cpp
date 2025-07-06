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

        if (options.versionRequested()) {
            std::cout << 
                commsdsl::versionMajor() << '.' << 
                commsdsl::versionMinor() << '.' <<
                commsdsl::versionPatch() << std::endl;
            return 0;
        }        

        commsdsl2emscripten::EmscriptenGenerator generator;
        auto& logger = generator.genLogger();

        if (options.quietRequested()) {
            logger.genSetMinLevel(commsdsl::parse::ParseErrorLevel_Warning);
        }

        if (options.warnAsErrRequested()) {
            logger.genSetWarnAsError();
        }

        if (options.hasNamespaceOverride()) {
            generator.genSetNamespaceOverride(options.getNamespace());
        }

        if (options.hasForcedInterface()) {
            generator.emscriptenSetForcedInterface(options.getForcedInterface());
        }

        generator.genSetOutputDir(options.getOutputDirectory());
        generator.genSetCodeDir(options.getCodeInputDirectory());
        generator.genSetMultipleSchemasEnabled(options.multipleSchemasEnabled());
        generator.genSetMinRemoteVersion(options.genGetMinRemoteVersion());
        generator.emscriptenSetMainNamespaceInNamesForced(options.isMainNamespaceInNamesForced());
        generator.emscriptenSetHasProtocolVersion(options.hasProtocolVersion());
        generator.emscriptenSetMessagesListFile(options.messagesListFile());
        generator.emscriptenSetForcedPlatform(options.forcedPlatform());
        generator.genSetTopNamespace("cc_emscripten");

        auto files = commsdsl2emscripten::getFilesList(options.getFilesListFile(), options.getFilesListPrefix());
        auto otherFiles = options.getFiles();
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
