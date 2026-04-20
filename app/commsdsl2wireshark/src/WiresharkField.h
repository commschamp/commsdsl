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

#pragma once

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/parse/ParseOptCond.h"

#include <string>
#include <vector>
#include <utility>

namespace commsdsl2wireshark
{

class WiresharkBitfieldField;
class WiresharkGenerator;
class WiresharkInterface;
class WiresharkField
{
public:
    using ParseOptCond = commsdsl::parse::ParseOptCond;
    using GenField = commsdsl::gen::GenField;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using WiresharkFieldsList = std::vector<WiresharkField*>;

    explicit WiresharkField(commsdsl::gen::GenField& field);
    virtual ~WiresharkField();

    static const WiresharkField* wiresharkCast(const GenField* field);
    static WiresharkField* wiresharkCast(GenField* field);

    static WiresharkFieldsList wiresharkTransformFieldsList(const GenField::GenFieldsList& fields);

    GenField& wiresharkGenField()
    {
        return m_genField;
    }

    const GenField& wiresharkGenField() const
    {
        return m_genField;
    }

    bool wiresharkPrepare();
    std::string wiresharkDissectName(const WiresharkField* refField = nullptr) const;
    std::string wiresharkDissectCode(const WiresharkField* refField = nullptr) const;
    std::string wiresharkExtractorsRegCode(const WiresharkField* refField = nullptr) const;
    std::string wiresharkFieldObjName(const WiresharkField* refField = nullptr) const;
    std::string wiresharkFieldRegistration(const WiresharkField* refField = nullptr) const;
    std::string wiresharkValidFuncName(const WiresharkField* refField = nullptr) const;
    std::string wiresharkValidFuncBody(const WiresharkField* refField = nullptr) const;
    std::string wiresharkValueFuncName(const WiresharkField* refField = nullptr) const;
    std::string wiresharkValueFuncBody(const WiresharkField* refField = nullptr) const;

    const std::string& wiresharkCustomNameCode(const WiresharkField* refField = nullptr) const;
    bool wiresharkHasCustomNameCode(const WiresharkField* refField = nullptr) const;

    std::string wiresharkTvbRangeAccess() const;
    bool wiresharkIsBitfieldMember() const;
    const WiresharkBitfieldField* wiresharkParentBitfield() const;

    bool wiresharkHasTrivialValid() const;
    std::size_t wiresharkMinFieldLength(const WiresharkField* refField = nullptr) const;

    static std::string wiresharkDslCondToString(
        const WiresharkGenerator& generator,
        const WiresharkFieldsList& fields,
        const WiresharkInterface& interface,
        const ParseOptCond& cond);

    std::string wiresharkValueAccessStr(const std::string& accStr, const WiresharkField* refField = nullptr) const;
    std::string wiresharkSizeAccessStr(const std::string& accStr, const WiresharkField* refField = nullptr) const;
    std::string wiresharkCompPrepValueStr(const std::string& value) const;
    std::string wiresharkExistsCheckStr(const std::string& accStr) const;
    std::string wiresharkVersionCheckStr(const WiresharkInterface& interface) const;

    bool wiresharkNeedsOptionalModeDefinition() const;
    const WiresharkFieldsList& wiresharkSiblings() const;
    const WiresharkFieldsList& wiresharkMemberFields() const;
    std::string wiresharkDefaultAssignments(const WiresharkField* refField = nullptr) const;

    static std::pair<std::string, std::string> wiresharkSplitAccStr(const std::string& accStr);
    static std::pair<const WiresharkField*, std::string> wiresharkSplitMemberAccStr(const std::string& accStr, const WiresharkFieldsList& fields);

protected:
    virtual std::string wiresharkDissectNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValidFuncNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValueFuncNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkDissectCodeImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValidCheckCodeImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkFieldObjNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkMembersDissectCodeImpl() const;
    virtual std::string wiresharkTvbRangeAccessImpl() const;
    virtual std::string wiresharkDissectLengthCheckImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkDissectBodyImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValidFuncBodyImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValueFuncBodyImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const;
    virtual std::string wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const;
    virtual std::string wiresharkCompPrepValueStrImpl(const std::string& value) const;
    virtual std::string wiresharkExistsCheckStrImpl(const std::string& accStr) const;
    virtual std::string wiresharkDefaultAssignmentsImpl(const WiresharkField* refField) const;
    virtual bool wiresharkHasTrivialValidImpl() const;
    virtual const WiresharkFieldsList& wiresharkMemberFieldsImpl() const;

    std::string wiresharkFieldRefName(const WiresharkField* refField) const;
    std::string wiresharkForcedIntegralFieldMask(const WiresharkField* refField) const;
    std::string wiresharkForcedIntegralFieldType(const WiresharkField* refField) const;
    unsigned wiresharkForcedMaskShift(const WiresharkField* refField) const;
    unsigned wiresharkForcedBitLength(const WiresharkField* refField) const;
    std::string wiresharkFieldDescriptionStr(const WiresharkField* refField) const;
    std::string wiresharkFieldDisplayNameStr(const WiresharkField* refField) const;
    std::string wiresharkFieldNameVarNameStr(const WiresharkField* refField) const;
    bool wiresharkHasOverrideCode() const;
    static std::string wiresharkDissectSignature();
    static std::string wiresharkHexString(std::uintmax_t val, unsigned hexWidth);
    std::string wiresharkEmptyBufferCheckCode() const;
    std::string wiresharkProcessIntegralValue(const std::string& val) const;
    std::string wiresharkProcessFloatValue(const std::string& val) const;
    const WiresharkInterface& wiresharkInterface() const;

    static const std::string& wiresharkRangeStr();
    static const std::string& wiresharkFieldSubtreeStr();
    static const std::string& wiresharkValStr();
    static const std::string& wiresharkFieldStr();
    static const std::string& wiresharkNextOffsetStr();
    static const std::string& wiresharkOffsetStr();
    static const std::string& wiresharkOffsetLimitStr();
    static const std::string& wiresharkResultStr();
    static const std::string& wiresharkTvbStr();
    static const std::string& wiresharkTreeStr();

private:
    using WiresharkCustomCodeFunc = std::string (WiresharkField::*)(bool& hasCode) const;

    struct WiresharkCustomCode
    {
        std::string m_read;
        std::string m_valid;
        std::string m_name;
        std::string m_value;

        bool m_hasRead = false;
        bool m_hasValid = false;
        bool m_hasName = false;
        bool m_hasValue = false;
    };

    bool wiresharkCopyCodeFromInternal();
    bool wiresharkPrepareOverrideInternal(
        commsdsl::parse::ParseOverrideType type,
        const std::string& name,
        WiresharkCustomCodeFunc codeFunc,
        std::string& code,
        bool& hasCode);

    std::string wiresharkNameDefInternal(const WiresharkField* refField) const;
    std::string wiresharkDissectBodyInternal(const WiresharkField* refField) const;
    std::string wiresharkCustomReadCodeInternal(bool& hasRealCode) const;
    std::string wiresharkCustomValidCodeInternal(bool& hasRealCode) const;
    std::string wiresharkCustomNameCodeInternal(bool& hasRealCode) const;
    std::string wiresharkCustomValueCodeInternal(bool& hasRealCode) const;
    std::string wiresharkDissectValidCheckInternal(const WiresharkField* refField) const;
    std::string wiresharkValidFuncCodeInternal(const WiresharkField* refField) const;
    std::string wiresharkValueFuncCodeInternal(const WiresharkField* refField) const;
    bool wiresharkHasTrivialValidInternal() const;
    std::string wiresharkProcessNumericValueInternal(const std::string& val) const;
    std::string wiresharkDisscetVersionCheckInternal(const WiresharkField* refField) const;

    static std::string wiresharkDslCondToStringFieldValueCompInternal(
        const WiresharkField* leftField,
        const std::string& accStr,
        const std::string& op,
        const std::string& value,
        const WiresharkInterface& interface);

    static std::string wiresharkDslCondToStringFieldFieldCompInternal(
        const WiresharkField* leftField,
        const std::string& leftAccStr,
        const std::string& op,
        const WiresharkField* rightField,
        const std::string& rightAccStr,
        const WiresharkInterface& interface);

    static std::string wiresharkDslCondToStringFieldSizeCompInternal(
        const WiresharkField* field,
        const std::string& accStr,
        const std::string& op,
        const std::string& value,
        const WiresharkInterface& interface);

    static std::string wiresharkDslCondToStringFieldExistsCompInternal(
        const WiresharkField* field,
        const std::string& accStr,
        const std::string& op,
        const WiresharkInterface& interface);

    GenField& m_genField;
    WiresharkCustomCode m_customCode;
    mutable commsdsl::gen::util::GenStringsList m_dissected;
};

} // namespace commsdsl2wireshark
