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

#include "SwigField.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"

#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;

#include <cassert>


namespace commsdsl2swig
{

SwigField::SwigField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

SwigField::~SwigField() = default;


SwigField::SwigFieldsList SwigField::swigTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    SwigFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* swighField = 
            const_cast<SwigField*>(
                dynamic_cast<const SwigField*>(fPtr.get()));

        assert(swighField != nullptr);
        result.push_back(swighField);
    }

    return result;    
}

bool SwigField::swigWrite() const
{
    auto* parent = m_field.getParent();
    if (parent == nullptr) {
        assert(false); // Should not happen
        return false;
    } 

    auto type = parent->elemType();
    if (type != commsdsl::gen::Elem::Type::Type_Namespace)
    {
        // Skip write for non-global fields,
        // The code generation will be driven by other means
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_field.isReferenced())) {
        // Code for not referenced does not exist
        return true;
    }

    auto& generator = m_field.generator();
    auto filePath = comms::headerPathFor(m_field, generator);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"DEF", swigClassDefInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

std::string SwigField::swigClassDefInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "};\n";

    auto& generator = SwigGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.swigClassName(m_field)}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
