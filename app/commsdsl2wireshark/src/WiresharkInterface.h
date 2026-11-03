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

#include "WiresharkField.h"

#include "commsdsl/gen/GenInterface.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkInterface final : public commsdsl::gen::GenInterface
{
    using GenBase = commsdsl::gen::GenInterface;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenInterface = commsdsl::gen::GenInterface;
    using WiresharkFieldsList = WiresharkField::WiresharkFieldsList;

    WiresharkInterface(WiresharkGenerator& generator, ParseInterface parseObj, GenElem* parent);
    virtual ~WiresharkInterface();

    [[nodiscard]] static WiresharkInterface* wiresharkCast(GenInterface* obj)
    {
        return static_cast<WiresharkInterface*>(obj);
    }

    [[nodiscard]] static const WiresharkInterface* wiresharkCast(const GenInterface* obj)
    {
        return static_cast<const WiresharkInterface*>(obj);
    }

    [[nodiscard]] bool wiresharkDissectionAllowed() const;
    [[nodiscard]] std::string wiresharkDissectCode() const;
    [[nodiscard]] std::string wiresharkExtractorsRegCode() const;
    [[nodiscard]] bool wiresharkNeedsOptionalModeDefinition() const;
    [[nodiscard]] const WiresharkFieldsList& wiresharkMemberFields() const;
    [[nodiscard]] std::string wiresharkDefaultAssignments() const;
    [[nodiscard]] const WiresharkField* wiresharkFindField(const std::string& name) const;
    [[nodiscard]] const WiresharkField* wiresharkVersionField() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;

private:
    WiresharkFieldsList m_wiresharkFields;
};

} // namespace commsdsl2wireshark
