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

    std::string wiresharkDissectName() const;
    std::string wiresharkDissectCode() const;
    std::string wiresharkFieldObjName() const;

protected:
    virtual std::string wiresharkFieldRegistrationImpl() const;

    std::string wiresharkFieldRefName() const;
    std::string wiresharkForcedIntegralFieldMask() const;
    std::string wiresharkFieldDescriptionStr() const;

private:
    std::string wiresharkDissectBodyInternal() const;

    GenField& m_genField;
};

} // namespace commsdsl2wireshark
