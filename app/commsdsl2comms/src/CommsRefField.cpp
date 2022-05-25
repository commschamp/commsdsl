//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsRefField::prepareImpl()
{
    bool result = Base::prepareImpl() && commsPrepare();
    if (!result) {
        return false;
    }

    auto* refField = referencedField();
    m_commsReferencedField = dynamic_cast<CommsField*>(refField);
    assert(m_commsReferencedField != nullptr);
    m_commsReferencedField->commsSetReferenced();

    if ((refDslObj().semanticType() == commsdsl::parse::Field::SemanticType::Length) && 
        (refField->dslObj().semanticType() != commsdsl::parse::Field::SemanticType::Length) &&
        (!commsHasCustomValue()) && 
        (!m_commsReferencedField->commsHasCustomValue())) {
        generator().logger().warning(
            "Field \"" + comms::scopeFor(*this, generator()) + "\" is used as \"length\" field (semanticType=\"length\"), but custom value "
            "retrieval functionality is not provided. Please create relevant code injection functionality with \"" + 
            strings::valueFileSuffixStr() + "\" file name suffix. Inside that file the following functions are "
            "expected to be defined: getValue(), setValue(), and maxValue()."
        );
    }     

    return true;
}

bool CommsRefField::writeImpl() const
{
    return commsWrite();
}

CommsRefField::IncludesList CommsRefField::commsCommonIncludesImpl() const 
{
    assert(m_commsReferencedField != nullptr);
    IncludesList result = {
        comms::relCommonHeaderPathFor(m_commsReferencedField->field(), generator())
    };

    return result;
}

std::string CommsRefField::commsCommonCodeBaseClassImpl() const
{
    assert(m_commsReferencedField != nullptr);
    return comms::commonScopeFor(m_commsReferencedField->field(), generator());
}

std::string CommsRefField::commsCommonCodeBodyImpl() const
{
    if (!commsDefHasNameFuncImpl()) {
        return strings::emptyString();
    }

    return commsCommonNameFuncCode();
}

std::string CommsRefField::commsCommonMembersBaseClassImpl() const
{
    assert(m_commsReferencedField != nullptr);
    if (!m_commsReferencedField->commsHasMembersCode()) {
        return strings::emptyString();
    }

    auto str = comms::commonScopeFor(m_commsReferencedField->field(), generator());
    auto& commonSuffix = strings::commonSuffixStr();
    assert(commonSuffix.size() < str.size());
    auto commonSuffixPos = str.size() - commonSuffix.size();
    assert(commonSuffix == str.substr(commonSuffixPos));
    str.insert(commonSuffixPos, strings::membersSuffixStr());
    return str;
}

CommsRefField::IncludesList CommsRefField::commsDefIncludesImpl() const
{
    assert(m_commsReferencedField != nullptr);
    IncludesList result = {
        comms::relHeaderPathFor(m_commsReferencedField->field(), generator())
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

    std::string templOpt;
    if (!comms::isInterfaceDeepMemberField(*this)) {
        templOpt = "TOpt";
    }
    
    util::ReplacementMap repl = {
        {"REF_FIELD", comms::scopeFor(m_commsReferencedField->field(), generator())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    return util::processTemplate(Templ, repl);
}

bool CommsRefField::commsDefHasNameFuncImpl() const
{
    auto dslObj = refDslObj();
    auto thisDisplayName = util::displayName(dslObj.displayName(), dslObj.name());
    auto refDslObj = m_commsReferencedField->field().dslObj();
    auto refDisplayName = util::displayName(refDslObj.displayName(), refDslObj.name());

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

std::string CommsRefField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddProtocolOptInternal(opts);
    commsAddFieldDefOptions(opts);
    commsAddBitLengthOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

void CommsRefField::commsAddProtocolOptInternal(StringsList& opts) const
{
    if (comms::isInterfaceDeepMemberField(*this)) {
        opts.push_back(comms::scopeForOptions(strings::defaultOptionsClassStr(), generator()));
    }
    else {
        opts.push_back("TOpt");
    }
}

void CommsRefField::commsAddBitLengthOptInternal(StringsList& opts) const
{
    auto obj = refDslObj();
    auto bitLength = obj.bitLength();
    if (bitLength != 0U) {
        opts.push_back("comms::option::def::FixedBitLength<" + util::numToString(bitLength) + '>');
    }    
}

} // namespace commsdsl2comms
