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

#include "commsdsl/gen/GenMessage.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkMessage final : public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenMessage = commsdsl::gen::GenMessage;
    using WiresharkFieldsList = WiresharkField::WiresharkFieldsList;

    WiresharkMessage(WiresharkGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~WiresharkMessage();

    static WiresharkMessage& wiresharkCast(GenMessage& obj)
    {
        return static_cast<WiresharkMessage&>(obj);
    }

    static const WiresharkMessage& wiresharkCast(const GenMessage& obj)
    {
        return static_cast<const WiresharkMessage&>(obj);
    }

    std::string wiresharkDissectName() const;
    std::string wiresharkDissectCode() const;

protected:
    virtual bool genPrepareImpl() override;

private:
    WiresharkFieldsList m_wiresharkFields;
};

} // namespace commsdsl2wireshark
