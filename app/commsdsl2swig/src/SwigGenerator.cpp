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

#include "SwigGenerator.h"

#include "Swig.h"
#include "SwigBitfieldField.h"
#include "SwigBundleField.h"
#include "SwigDataField.h"
#include "SwigEnumField.h"
#include "SwigFloatField.h"
#include "SwigIntField.h"
#include "SwigListField.h"
#include "SwigOptionalField.h"
#include "SwigRefField.h"
#include "SwigSetField.h"
#include "SwigStringField.h"
#include "SwigVariantField.h"

#include "commsdsl/version.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

const std::string& SwigGenerator::fileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2swig v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

std::string SwigGenerator::inputCodePathForFile(const std::string& name) const
{
    return getCodeDir() + '/' + name;
}

bool SwigGenerator::writeImpl()
{
    return 
        Swig::swigWrite(*this) &&
    //     TestCmake::write(*this) &&
        swigWriteExtraFilesInternal();

}

SwigGenerator::FieldPtr SwigGenerator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigIntField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigEnumField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigSetField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigFloatField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigBitfieldField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigBundleField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigStringField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigDataField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigListField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigRefField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigOptionalField>(*this, dslObj, parent);
}

SwigGenerator::FieldPtr SwigGenerator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SwigVariantField>(*this, dslObj, parent);
}

bool SwigGenerator::swigWriteExtraFilesInternal() const
{
    auto& inputDir = getCodeDir();
    if (inputDir.empty()) {
        return true;
    }

    auto& outputDir = getOutputDir();
    auto pos = inputDir.size();
    auto endIter = fs::recursive_directory_iterator();
    for (auto iter = fs::recursive_directory_iterator(inputDir); iter != endIter; ++iter) {
        if (!iter->is_regular_file()) {
            continue;
        }
        

        auto srcPath = iter->path();
        auto ext = srcPath.extension().string();

        static const std::string ReservedExt[] = {
            strings::replaceFileSuffixStr(),
            strings::extendFileSuffixStr(),
            strings::publicFileSuffixStr(),
            strings::incFileSuffixStr(),
            strings::appendFileSuffixStr(),
        };        
        auto extIter = std::find(std::begin(ReservedExt), std::end(ReservedExt), ext);
        if (extIter != std::end(ReservedExt)) {
            continue;
        }

        auto pathStr = srcPath.string();
        auto posTmp = pos;
        while (posTmp < pathStr.size()) {
            if (pathStr[posTmp] == fs::path::preferred_separator) {
                ++posTmp;
                continue;
            }
            break;
        }

        if (pathStr.size() <= posTmp) {
            continue;
        }

        std::string relPath(pathStr, posTmp);
        auto& protSchema = protocolSchema();
        auto schemaNs = util::strToName(protSchema.schemaName());
        do {
            if (protSchema.mainNamespace() == schemaNs) {
                break;
            }

            auto srcPrefix = (fs::path(strings::includeDirStr()) / schemaNs).string();
            if (!util::strStartsWith(relPath, srcPrefix)) {
                break;
            }

            auto dstPrefix = (fs::path(strings::includeDirStr()) / protSchema.mainNamespace()).string();
            relPath = dstPrefix + std::string(relPath, srcPrefix.size());
        } while (false);

        auto destPath = fs::path(outputDir) / relPath;
        logger().info("Copying " + destPath.string());

        if (!createDirectory(destPath.parent_path().string())) {
            return false;
        }

        std::error_code ec;
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            logger().error("Failed to copy with reason: " + ec.message());
            return false;
        }

        if (protSchema.mainNamespace() != schemaNs) {
            // The namespace has changed

            auto destStr = destPath.string();
            std::ifstream stream(destStr);
            if (!stream) {
                logger().error("Failed to open " + destStr + " for modification.");
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            stream.close();

            util::strReplace(content, "namespace " + schemaNs, "namespace " + protSchema.mainNamespace());
            std::ofstream outStream(destStr, std::ios_base::trunc);
            if (!outStream) {
                logger().error("Failed to modify " + destStr + ".");
                return false;
            }

            outStream << content;
            logger().info("Updated " + destStr + " to have proper main namespace.");
        }
    }
    return true;
}

} // namespace commsdsl2swig
