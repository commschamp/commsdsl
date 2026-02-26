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

#include <string>
#include <vector>

namespace commsdsl2wireshark
{

class WiresharkField
{
public:
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
    std::string wiresharkFieldObjName(const WiresharkField* refField) const;
    std::string wiresharkFieldRegistration(const WiresharkField* refField = nullptr) const;

    const std::string& wiresharkCustomNameCode(const WiresharkField* refField = nullptr) const;
    bool wiresharkHasCustomNameCode(const WiresharkField* refField = nullptr) const;

    std::string wiresharkTvbRangeAccess() const;

protected:
    virtual std::string wiresharkDissectNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkDissectCodeImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkFieldObjNameImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const;
    virtual std::string wiresharkMembersDissectCodeImpl() const;
    virtual std::string wiresharkTvbRangeAccessImpl() const;

    std::string wiresharkFieldRefName(const WiresharkField* refField) const;
    bool wiresharkIsBitfieldMember() const;
    std::string wiresharkForcedIntegralFieldMask(const WiresharkField* refField) const;
    std::string wiresharkForcedIntegralFieldType(const WiresharkField* refField) const;
    unsigned wiresharkForcedMaskShift(const WiresharkField* refField) const;
    unsigned wiresharkForcedBitLength(const WiresharkField* refField) const;
    std::string wiresharkFieldDescriptionStr(const WiresharkField* refField) const;
    std::string wiresharkFieldDisplayNameStr(const WiresharkField* refField) const;
    std::string wiresharkFieldNameVarNameStr(const WiresharkField* refField) const;
    bool wiresharkHasOverrideCode() const;

private:
    using WiresharkCustomCodeFunc = std::string (WiresharkField::*)(bool& hasCode) const;

    struct WiresharkCustomCode
    {
        std::string m_read;
        std::string m_valid;
        std::string m_name;

        bool m_hasRead = false;
        bool m_hasValid = false;
        bool m_hasName = false;
    };

    bool wiresharkCopyCodeFromInternal();
    bool wiresharkPrepareOverrideInternal(
        commsdsl::parse::ParseOverrideType type,
        const std::string& name,
        WiresharkCustomCodeFunc codeFunc,
        std::string& code,
        bool& hasCode);

    std::string wiresharkNameDefInternal(const WiresharkField* refField) const;
    std::string wiresharkDissectBodyInternal() const;
    std::string wiresharkCustomReadCodeInternal(bool& hasRealCode) const;
    std::string wiresharkCustomValidCodeInternal(bool& hasRealCode) const;
    std::string wiresharkCustomNameCodeInternal(bool& hasRealCode) const;

    GenField& m_genField;
    WiresharkCustomCode m_customCode;
};

} // namespace commsdsl2wireshark
