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

#include <algorithm>
#include <cassert>
#include <map>
#include <type_traits>
#include <utility>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

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
    return wiresharkDissectCodeImpl(refField);
}

std::string WiresharkField::wiresharkExtractorsRegCode(const WiresharkField* refField) const
{
    if (!m_genField.genIsReferenced()) {
        return strings::genEmptyString();
    }

    return wiresharkExtractorsRegCodeImpl(refField);
}

std::string WiresharkField::wiresharkFieldObjName(const WiresharkField* refField) const
{
    return wiresharkFieldObjNameImpl(refField);
}

std::string WiresharkField::wiresharkFieldRegistration(const WiresharkField* refField) const
{
    return wiresharkFieldRegistrationImpl(refField);
}

std::string WiresharkField::wiresharkValidFuncName(const WiresharkField* refField) const
{
    return wiresharkValidFuncNameImpl(refField);
}

std::string WiresharkField::wiresharkValidFuncCode(const WiresharkField* refField) const
{
    return wiresharkValidFuncCodeInternal(refField);
}

const std::string& WiresharkField::wiresharkCustomNameCode(const WiresharkField* refField) const
{
    if (refField != nullptr) {
        return refField->wiresharkCustomNameCode();
    }

    return m_customCode.m_name;
}

bool WiresharkField::wiresharkHasCustomNameCode(const WiresharkField* refField) const
{
    if (refField != nullptr) {
        return refField->wiresharkHasCustomNameCode();
    }

    return m_customCode.m_hasName;
}

std::string WiresharkField::wiresharkTvbRangeAccess() const
{
    return wiresharkTvbRangeAccessImpl();
}

bool WiresharkField::wiresharkIsBitfieldMember() const
{
    return wiresharkParentBitfield() != nullptr;
}

const WiresharkBitfieldField* WiresharkField::wiresharkParentBitfield() const
{
    auto* parent = m_genField.genGetParent();
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

bool WiresharkField::wiresharkHasTrivialValid() const
{
    return wiresharkHasTrivialValidInternal();
}

std::size_t WiresharkField::wiresharkMinFieldLength(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    auto parseObj = genField->genParseObj();
    auto len = parseObj.parseMinLength();
    auto* bitfieldParent = wiresharkParentBitfield();
    if (bitfieldParent == nullptr) {
        return len;
    }

    return std::max(len, bitfieldParent->wiresharkMinFieldLength());
}

std::string WiresharkField::wiresharkDslCondToString(
    const WiresharkGenerator& generator,
    const WiresharkFieldsList& siblings,
    const ParseOptCond& cond)
{
    if (cond.parseKind() == ParseOptCond::ParseKind::List) {
        using ParseType = commsdsl::parse::ParseOptCondList::ParseType;
        static const std::string TypeConnectMap[] = {
            /* And */ " and\n",
            /* Or */ " or\n",
        };
        static const std::size_t TypeConnectMapSize = std::extent_v<decltype(TypeConnectMap)>;
        static_assert(TypeConnectMapSize == static_cast<unsigned>(ParseType::NumOfValues));

        commsdsl::parse::ParseOptCondList listCond(cond);
        auto typeIdx = static_cast<unsigned>(listCond.parseType());
        assert(typeIdx < TypeConnectMapSize);
        if (TypeConnectMapSize <= typeIdx) {
            return strings::genEmptyString();
        }

        GenStringsList elems;
        auto conditions = listCond.parseConditions();
        for (auto& c : conditions) {
            elems.push_back(wiresharkDslCondToString(generator, siblings, c));
        }

        static const std::string Templ =
            "(#^#COND#$#)"
            ;

        util::GenReplacementMap repl = {
            {"COND", util::genStrListToString(elems, TypeConnectMap[typeIdx], "")},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    assert(cond.parseKind() == ParseOptCond::ParseKind::Expr);
    static const std::map<std::string, std::string> OpMap = {
        {"<", " < "},
        {"<=", " <= "},
        {">", " > "},
        {">=", " >= "},
        {"=", " == "},
        {"!=", " ~= "},
        {"!", "not "},
        {strings::genEmptyString(), strings::genEmptyString()},
    };

    commsdsl::parse::ParseOptCondExpr exprCond(cond);
    auto opIter = OpMap.find(exprCond.parseOp());
    if (opIter == OpMap.end()) {
        generator.genLogger().genError("Unexpected condition operation: " + exprCond.parseOp());
        return strings::genEmptyString();
    }

    auto findSiblingFieldFunc =
        [&siblings](const std::string& name) -> const WiresharkField*
        {
            auto iter =
                std::find_if(
                    siblings.begin(), siblings.end(),
                    [&name](auto& f)
                    {
                        return f->wiresharkGenField().genParseObj().parseName() == name;
                    });

            if (iter == siblings.end()) {
                return nullptr;
            }

            return *iter;
        };

    auto leftInfo = exprCond.parseLeftInfo();
    auto& op = opIter->second;
    auto rightInfo = exprCond.parseRightInfo();

    using ParseOperandType = commsdsl::parse::ParseOptCondExpr::ParseOperandType;
    using ParseAccMode = commsdsl::parse::ParseOptCondExpr::ParseAccMode;
    if (leftInfo.m_type != ParseOperandType::Invalid) {
        assert(!op.empty());
        assert(rightInfo.m_type != ParseOperandType::Invalid);

        auto leftSepPos = leftInfo.m_access.find(".");
        std::string leftFieldName(leftInfo.m_access, 0, leftSepPos);

        const WiresharkField* leftField = nullptr;
        if (leftInfo.m_type == ParseOperandType::InterfaceRef) {
            // TODO: implement
            assert(false);
            //leftField = wiresharkFindInterfaceFieldInternal(generator, leftInfo.m_access);
        }
        else if (leftInfo.m_type == ParseOperandType::SiblingRef) {
            leftField = findSiblingFieldFunc(leftFieldName);
        }

        if (leftField == nullptr) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::genEmptyString();
        }

        std::string remLeft;
        if (leftSepPos < leftInfo.m_access.size()) {
            remLeft = leftInfo.m_access.substr(leftSepPos + 1);
        }

        assert(leftInfo.m_mode != ParseAccMode::Exists);
        assert(rightInfo.m_mode != ParseAccMode::Size);
        assert(rightInfo.m_mode != ParseAccMode::Exists);

        if (leftInfo.m_mode == ParseAccMode::Size) {
            // TODO: implement
            assert(false);
            //return commsDslCondToStringFieldSizeCompInternal(leftField, remLeft, op, rightInfo.m_access);
        }

        if (rightInfo.m_type == ParseOperandType::Value) {
            return wiresharkDslCondToStringFieldValueCompInternal(leftField, remLeft, op, rightInfo.m_access);
        }

        auto rigthSepPos = rightInfo.m_access.find(".");
        std::string rightFieldName(rightInfo.m_access, 0, rigthSepPos);

        const WiresharkField* rightField = nullptr;
        if (rightInfo.m_type == ParseOperandType::InterfaceRef) {
            // TODO: implement
            assert(false);
            //rightField = wiresharkFindInterfaceFieldInternal(generator, rightInfo.m_access);
        }
        else if (rightInfo.m_type == ParseOperandType::SiblingRef) {
            rightField = findSiblingFieldFunc(rightFieldName);
        }

        if (rightField == nullptr) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::genEmptyString();
        }

        std::string remRight;
        if (rigthSepPos < rightInfo.m_access.size()) {
            remRight = rightInfo.m_access.substr(rigthSepPos + 1);
        }

        return wiresharkDslCondToStringFieldFieldCompInternal(leftField, remLeft, op, rightField, remRight);
    }

    // Reference to bit in "set".
    if ((rightInfo.m_type != ParseOperandType::InterfaceRef) &&
        (rightInfo.m_type != ParseOperandType::SiblingRef)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    // TODO: implement
    assert(false);
    return strings::genEmptyString();

    // auto rightSepPos = rightInfo.m_access.find(".");
    // std::string rightFieldName(rightInfo.m_access, 0, rightSepPos);

    // const WiresharkField* rightField = nullptr;
    // if (rightInfo.m_type == ParseOperandType::InterfaceRef) {
    //     rightField = wiresharkFindInterfaceFieldInternal(generator, rightInfo.m_access);
    // }
    // else if (rightInfo.m_type == ParseOperandType::SiblingRef) {
    //     rightField = findSiblingFieldFunc(rightFieldName);
    // }

    // if (rightField == nullptr) {
    //     [[maybe_unused]] static constexpr bool Should_not_happen = false;
    //     assert(Should_not_happen);
    //     return strings::genEmptyString();
    // }

    // assert(rightInfo.m_mode != ParseAccMode::Size);

    // std::string remRight;
    // if (rightSepPos < rightInfo.m_access.size()) {
    //     remRight = rightInfo.m_access.substr(rightSepPos + 1);
    // }

    // if (rightInfo.m_mode == ParseAccMode::Exists) {
    //     return commsDslCondToStringFieldExistsCompInternal(rightField, remRight, op);
    // }

    // auto rightAccName = comms::genAccessName(rightField->commsGenField().genParseObj().parseName());
    // auto checks = rightField->commsCompOptChecks(remRight, commsGetFieldAccessPrefixInternal(*rightField) + rightAccName + "()");
    // checks.push_back(op + commsGetFieldAccessPrefixInternal(*rightField) + rightAccName + "()" + rightField->commsValueAccessStr(remRight));

    // return util::genStrListToString(checks, " &&\n", "");
}

std::string WiresharkField::wiresharkValueAccessStr(const std::string& accStr, const WiresharkField* refField) const
{
    return wiresharkValueAccessStrImpl(accStr, refField);
}

std::string WiresharkField::wiresharkCompPrepValueStr(const std::string& value) const
{
    return wiresharkCompPrepValueStrImpl(value);
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

std::string WiresharkField::wiresharkValidFuncNameImpl(const WiresharkField* refField) const
{
    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*genField, strings::genValidSuffixStr());
}

std::string WiresharkField::wiresharkDissectCodeImpl(const WiresharkField* refField) const
{
    if (comms::genIsGlobalField(m_genField) && (!m_genField.genIsReferenced())) {
        m_genField.genGenerator().genLogger().genDebug("Field " + m_genField.genParseObj().parseInnerRef() + " is not really referenced, skipping dissect code generation");
        return strings::genEmptyString();
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto parseObj = m_genField.genParseObj();
    if (!wiresharkGenerator.genDoesElementExist(parseObj.parseSinceVersion(), parseObj.parseDeprecatedSince(), parseObj.parseIsDeprecatedRemoved())) {
        return strings::genEmptyString();
    }

    const auto* genField = &m_genField;
    if (refField != nullptr) {
        genField = &(refField->wiresharkGenField());
    }

    m_genField.genGenerator().genLogger().genDebug("Generating dissect code for field " + m_genField.genParseObj().parseInnerRef());

    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#NAME_VAR#$#\n"
        "#^#REG#$#\n"
        "#^#VALID_FUNC#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(#^#SIG#$#)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*genField);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"REG", wiresharkFieldRegistration(refField)},
        {"NAME", wiresharkDissectName(refField)},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
        {"NAME_VAR", wiresharkNameDefInternal(refField)},
        {"SIG", wiresharkDissectSignature()},
        {"VALID_FUNC", wiresharkValidFuncCodeInternal(refField)},
    };

    if (refField == nullptr) {
        repl["MEMBERS"] = wiresharkMembersDissectCodeImpl();
    }

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal(refField);
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#REG_FUNC#$#(\"#^#REF_NAME#$#\", #^#FIELD#$#)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"REG_FUNC", Wireshark::wiresharkCreateExtractorFuncName(wiresharkGenerator)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"FIELD", wiresharkFieldObjName(refField)},
    };

    return util::genProcessTemplate(Templ, repl);
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

std::string WiresharkField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.bytes(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.SPACE, #^#DESC#$#))\n"
    ;

    auto obj = m_genField.genParseObj();
    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(m_genField.genGenerator()))},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkMembersDissectCodeImpl() const
{
    return std::string();
}

std::string WiresharkField::wiresharkTvbRangeAccessImpl() const
{
    [[maybe_unused]] static constexpr bool Should_not_be_called = false;
    assert(Should_not_be_called);
    return strings::genEmptyString();
}

std::string WiresharkField::wiresharkDissectLengthCheckImpl(const WiresharkField* refField) const
{
    auto parseObj = m_genField.genParseObj();
    if (parseObj.parseMinLength() == 0) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "if offset_limit < (offset + #^#LEN#$#) then\n"
        "    return #^#ERROR#$#, offset\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::NotEnoughData)},
        {"LEN", std::to_string(wiresharkMinFieldLength(refField))},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkDissectBodyImpl(const WiresharkField* refField) const
{
    // TODO
    static_cast<void>(refField);
    return "-- TODO: not implemented";
    //return strings::genEmptyString();
}

std::string WiresharkField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    // TODO: assert for not overriden
    return
        "-- TODO: BUG: not overriden\n"
        "return true";
}

std::string WiresharkField::wiresharkValueAccessStrImpl(
    [[maybe_unused]] const std::string& accStr,
    const WiresharkField* refField) const
{
    assert(accStr.empty());
    static const std::string Templ =
        "#^#FUNC#$#(#^#FIELD#$#)"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"FIELD", wiresharkFieldObjName(refField)},
        {"FUNC", Wireshark::wiresharkFieldValueFuncName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkCompPrepValueStrImpl(const std::string& value) const
{
    return value;
}

bool WiresharkField::wiresharkHasTrivialValidImpl() const
{
    return true;
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

    auto* parentBitfield = refField->wiresharkParentBitfield();
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

    auto* parentBitfield = wiresharkParentBitfield();
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

    auto* parentBitfield = refField->wiresharkParentBitfield();
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

    auto* parentBitfield = wiresharkParentBitfield();
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

std::string WiresharkField::wiresharkDissectSignature()
{
    static const std::string Templ =
        "#^#TVB#$#, #^#TREE#$#, #^#OFFSET#$#, #^#LIMIT#$#, #^#FIELD#$#"
        ;

    util::GenReplacementMap repl = {
        {"TVB", wiresharkTvbStr()},
        {"TREE", wiresharkTreeStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"FIELD", wiresharkFieldStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkHexString(std::uintmax_t value, unsigned hexWidth)
{
    auto str = util::genNumToString(value, hexWidth);
    auto pos = str.find_first_of("UL");
    str.resize(std::min(str.size(), pos));
    return str;
}

std::string WiresharkField::wiresharkEmptyBufferCheckCode() const
{
    static const std::string Templ =
        "if offset == offset_limit then"
        "    return #^#ERROR#$#, offset\n"
        "end"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::NotEnoughData)},
    };

    return util::genProcessTemplate(Templ, repl);
}

const std::string& WiresharkField::wiresharkRangeStr()
{
    static const std::string Str("range");
    return Str;
}

const std::string& WiresharkField::wiresharkFieldSubtreeStr()
{
    static const std::string Str("field_subtree");
    return Str;
}

const std::string& WiresharkField::wiresharkValStr()
{
    static const std::string Str("val");
    return Str;
}

const std::string& WiresharkField::wiresharkFieldStr()
{
    static const std::string Str("field");
    return Str;
}

const std::string& WiresharkField::wiresharkNextOffsetStr()
{
    static const std::string Str("next_offset");
    return Str;
}

const std::string& WiresharkField::wiresharkOffsetStr()
{
    static const std::string Str("offset");
    return Str;
}

const std::string& WiresharkField::wiresharkOffsetLimitStr()
{
    static const std::string Str("offset_limit");
    return Str;
}

const std::string& WiresharkField::wiresharkResultStr()
{
    static const std::string Str("result");
    return Str;
}

const std::string& WiresharkField::wiresharkTvbStr()
{
    static const std::string Str("tvb");
    return Str;
}

const std::string& WiresharkField::wiresharkTreeStr()
{
    static const std::string Str("tree");
    return Str;
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
        {"COMMENT", wiresharkCustomNameCode(refField)},
        {"VAR_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"NAME", wiresharkFieldDisplayNameStr(refField)},
    };

    if (wiresharkHasCustomNameCode(refField)) {
        repl["NAME"] = std::move(repl["COMMENT"]);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkDissectBodyInternal(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#FIELD_STR#$# = #^#FIELD_STR#$# or #^#FIELD#$#\n"
        "#^#LENGTH#$#\n"
        "local result = #^#ERROR#$#\n"
        "local next_offset = offset\n"
        "#^#REST#$#\n"
        "#^#VALID#$#\n"
        "return result, next_offset"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"FIELD", wiresharkFieldObjName(refField)},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::InvalidMsgData)},
        {"REST", wiresharkDissectBodyImpl(refField)},
        {"LENGTH", wiresharkDissectLengthCheckImpl(refField)},
        {"VALID", wiresharkDissectValidCheckInternal(refField)},
        {"FIELD_STR", wiresharkFieldStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkCustomReadCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(m_genField);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    //m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace read functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &hasRealCode);
}

std::string WiresharkField::wiresharkCustomValidCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genField, strings::genValidSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    //m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace valid functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &hasRealCode);
}

std::string WiresharkField::wiresharkCustomNameCodeInternal(bool& hasRealCode) const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genField, strings::genNameSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    //m_genField.genGenerator().genLogger().genDebug("Looking for \"" + replaceFileName + "\" to replace name functionality");
    return wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace name value", &hasRealCode);
}

std::string WiresharkField::wiresharkDissectValidCheckInternal(const WiresharkField* refField) const
{
    if (wiresharkHasTrivialValidInternal()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local valid, print_warn = #^#VALID_FUNC#$#(#^#FIELD#$#)\n"
        "if not valid then\n"
        "    #^#CODE#$#\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"VALID_FUNC", wiresharkValidFuncName(refField)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"FIELD", wiresharkFieldStr()}
    };

    if (m_genField.genParseObj().parseIsFailOnInvalid()) {
        static const std::string FailTempl =
            "#^#SUBTREE#$#:set_hidden(true)\n"
            "return result, offset"
            ;

        repl["CODE"] = util::genProcessTemplate(FailTempl, repl);
    }
    else {
        static const std::string FailTempl =
            "if print_warn then\n"
            "    #^#SUBTREE#$#:add_expert_info(PI_PROTOCOL, PI_WARN, \"Invalid field value\")\n"
            "end"
            ;
        repl["CODE"] = util::genProcessTemplate(FailTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkField::wiresharkValidFuncCodeInternal(const WiresharkField* refField) const
{
    if (wiresharkHasTrivialValidInternal()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local function #^#NAME#$##^#SUFFIX#$#(#^#FIELD#$#)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genField.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genField, strings::genValidSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"NAME", wiresharkValidFuncName(refField)},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
        {"FIELD", wiresharkFieldStr()},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkValidFuncBodyImpl(refField);
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkField::wiresharkHasTrivialValidInternal() const
{
    if (m_customCode.m_hasValid) {
        return false;
    }

    return wiresharkHasTrivialValidImpl();
}

std::string WiresharkField::wiresharkDslCondToStringFieldValueCompInternal(
    const WiresharkField* field,
    const std::string& accStr,
    const std::string& op,
    const std::string& value)
{
    auto valueStr = field->wiresharkCompPrepValueStr(value);
    return '(' + field->wiresharkValueAccessStr(accStr) + op + valueStr + ')';
}

std::string WiresharkField::wiresharkDslCondToStringFieldFieldCompInternal(
    const WiresharkField* leftField,
    const std::string& leftAccStr,
    const std::string& op,
    const WiresharkField* rightField,
    const std::string& rightAccStr)
{
    // TODO:
    static_cast<void>(leftField);
    static_cast<void>(leftAccStr);
    static_cast<void>(op);
    static_cast<void>(rightField);
    static_cast<void>(rightAccStr);
    assert(false);
    return strings::genEmptyString();
    // auto leftAccName = comms::genAccessName(leftField->commsGenField().genParseObj().parseName());
    // auto leftPrefix = commsGetFieldAccessPrefixInternal(*leftField) + leftAccName + "()";
    // auto rightAccName = comms::genAccessName(rightField->commsGenField().genParseObj().parseName());
    // auto rightPrefix = commsGetFieldAccessPrefixInternal(*rightField) + rightAccName + "()";

    // auto optConds = leftField->commsCompOptChecks(leftAccStr, leftPrefix);
    // rightField->commsCompOptChecks(rightAccStr, optConds, rightPrefix);
    // auto valueStr = rightPrefix + rightField->commsValueAccessStr(rightAccStr);
    // auto typeCast = leftField->commsCompValueCastType(leftAccStr);
    // if (!typeCast.empty()) {
    //     auto accName = comms::genAccessName(leftField->commsGenField().genParseObj().parseName());
    //     valueStr = "static_cast<typename Field_" + accName + "::" + typeCast + ">(" + valueStr + ")";
    // }

    // auto expr = leftPrefix + leftField->commsValueAccessStr(leftAccStr) + ' ' + op + ' ' + valueStr;

    // if (optConds.empty()) {
    //     return expr;
    // }

    // static const std::string Templ =
    //     "#^#COND#$# &&\n"
    //     "(#^#EXPR#$#)";

    // util::GenReplacementMap repl = {
    //     {"COND", util::genStrListToString(optConds, " &&\n", "")},
    //     {"EXPR", std::move(expr)},
    // };

    // return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
