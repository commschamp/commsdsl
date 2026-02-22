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

#include "WiresharkListField.h"

#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

WiresharkListField::WiresharkListField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkListField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    return true;
}

std::string WiresharkListField::wiresharkMembersDissectCodeImpl() const
{
    util::GenStringsList elems;

    auto addDisectCodeFunc =
        [&elems](auto* field)
        {
            if (field != nullptr) {
                elems.push_back(WiresharkField::wiresharkCast(field)->wiresharkDissectCode());
            }
        };

    addDisectCodeFunc(genMemberElementField());
    addDisectCodeFunc(genMemberCountPrefixField());
    addDisectCodeFunc(genMemberLengthPrefixField());
    addDisectCodeFunc(genMemberElemLengthPrefixField());
    addDisectCodeFunc(genMemberTermSuffixField());

    genGenerator().genLogger().genDebug("There are " + std::to_string(elems.size()) + " member elements of the " + genParseObj().parseInnerRef() + " list field");
    if (elems.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(elems, "\n", "");
}

} // namespace commsdsl2wireshark
