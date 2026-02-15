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

// bool wiresharkHasOrigCodeInternal(commsdsl::parse::ParseOverrideType value)
// {
//     return (value != commsdsl::parse::ParseOverrideType_Replace);
// }

bool wiresharkIsOverrideCodeAllowedInternal(commsdsl::parse::ParseOverrideType value)
{
    return (value != commsdsl::parse::ParseOverrideType_None);
}

bool wiresharkIsOverrideCodeRequiredInternal(commsdsl::parse::ParseOverrideType value)
{
    return
        (value == commsdsl::parse::ParseOverrideType_Replace) ||
        (value == commsdsl::parse::ParseOverrideType_Extend);
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

bool WiresharkField::wiresharkPrepare()
{
    if (!wiresharkCopyCodeFromInternal()) {
        return false;
    }

    auto& obj = m_genField.genParseObj();
    bool overrides =
        wiresharkPrepareOverrideInternal(obj.parseReadOverride(), "read", &WiresharkField::wiresharkCustomReadCodeInternal, m_customCode.m_read, m_customCode.m_hasRead) &&
        wiresharkPrepareOverrideInternal(obj.parseValidOverride(), "valid", &WiresharkField::wiresharkCustomValidCodeInternal, m_customCode.m_valid, m_customCode.m_hasValid) &&
        wiresharkPrepareOverrideInternal(obj.parseNameOverride(), "name", &WiresharkField::wiresharkCustomNameCodeInternal, m_customCode.m_name, m_customCode.m_hasName)
        ;

    if (!overrides) {
        return false;
    }

    return true;
}

std::string WiresharkField::wiresharkDissectName(const WiresharkField* refField) const
{
    return wiresharkDissectNameImpl(refField);
}

std::string WiresharkField::wiresharkDissectCode(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    if (!genField->genIsReferenced()) {
        genField->genGenerator().genLogger().genDebug("Field " + genField->genName() + " is not really referenced, skipping dissect code generation");
        return strings::genEmptyString();
    }

    // TODO: valid function if needed
    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#NAME_VAR#$#\n"
        "#^#REG#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offsetLimit)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*genField);
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
        {"NAME_VAR", wiresharkNameDefInternal(refField)},
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
    return wiresharkFieldObjNameImpl(refField);
}

std::string WiresharkField::wiresharkFieldRegistration(const WiresharkField* refField) const
{
    return wiresharkFieldRegistrationImpl(refField);
}

std::string WiresharkField::wiresharkDissectNameImpl(const WiresharkField* refField) const
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

std::string WiresharkField::wiresharkFieldObjNameImpl(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genField->genGenerator());
    auto scope = comms::genScopeFor(*genField, wiresharkGenerator, false);
    return Wireshark::wiresharkProtocolObjName(wiresharkGenerator) + '_' + util::genStrReplace(scope, "::", "_");
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

bool WiresharkField::wiresharkIsBitfieldMember() const
{
    return wiresharkParentBitfieldInternal(*this) != nullptr;
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

std::string WiresharkField::wiresharkFieldNameVarNameStr(const WiresharkField* refField) const
{
    return wiresharkFieldObjName(refField) + strings::genNameSuffixStr();
}

bool WiresharkField::wiresharkHasOverrideCode() const
{
    return
        m_customCode.m_hasRead ||
        m_customCode.m_hasValid ||
        m_customCode.m_hasName;
}

bool WiresharkField::wiresharkCopyCodeFromInternal()
{
    auto obj = m_genField.genParseObj();
    auto& copyFrom = obj.parseCopyCodeFrom();
    if (copyFrom.empty()) {
        return true;
    }

    auto& gen = m_genField.genGenerator();
    auto* origField = gen.genFindField(copyFrom);
    if (origField == nullptr) {
        gen.genLogger().genError(
            "Failed to find referenced field \"" + copyFrom + "\" for copying overriding code.");
        assert(false); // Should not happen
        return false;
    }

    auto* wiresharkField = wiresharkCast(origField);
    assert(wiresharkField != nullptr);

    m_customCode = wiresharkField->m_customCode;
    return true;
}

bool WiresharkField::wiresharkPrepareOverrideInternal(
    commsdsl::parse::ParseOverrideType type,
    const std::string& name,
    WiresharkCustomCodeFunc codeFunc,
    std::string& code,
    bool& hasCode)
{
    if (wiresharkIsOverrideCodeRequiredInternal(type) && (!comms::genIsGlobalField(m_genField))) {
        m_genField.genGenerator().genLogger().genError(
            "Overriding \"" + name + "\" operation is not supported for non global fields, detected on \"" +
            comms::genScopeFor(m_genField, m_genField.genGenerator()) + "\".");
        return false;
    }

    do {
        if (!wiresharkIsOverrideCodeAllowedInternal(type)) {
            code.clear();
            hasCode = false;
            break;
        }

        bool hasCodeTmp = false;
        auto customCode = (this->*codeFunc)(hasCodeTmp);
        if ((hasCodeTmp) || (!hasCode)) {
            code = std::move(customCode);
            hasCode = hasCode || hasCodeTmp;
            break;
        }

    } while (false);

    if ((!hasCode) && wiresharkIsOverrideCodeRequiredInternal(type)) {
        m_genField.genGenerator().genLogger().genError(
            "Overriding \"" + name + "\" operation is not provided in injected code for field \"" +
            m_genField.genParseObj().parseExternalRef() + "\".");
        return false;
    }

    return true;
}

std::string WiresharkField::wiresharkNameDefInternal(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#COMMENT#$#"
        "local #^#VAR_NAME#$# = \"#^#NAME#$#\"\n"
    ;

    util::GenReplacementMap repl = {
        {"VAR_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"NAME", wiresharkFieldDisplayNameStr(refField)},
    };

    if (m_customCode.m_hasName) {
        repl["NAME"] = m_customCode.m_name;
    }
    else {
        repl["COMMENT"] = m_customCode.m_name;
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkDissectBodyInternal() const
{
    // TODO:
    return std::string();
}

std::string WiresharkField::wiresharkCustomReadCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(m_genField);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace read functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &hasRealCode);
}

std::string WiresharkField::wiresharkCustomValidCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genField, strings::genValidSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace valid functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &hasRealCode);
}

std::string WiresharkField::wiresharkCustomNameCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genField, strings::genNameSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace name functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace name definition", &hasRealCode);
}

} // namespace commsdsl2wireshark
