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

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <functional>
#include <tuple>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

[[nodiscard]] auto wiresharkParseObjAsTupleInternal(const WiresharkRefField::ParseField& parseObj)
{
    return
        std::make_tuple(
            std::cref(parseObj.parseName()),
            std::cref(parseObj.parseDisplayName()),
            std::cref(parseObj.parseDescription()),
            parseObj.parseSemanticType(),
            parseObj.parseIsPseudo(),
            parseObj.parseIsFailOnInvalid());
}

} // namespace

WiresharkRefField::WiresharkRefField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkRefField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    m_wiresharkField = WiresharkField::wiresharkCast(genReferencedField());
    assert(m_wiresharkField != nullptr);

    m_alias = wiresharkIsAliasInternal();
    return true;
}

std::string WiresharkRefField::wiresharkDissectNameImpl(const WiresharkField* refField) const
{
    if (!m_alias) {
        return WiresharkBase::wiresharkDissectNameImpl(refField);
    }

    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkDissectName(refField);
}

std::string WiresharkRefField::wiresharkDissectCodeImpl(const WiresharkField* refField) const
{
    if (m_alias) {
        return strings::genEmptyString();
    }

    if (!wiresharkMustCopyDissectInternal()) {
        return WiresharkBase::wiresharkDissectCodeImpl(refField);
    }

    if (refField == nullptr) {
        refField = this;
    }

    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkDissectCode(refField);
}

std::string WiresharkRefField::wiresharkExtractorsRegCodeImpl() const
{
    if (m_alias) {
        return strings::genEmptyString();
    }

    return WiresharkBase::wiresharkExtractorsRegCodeImpl();
}

std::string WiresharkRefField::wiresharkFieldObjNameImpl(const WiresharkField* refField) const
{
    if (m_alias) {
        return WiresharkBase::wiresharkFieldObjNameImpl(refField);
    }

    if (refField == nullptr) {
        refField = this;
    }

    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkFieldObjName(refField);
}

std::string WiresharkRefField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    assert(m_wiresharkField != nullptr);
    if (m_alias) {
        genGenerator().genLogger().genDebug(genName() + " field is full alias to " + m_wiresharkField->wiresharkGenField().genParseObj().parseExternalRef() + ", not generating registration");
        return strings::genEmptyString();
    }

    if (refField == nullptr) {
        refField = this;
    }

    return m_wiresharkField->wiresharkFieldRegistration(refField);
}

std::string WiresharkRefField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(!m_alias);
    assert(m_wiresharkField != nullptr);
    assert(!wiresharkMustCopyDissectInternal());

    static const std::string Templ =
        "result, next_offset = #^#FIELD#$#(#^#SIG#$#)"
        ;

    util::GenReplacementMap repl {
        {"SIG", wiresharkDissectSignature()},
        {"FIELD", m_wiresharkField->wiresharkDissectName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkRefField::wiresharkIsAliasInternal() const
{
    if (wiresharkMustCopyDissectInternal()) {
        return false;
    }

    assert(m_wiresharkField != nullptr);
    auto& thisObj = wiresharkGenField().genParseObj();
    auto& refObj = m_wiresharkField->wiresharkGenField().genParseObj();
    return wiresharkParseObjAsTupleInternal(thisObj) == wiresharkParseObjAsTupleInternal(refObj);
}

bool WiresharkRefField::wiresharkMustCopyDissectInternal() const
{
    return
        wiresharkIsBitfieldMember() ||
        wiresharkHasOverrideCode();
}

} // namespace commsdsl2wireshark
