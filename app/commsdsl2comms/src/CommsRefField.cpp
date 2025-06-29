//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsRefField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <cassert>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

CommsRefField::CommsRefField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsRefField::genPrepareImpl()
{
    bool result = Base::genPrepareImpl() && commsPrepare();
    if (!result) {
        return false;
    }

    auto* refField = genReferencedField();
    m_commsReferencedField = dynamic_cast<CommsField*>(refField);
    assert(m_commsReferencedField != nullptr);

    if ((genRefFieldParseObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::Length) && 
        (refField->genParseObj().parseSemanticType() != commsdsl::parse::ParseField::SemanticType::Length) &&
        (!commsHasCustomValue()) && 
        (!m_commsReferencedField->commsHasCustomValue())) {
        genGenerator().genLogger().genWarning(
            "Field \"" + comms::genScopeFor(*this, genGenerator()) + "\" is used as \"length\" field (semanticType=\"length\"), but custom value "
            "retrieval functionality is not provided. Please create relevant code injection functionality with \"" + 
            strings::genValueFileSuffixStr() + "\" file name suffix. Inside that file the following functions are "
            "expected to be defined: getValue(), setValue(), and maxValue()."
        );
    }     

    return true;
}

bool CommsRefField::genWriteImpl() const
{
    return commsWrite();
}

CommsRefField::IncludesList CommsRefField::commsCommonIncludesImpl() const 
{
    assert(m_commsReferencedField != nullptr);
    IncludesList result = {
        comms::genRelCommonHeaderPathFor(m_commsReferencedField->field(), genGenerator())
    };

    return result;
}

std::string CommsRefField::commsCommonCodeBaseClassImpl() const
{
    assert(m_commsReferencedField != nullptr);
    return comms::genCommonScopeFor(m_commsReferencedField->field(), genGenerator());
}

std::string CommsRefField::commsCommonCodeBodyImpl() const
{
    if (!commsDefHasNameFuncImpl()) {
        return strings::genEmptyString();
    }

    return commsCommonNameFuncCode();
}

std::string CommsRefField::commsCommonMembersBaseClassImpl() const
{
    assert(m_commsReferencedField != nullptr);
    if (!m_commsReferencedField->commsHasMembersCode()) {
        return strings::genEmptyString();
    }

    auto str = comms::genCommonScopeFor(m_commsReferencedField->field(), genGenerator());
    auto& commonSuffix = strings::genCommonSuffixStr();
    assert(commonSuffix.size() < str.size());
    auto commonSuffixPos = str.size() - commonSuffix.size();
    assert(commonSuffix == str.substr(commonSuffixPos));
    str.insert(commonSuffixPos, strings::genMembersSuffixStr());
    return str;
}

CommsRefField::IncludesList CommsRefField::commsDefIncludesImpl() const
{
    assert(m_commsReferencedField != nullptr);
    IncludesList result = {
        comms::genRelHeaderPathFor(m_commsReferencedField->field(), genGenerator())
    };

    return result;
}

std::string CommsRefField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "#^#REF_FIELD#$#<\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";    

    assert(m_commsReferencedField != nullptr);

    util::ReplacementMap repl = {
        {"REF_FIELD", comms::genScopeFor(m_commsReferencedField->field(), genGenerator())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool CommsRefField::commsIsLimitedCustomizableImpl() const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsIsFieldCustomizable();
}

bool CommsRefField::commsDefHasNameFuncImpl() const
{
    auto dslObj = genRefFieldParseObj();
    auto thisDisplayName = util::genDisplayName(dslObj.parseDisplayName(), dslObj.parseName());
    auto refDslObj = m_commsReferencedField->field().genParseObj();
    auto refDisplayName = util::genDisplayName(refDslObj.parseDisplayName(), refDslObj.parseName());

    return thisDisplayName != refDisplayName;
}

std::size_t CommsRefField::commsMinLengthImpl() const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsMinLength();
}

std::size_t CommsRefField::commsMaxLengthImpl() const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsMaxLength();
}

std::string CommsRefField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsValueAccessStr(accStr, prefix);
}

std::string CommsRefField::commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsSizeAccessStr(accStr, prefix);
}

void CommsRefField::commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsCompOptChecks(accStr, checks, prefix);
}

std::string CommsRefField::commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsCompValueCastType(accStr, prefix);
}

std::string CommsRefField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsCompPrepValueStr(accStr, value);
}

bool CommsRefField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    assert(m_commsReferencedField != nullptr);
    return m_commsReferencedField->commsVerifyInnerRef(refStr);
}

std::string CommsRefField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddProtocolOptInternal(opts);
    commsAddFieldDefOptions(opts);
    commsAddBitLengthOptInternal(opts);

    return util::genStrListToString(opts, ",\n", "");
}

void CommsRefField::commsAddProtocolOptInternal(StringsList& opts) const
{
    if (comms::genIsInterfaceDeepMemberField(*this)) {
        opts.push_back(comms::genScopeForOptions(strings::genDefaultOptionsClassStr(), genGenerator()));
    }
    else {
        opts.push_back("TOpt");
    }
}

void CommsRefField::commsAddBitLengthOptInternal(StringsList& opts) const
{
    auto obj = genRefFieldParseObj();
    auto bitLength = obj.parseBitLength();
    if (bitLength != 0U) {
        opts.push_back("comms::option::def::FixedBitLength<" + util::genNumToString(bitLength) + '>');
    }    
}

} // namespace commsdsl2comms
