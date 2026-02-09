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

#include "WiresharkRefField.h"

#include "WiresharkGenerator.h"

#include <cassert>

namespace commsdsl2wireshark
{

WiresharkRefField::WiresharkRefField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkRefField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_wiresharkField = WiresharkField::wiresharkCast(genReferencedField());
    assert(m_wiresharkField != nullptr);
    return true;
}

std::string WiresharkRefField::wiresharkFieldRegistrationImpl(const std::string& objName, const std::string& refName) const
{
    auto thisObjName = objName;
    auto thisRefName = refName;

    if (thisObjName.empty()) {
        thisObjName = wiresharkFieldObjName();
    }

    if (thisRefName.empty()) {
        thisRefName = wiresharkFieldRefName();
    }

    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkFieldRegistration(objName, refName);
}

} // namespace commsdsl2wireshark
