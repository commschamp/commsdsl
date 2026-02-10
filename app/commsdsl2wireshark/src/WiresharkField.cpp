//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkField.h"

#include "Wireshark.h"
#include "WiresharkBitfieldField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/parse/ParseField.h"

#include <cassert>
#include <utility>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

const WiresharkBitfieldField* wiresharkParentBitfieldInternal(const WiresharkField& field)
{
    auto* parent = field.wiresharkGenField().genGetParent();
    assert(parent != nullptr);
    if (parent->genElemType() != commsdsl::gen::GenElem::GenType_Field) {
        return nullptr;
    }

    auto* asField = static_cast<const commsdsl::gen::GenField*>(parent);
    if (asField->genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Bitfield) {
        return nullptr;
    }

    return static_cast<const WiresharkBitfieldField*>(asField);
}

} // namespace

WiresharkField::WiresharkField(GenField& field) :
    m_genField(field)
{
}

WiresharkField::~WiresharkField() = default;

const WiresharkField* WiresharkField::wiresharkCast(const GenField* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    return dynamic_cast<const WiresharkField*>(field);
}

WiresharkField* WiresharkField::wiresharkCast(GenField* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    return dynamic_cast<WiresharkField*>(field);
}

WiresharkField::WiresharkFieldsList WiresharkField::wiresharkTransformFieldsList(const GenField::GenFieldsList& fields)
{
    WiresharkFieldsList result;
    result.reserve(fields.size());
    for (auto& f : fields) {
        auto* wiresharkField = const_cast<WiresharkField*>(wiresharkCast(f.get()));
        assert(wiresharkField != nullptr);
        result.push_back(wiresharkField);
    }

    return result;
}

std::string WiresharkField::wiresharkDissectName(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    if (!genField->genIsReferenced()) {
        return strings::genEmptyString();
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(*genField);
}

std::string WiresharkField::wiresharkDissectCode(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    if (!genField->genIsReferenced()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#REG#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offsetLimit)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(*genField);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"MEMBERS", wiresharkMembersDissectCodeImpl(refField)},
        {"REG", wiresharkFieldRegistration(refField)},
        {"NAME", wiresharkDissectName(refField)},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkFieldObjName(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    auto scope = comms::genScopeFor(*genField, wiresharkGenerator, false);
    return Wireshark::wiresharkProtocolObjName(wiresharkGenerator) + '_' + util::genStrReplace(scope, "::", "_");
}

std::string WiresharkField::wiresharkFieldRegistration(const WiresharkField* refField) const
{
    return wiresharkFieldRegistrationImpl(refField);
}

std::string WiresharkField::wiresharkFieldRegistrationImpl([[maybe_unused]] const WiresharkField* refField) const
{
    return std::string();
}

std::string WiresharkField::wiresharkMembersDissectCodeImpl([[maybe_unused]] const WiresharkField* refField) const
{
    return std::string();
}

std::string WiresharkField::wiresharkFieldRefName(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    auto scope = comms::genScopeFor(*genField, wiresharkGenerator, false);
    return Wireshark::wiresharkProtocolObjName(wiresharkGenerator) + '.' + util::genStrReplace(scope, "::", ".");
}

std::string WiresharkField::wiresharkForcedIntegralFieldMask(const WiresharkField* refField) const
{
    if (refField == nullptr) {
        refField = this;
    }

    auto* parentBitfield = wiresharkParentBitfieldInternal(*refField);
    if (parentBitfield == nullptr) {
        return strings::genNilStr();
    }

    return parentBitfield->wiresharkForcedBitfieldMask(*refField);
}

std::string WiresharkField::wiresharkForcedIntegralFieldType(const WiresharkField* refField) const
{
    if (refField == nullptr) {
        refField = this;
    }

    auto* parentBitfield = wiresharkParentBitfieldInternal(*refField);
    if (parentBitfield == nullptr) {
        return strings::genEmptyString();
    }

    return parentBitfield->wiresharkIntegralType();
}

unsigned WiresharkField::wiresharkForcedMaskShift(const WiresharkField* refField) const
{
    if (refField == nullptr) {
        refField = this;
    }

    auto* parentBitfield = wiresharkParentBitfieldInternal(*refField);
    if (parentBitfield == nullptr) {
        return 0U;
    }

    return parentBitfield->wiresharkMaskShiftFor(*refField);
}

unsigned WiresharkField::wiresharkForcedBitLength(const WiresharkField* refField) const
{
    if (refField == nullptr) {
        refField = this;
    }

    auto* parentBitfield = wiresharkParentBitfieldInternal(*refField);
    if (parentBitfield == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(parentBitfield->genParseObj().parseMaxLength() * 8U);
}

std::string WiresharkField::wiresharkDissectBodyInternal() const
{
    // TODO:
    return std::string();
}

std::string WiresharkField::wiresharkFieldDescriptionStr(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    auto obj = genField->genParseObj();
    auto desc = genField->genParseObj().parseDescription();

    if ((desc.empty()) && (genField != &m_genField)) {
        desc = m_genField.genParseObj().parseDescription();
    }

    if (desc.empty()) {
        return strings::genNilStr();
    }

    return "\"" + desc + "\"";
}

std::string WiresharkField::wiresharkFieldDisplayNameStr(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    auto obj = genField->genParseObj();
    auto dispName = obj.parseDisplayName();
    if (!dispName.empty()) {
        return dispName;
    }

    if (genField != &m_genField) {
        dispName = m_genField.genParseObj().parseDisplayName();
    }

    if (!dispName.empty()) {
        return dispName;
    }

    return util::genDisplayName(dispName, obj.parseName());
}

} // namespace commsdsl2wireshark
