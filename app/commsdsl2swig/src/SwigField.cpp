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
#include "SwigOptionalField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
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

bool SwigField::swigIsVersionOptional() const
{
    return comms::isVersionOptionaField(m_field, m_field.generator());
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
        "#^#MEMBERS#$#\n"
        "#^#DEF#$#\n"
        "#^#OPTIONAL#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"MEMBERS", swigMembersDefImpl()},
        {"DEF", swigClassDefInternal()},
        {"OPTIONAL", swigOptionalDefInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

std::string SwigField::swigMembersDefImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigValueTypeImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigValueAccImpl() const
{
    return std::string(
        "const ValueType& getValue() const;\n"
        "void setValue(const ValueType&);\n"
    );
}

std::string SwigField::swigExtraPublicFuncsImpl() const
{
    return strings::emptyString();
}

const std::string& SwigField::swigCommonPublicFuncs()
{
    static const std::string Templ = 
        "static const char* name();\n"
        "comms_ErrorStatus read(const unsigned char*& iter, unsigned long len);\n"
        "comms_ErrorStatus write(unsigned char*& iter, unsigned long len) const;\n"
        "bool refresh();\n"
        "unsigned long long length();\n"
        "bool valid() const;\n"
    ;
    return Templ;
}

std::string SwigField::swigClassDefInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "public:\n"
        "    #^#VALUE_TYPE#$#\n"
        "    #^#VALUE_ACC#$#\n"
        "    #^#COMMON_FUNCS#$#\n"
        "    #^#EXTRA#$#\n"
        "};\n";

    auto& generator = SwigGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.swigClassName(m_field)},
        {"VALUE_TYPE", swigValueTypeImpl()},
        {"VALUE_ACC", swigValueAccImpl()},
        {"COMMON_FUNCS", swigCommonPublicFuncs()},
        {"EXTRA", swigExtraPublicFuncsImpl()},
    };

    if (swigIsVersionOptional()) {
        repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    }

    return util::processTemplate(Templ, repl);
}

std::string SwigField::swigOptionalDefInternal() const
{
    if (!swigIsVersionOptional()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "   #^#BODY#$#"
        "};\n";     

    auto& generator = SwigGenerator::cast(m_field.generator());
    auto className = generator.swigClassName(m_field);
    util::ReplacementMap repl = {
        {"CLASS_NAME", className},
        {"BODY", SwigOptionalField::swigDefBodyCode(className + strings::versionOptionalFieldSuffixStr())},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
