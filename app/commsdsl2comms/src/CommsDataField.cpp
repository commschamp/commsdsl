//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "CommsDataField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

CommsDataField::CommsDataField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsDataField::prepareImpl()
{
    bool result = Base::prepareImpl() && commsPrepare();
    if (result) {
        auto* externalPrefix = externalPrefixField();
        m_commsExternalPrefixField = dynamic_cast<CommsField*>(externalPrefix);
        assert((m_commsExternalPrefixField != nullptr) || (externalPrefix == nullptr)); // Make sure dynamic cast is successful

        auto* memberPrefix = memberPrefixField();
        m_commsMemberPrefixField = dynamic_cast<CommsField*>(memberPrefix);
        assert((m_commsMemberPrefixField != nullptr) || (memberPrefix == nullptr)); // Make sure dynamic cast is successful
    }
    return result;
}

bool CommsDataField::writeImpl() const
{
    return commsWrite();
}

CommsDataField::IncludesList CommsDataField::commsCommonIncludesImpl() const 
{
    if (m_commsMemberPrefixField == nullptr) {
        return IncludesList();
    }

    return m_commsMemberPrefixField->commsCommonIncludes();
}

std::string CommsDataField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsDataField::commsCommonMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::emptyString();
    }

    return m_commsMemberPrefixField->commsCommonCode();
}

CommsDataField::IncludesList CommsDataField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/ArrayList.h",
        "<cstdint>"
    };

    do {
        auto obj = dataDslObj();
        if (!obj.defaultValue().empty()) {
            result.insert(result.end(), {
                "comms/util/assign.h",
                "<iterator>"                
            });            
        }   

        if (m_commsMemberPrefixField != nullptr) {
            auto extraIncs = m_commsMemberPrefixField->commsDefIncludes();
            result.reserve(result.size() + extraIncs.size());
            std::move(extraIncs.begin(), extraIncs.end(), std::back_inserter(result));
            break;
        }

        if (m_commsExternalPrefixField != nullptr) {
            result.push_back(comms::relHeaderPathFor(m_commsExternalPrefixField->field(), generator()));
        }

        auto& detachedPrefixName = obj.detachedPrefixFieldName();
        if (!detachedPrefixName.empty()) {
            result.insert(result.end(), {
                "<algorithm>",
                "<limits>"
            });
        }
    } while (false);
    return result;
}

std::string CommsDataField::commsDefMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::emptyString();
    }

    return m_commsMemberPrefixField->commsDefCode();
}

std::string CommsDataField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
    "comms::field::ArrayList<\n"
    "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "    std::uint8_t#^#COMMA#$#\n"
    "    #^#FIELD_OPTS#$#\n"
    ">";    

    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", generator().schemaOf(*this).mainNamespace()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsDataField::commsDefConstructCodeImpl() const
{
    auto obj = dataDslObj();
    auto& defaultValue = obj.defaultValue();
    if (defaultValue.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "static const std::uint8_t Data[] = {\n"
        "    #^#BYTES#$#\n"
        "};\n"
        "comms::util::assign(Base::value(), std::begin(Data), std::end(Data));\n"
        ;
    util::StringsList bytes;
    bytes.reserve(defaultValue.size());
    for (auto& b : defaultValue) {
        std::stringstream stream;
        stream << std::hex << "0x" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(b);
        bytes.push_back(stream.str());
    }

    std::string bytesStr = util::strListToString(bytes, ", ", "");
    bytesStr = util::strMakeMultiline(bytesStr);

    util::ReplacementMap repl = {
        {"BYTES", std::move(bytesStr)}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsDataField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = dataDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return strings::emptyString();
    }

    auto sepPos = detachedPrefixName.find_first_of(".");
    std::string sibName = detachedPrefixName.substr(0, sepPos);
    std::string accRest;
    if (sepPos < detachedPrefixName.size()) {
        accRest = detachedPrefixName.substr(sepPos + 1);
    }

    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&sibName](auto& f)
            {
                return f->field().dslObj().name() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::emptyString();
    }

    auto fieldPrefix = "field_" + comms::accessName(dslObj().name()) + "()";
    auto sibPrefix = "field_" + comms::accessName((*iter)->field().dslObj().name()) + "()";

    auto conditions = commsCompOptChecks(std::string(), fieldPrefix);
    auto sibConditions = (*iter)->commsCompOptChecks(accRest, sibPrefix);

    if (!sibConditions.empty()) {
        std::move(sibConditions.begin(), sibConditions.end(), std::back_inserter(conditions));
    }

    util::ReplacementMap repl = {
        {"DATA_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
        {"LEN_VALUE", (*iter)->commsValueAccessStr(accRest, sibPrefix)},
    };

    if (conditions.empty()) {
        static const std::string Templ =
            "#^#DATA_FIELD#$#.forceReadLength(\n"
            "    static_cast<std::size_t>(#^#LEN_VALUE#$#));\n";
        
        return util::processTemplate(Templ, repl);
    }

    static const std::string Templ = 
        "if (#^#COND#$#) {\n"
        "    #^#DATA_FIELD#$#.forceReadLength(\n"
        "        static_cast<std::size_t>(#^#LEN_VALUE#$#));\n"        
        "}";

    repl["COND"] = util::strListToString(conditions, " &&\n", "");
    return util::processTemplate(Templ, repl);
}

std::string CommsDataField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = dataDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return strings::emptyString();
    }

    auto sepPos = detachedPrefixName.find_first_of(".");
    std::string sibName = detachedPrefixName.substr(0, sepPos);
    std::string accRest;
    if (sepPos < detachedPrefixName.size()) {
        accRest = detachedPrefixName.substr(sepPos + 1);
    }

    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&sibName](auto& f)
            {
                return f->field().dslObj().name() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::emptyString();
    }

    static const std::string Templ = 
        "auto lenValue = #^#LEN_VALUE#$#;\n"
        "auto realLength = #^#DATA_FIELD#$#.value().size();\n"
        "if (static_cast<std::size_t>(lenValue) == realLength) {\n"
        "    return false;\n"
        "}\n\n"
        "using LenValueType = typename std::decay<decltype(lenValue)>::type;\n"
        "static const auto MaxLenValue = static_cast<std::size_t>(std::numeric_limits<LenValueType>::max());\n"
        "auto maxAllowedLen = std::min(MaxLenValue, realLength);\n"
        "#^#LEN_FIELD#$#.setValue(maxAllowedLen);\n"
        "if (maxAllowedLen < realLength) {\n"
        "    #^#DATA_FIELD#$#.value().resize(maxAllowedLen);\n"
        "}\n"
        "return true;";

    auto fieldPrefix = "field_" + comms::accessName(dslObj().name()) + "()";
    auto sibPrefix = "field_" + comms::accessName((*iter)->field().dslObj().name()) + "()";
    util::ReplacementMap repl = {
        {"LEN_VALUE", (*iter)->commsValueAccessStr(accRest, sibPrefix)},
        {"LEN_FIELD", (*iter)->commsFieldAccessStr(accRest, sibPrefix)},
        {"DATA_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
    };

    return util::processTemplate(Templ, repl);
}

bool CommsDataField::commsIsLimitedCustomizableImpl() const
{
    return true;
}

std::string CommsDataField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::emptyString();
    }

    assert(fieldOptsFunc != nullptr);
    return (m_commsMemberPrefixField->*fieldOptsFunc)();
}

CommsDataField::StringsList CommsDataField::commsExtraDataViewDefaultOptionsImpl() const
{
    return 
        StringsList{
            "comms::option::app::OrigDataView"
        };
}

CommsDataField::StringsList CommsDataField::commsExtraBareMetalDefaultOptionsImpl() const
{
    auto obj = dataDslObj();
    auto fixedLength = obj.fixedLength();
    if (fixedLength != 0U) {
        return 
            StringsList{
                "comms::option::app::SequenceFixedSizeUseFixedSizeStorage"
            };        
    }

    return 
        StringsList{
            "comms::option::app::FixedSizeStorage<DEFAULT_SEQ_FIXED_STORAGE_SIZE>"
        };    
}

std::size_t CommsDataField::commsMaxLengthImpl() const
{
    auto obj = dataDslObj();
    if (obj.fixedLength() != 0U) {
        return CommsBase::commsMaxLengthImpl();
    }

    return comms::maxPossibleLength();    
}

std::string CommsDataField::commsSizeAccessStrImpl([[maybe_unused]] const std::string& accStr, const std::string& prefix) const
{
    assert(accStr.empty());
    return prefix + ".getValue().size()";
}

std::string CommsDataField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddFixedLengthOptInternal(opts);
    commsAddLengthPrefixOptInternal(opts);
    commsAddLengthForcingOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

void CommsDataField::commsAddFixedLengthOptInternal(StringsList& opts) const
{
    auto obj = dataDslObj();
    auto fixedLen = obj.fixedLength();
    if (fixedLen == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        util::numToString(static_cast<std::uintmax_t>(fixedLen)) +
        ">";
    opts.push_back(std::move(str));
}

void CommsDataField::commsAddLengthPrefixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalPrefixField == nullptr) && (m_commsMemberPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberPrefixField != nullptr) {
        prefixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::className(m_commsMemberPrefixField->field().name());
    }
    else {
        assert(m_commsExternalPrefixField != nullptr);
        prefixName = comms::scopeFor(m_commsExternalPrefixField->field(), generator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void CommsDataField::commsAddLengthForcingOptInternal(StringsList& opts) const
{
    auto obj = dataDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return;
    }

    opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
}

} // namespace commsdsl2comms
