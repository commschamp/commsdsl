//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CField.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

CField::CField(GenField& field) : 
    m_genField(field)
{
}

CField::~CField() = default;

CField::CFieldsList CField::cTransformFieldsList(const GenFieldsList& fields)
{
    CFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* cField = 
            const_cast<CField*>(
                dynamic_cast<const CField*>(fPtr.get()));

        assert(cField != nullptr);
        result.push_back(cField);
    }

    return result;
}

bool CField::cWrite() const
{
    if (!comms::genIsGlobalField(m_genField)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        m_genField.genGenerator().genLogger().genDebug(
            "Skipping file generation for non-referenced field \"" + m_genField.genParseObj().parseExternalRef() + "\".");
        return true;
    }
    
    return 
        cWriteHeaderInternal() &&
        cWriteSrcInternal();    
}

bool CField::cWriteHeaderInternal() const
{
    auto& generator = CGenerator::cCast(m_genField.genGenerator());
    auto filePath = generator.cAbsHeaderFor(m_genField);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#DEF#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        // {"INCLUDES", cHeaderIncludesInternal()},
        // {"DEF", cHeaderClass()},
        {"APPEND", util::genReadFileContents(generator.cInputAbsHeaderFor(m_genField) + strings::genAppendFileSuffixStr())}
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

bool CField::cWriteSrcInternal() const
{
    auto& generator = CGenerator::cCast(m_genField.genGenerator());
    auto filePath = generator.cAbsSourceFor(m_genField);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CODE#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        // {"INCLUDES", cSourceIncludesInternal()},
        // {"CODE", cSourceCode()},
        {"APPEND", util::genReadFileContents(generator.cInputAbsSourceFor(m_genField) + strings::genAppendFileSuffixStr())}
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

} // namespace commsdsl2c
