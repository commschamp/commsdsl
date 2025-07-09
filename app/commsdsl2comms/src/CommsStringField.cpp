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

#include "CommsStringField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

CommsStringField::CommsStringField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField parseObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, parseObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsStringField::genPrepareImpl()
{
    bool result = Base::genPrepareImpl() && commsPrepare();
    if (result) {
        auto* externalPrefix = genExternalPrefixField();
        m_commsExternalPrefixField = dynamic_cast<CommsField*>(externalPrefix);
        assert((m_commsExternalPrefixField != nullptr) || (externalPrefix == nullptr)); // Make sure dynamic cast is successful

        auto* memberPrefix = genMemberPrefixField();
        m_commsMemberPrefixField = dynamic_cast<CommsField*>(memberPrefix);
        assert((m_commsMemberPrefixField != nullptr) || (memberPrefix == nullptr)); // Make sure dynamic cast is successful
    }
    return result;
}

bool CommsStringField::genWriteImpl() const
{
    return commsWrite();
}

CommsStringField::CommsIncludesList CommsStringField::commsCommonIncludesImpl() const 
{
    if (m_commsMemberPrefixField == nullptr) {
        return CommsIncludesList();
    }

    return m_commsMemberPrefixField->commsCommonIncludes();
}

std::string CommsStringField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsStringField::commsCommonMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::genEmptyString();
    }

    return m_commsMemberPrefixField->commsCommonCode();
}

CommsStringField::CommsIncludesList CommsStringField::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/field/String.h"
    };

    do {
        auto obj = genStringFieldParseObj();
        if (obj.parseHasZeroTermSuffix()) {
            result.insert(result.end(), {
                "comms/field/IntValue.h",
                "<cstdint>"                
            });
        }

        auto& validValues = obj.parseValidValues();
        if (!validValues.empty()) {
            result.insert(result.end(), {
                "<algorithm>",
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
            result.push_back(comms::genRelHeaderPathFor(m_commsExternalPrefixField->commsGenField(), genGenerator()));
        }

        auto& detachedPrefixName = obj.parseDetachedPrefixFieldName();
        if (!detachedPrefixName.empty()) {
            result.insert(result.end(), {
                "<algorithm>",
                "<limits>"
            });
        }

    } while (false);
    return result;
}

std::string CommsStringField::commsDefMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::genEmptyString();
    }

    return m_commsMemberPrefixField->commsDefCode();
}

std::string CommsStringField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
    "comms::field::String<\n"
    "    #^#PROT_NAMESPACE#$#::field::FieldBase<>#^#COMMA#$#\n"
    "    #^#FIELD_OPTS#$#\n"
    ">";    

    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", genGenerator().genSchemaOf(*this).genMainNamespace()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsStringField::commsDefConstructCodeImpl() const
{
    auto obj = genStringFieldParseObj();
    auto& defaultValue = obj.parseDefaultValue();
    if (defaultValue.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "static const char Str[] = \"#^#STR#$#\";\n"
        "static const std::size_t StrSize = std::extent<decltype(Str)>::value;\n"
        "Base::setValue(typename Base::ValueType(&Str[0], StrSize - 1));\n";

    util::GenReplacementMap repl = {
        {"STR", defaultValue}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsStringField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genStringFieldParseObj();
    auto& detachedPrefixName = obj.parseDetachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return strings::genEmptyString();
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
                return f->commsGenField().genParseObj().parseName() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    auto fieldPrefix = "field_" + comms::genAccessName(genParseObj().parseName()) + "()";
    auto sibPrefix = "field_" + comms::genAccessName((*iter)->commsGenField().genParseObj().parseName()) + "()";

    auto conditions = commsCompOptChecks(std::string(), fieldPrefix);
    auto sibConditions = (*iter)->commsCompOptChecks(accRest, sibPrefix);

    if (!sibConditions.empty()) {
        std::move(sibConditions.begin(), sibConditions.end(), std::back_inserter(conditions));
    }

    util::GenReplacementMap repl = {
        {"STR_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
        {"LEN_VALUE", (*iter)->commsValueAccessStr(accRest, sibPrefix)},
    };

    if (conditions.empty()) {
        static const std::string Templ =
            "#^#STR_FIELD#$#.forceReadLength(\n"
            "    static_cast<std::size_t>(#^#LEN_VALUE#$#));\n";
        
        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ = 
        "if (#^#COND#$#) {\n"
        "    #^#STR_FIELD#$#.forceReadLength(\n"
        "        static_cast<std::size_t>(#^#LEN_VALUE#$#));\n"        
        "}";

    repl["COND"] = util::genStrListToString(conditions, " &&\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsStringField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genStringFieldParseObj();
    auto& detachedPrefixName = obj.parseDetachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return strings::genEmptyString();
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
                return f->commsGenField().genParseObj().parseName() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "auto lenValue = #^#LEN_VALUE#$#;\n"
        "auto realLength = #^#STR_FIELD#$#.value().size();\n"
        "if (static_cast<std::size_t>(lenValue) == realLength) {\n"
        "    return false;\n"
        "}\n\n"
        "using LenValueType = typename std::decay<decltype(lenValue)>::type;\n"
        "static const auto MaxLenValue = static_cast<std::size_t>(std::numeric_limits<LenValueType>::max());\n"
        "auto maxAllowedLen = std::min(MaxLenValue, realLength);\n"
        "#^#LEN_FIELD#$#.setValue(maxAllowedLen);\n"
        "if (maxAllowedLen < realLength) {\n"
        "    #^#STR_FIELD#$#.value().resize(maxAllowedLen);\n"
        "}\n"
        "return true;";

    auto fieldPrefix = "field_" + comms::genAccessName(genParseObj().parseName()) + "()";
    auto sibPrefix = "field_" + comms::genAccessName((*iter)->commsGenField().genParseObj().parseName()) + "()";
    util::GenReplacementMap repl = {
        {"LEN_VALUE", (*iter)->commsValueAccessStr(accRest, sibPrefix)},
        {"LEN_FIELD", (*iter)->commsFieldAccessStr(accRest, sibPrefix)},
        {"STR_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsStringField::commsDefValidFuncBodyImpl() const
{
    auto& validValues = genStringFieldParseObj().parseValidValues();
    if (validValues.empty()) {
        return std::string();
    }

    util::GenStringsList values;
    for (auto& info : validValues) {
        if (!genGenerator().genDoesElementExist(info.m_sinceVersion, info.m_deprecatedSince, true)) {
            continue;
        }

        values.push_back("\"" + info.m_value + "\"");
    }

    static const std::string Templ = 
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "static const typename Base::ValueType Map[] = {\n"
        "    #^#VALUES#$#\n"
        "};\n"
        "\n"
        "auto iter = std::lower_bound(std::begin(Map), std::end(Map), Base::getValue());\n"
        "return (iter != std::end(Map)) && ((*iter) == Base::getValue());\n"
        ;

    util::GenReplacementMap repl = {
        {"VALUES", util::genStrListToString(values, ",\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool CommsStringField::commsIsLimitedCustomizableImpl() const
{
    return true;
}

std::string CommsStringField::commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::genEmptyString();
    }

    assert(fieldOptsFunc != nullptr);
    return (m_commsMemberPrefixField->*fieldOptsFunc)();
}

CommsStringField::GenStringsList CommsStringField::commsExtraDataViewDefaultOptionsImpl() const
{
    return 
        GenStringsList{
            "comms::option::app::OrigDataView"
        };
}

CommsStringField::GenStringsList CommsStringField::commsExtraBareMetalDefaultOptionsImpl() const
{
    auto obj = genStringFieldParseObj();
    auto fixedLength = obj.parseFixedLength();
    if (fixedLength != 0U) {
        return 
            GenStringsList{
                "comms::option::app::SequenceFixedSizeUseFixedSizeStorage"
            };        
    }

    return 
        GenStringsList{
            "comms::option::app::FixedSizeStorage<DEFAULT_SEQ_FIXED_STORAGE_SIZE>"
        };    
}

std::string CommsStringField::commsSizeAccessStrImpl([[maybe_unused]] const std::string& accStr, const std::string& prefix) const
{
    assert(accStr.empty());
    return prefix + ".getValue().size()";
}

std::size_t CommsStringField::commsMaxLengthImpl() const
{
    auto obj = genStringFieldParseObj();
    if (obj.parseFixedLength() != 0U) {
        return CommsBase::commsMaxLengthImpl();
    }

    return comms::genMaxPossibleLength();
}

std::string CommsStringField::commsCompValueCastTypeImpl(
    [[maybe_unused]] const std::string& accStr, 
    [[maybe_unused]] const std::string& prefix) const
{
    assert(accStr.empty());
    return strings::genEmptyString();
}

std::string CommsStringField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    std::string valueTmp = value;
    do {
        if (value.empty()) {
            break;
        }

        static const char Prefix = strings::genStringRefPrefix();
        if (value[0] == Prefix) {
            auto* refField = genGenerator().genFindField(std::string(value, 1));
            if (refField == nullptr) {
                genGenerator().genLogger().genWarning("Failed to find referenced field: " + value);
                break;
            }

            if (refField->genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::String) {
                genGenerator().genLogger().genWarning("Not referencing <string> field: " + value);
                break;                
            }

            auto* refStringField = static_cast<const CommsStringField*>(refField);
            valueTmp = refStringField->genStringFieldParseObj().parseDefaultValue();
            break;
        }

        auto prefixPos = value.find_first_of(Prefix);
        if (prefixPos == std::string::npos) {
            break;
        }

        assert(0U < prefixPos);
        bool allBackSlashes =
            std::all_of(
                value.begin(), value.begin() + static_cast<std::ptrdiff_t>(prefixPos),
                [](char ch)
                {
                    return ch == '\\';
                });

        if (!allBackSlashes) {
            break;
        }

        valueTmp.assign(value, 1, std::string::npos);
    } while (false);

    return CommsBase::commsCompPrepValueStrImpl(accStr, '\"' + valueTmp + '\"');
}


std::string CommsStringField::commsDefFieldOptsInternal() const
{
    util::GenStringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddFixedLengthOptInternal(opts);
    commsAddLengthPrefixOptInternal(opts);
    commsAddTermSuffixOptInternal(opts);
    commsAddLengthForcingOptInternal(opts);

    return util::genStrListToString(opts, ",\n", "");
}

void CommsStringField::commsAddFixedLengthOptInternal(GenStringsList& opts) const
{
    auto obj = genStringFieldParseObj();
    auto fixedLen = obj.parseFixedLength();
    if (fixedLen == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        util::genNumToString(static_cast<std::uintmax_t>(fixedLen)) +
        ">";
    opts.push_back(std::move(str));
}

void CommsStringField::commsAddLengthPrefixOptInternal(GenStringsList& opts) const
{
    if ((m_commsExternalPrefixField == nullptr) && (m_commsMemberPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberPrefixField != nullptr) {
        prefixName = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();
        if (comms::genIsGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::genClassName(m_commsMemberPrefixField->commsGenField().genName());
    }
    else {
        assert(m_commsExternalPrefixField != nullptr);
        prefixName = comms::genScopeFor(m_commsExternalPrefixField->commsGenField(), genGenerator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void CommsStringField::commsAddTermSuffixOptInternal(GenStringsList& opts) const
{
    auto obj = genStringFieldParseObj();
    if (!obj.parseHasZeroTermSuffix()) {
        return;
    }

    static const std::string Templ =
        "comms::option::def::SequenceTerminationFieldSuffix<\n"
        "    comms::field::IntValue<\n"
        "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "        std::uint8_t,\n"
        "        comms::option::def::ValidNumValueRange<0, 0>,\n"
        "        comms::option::def::FailOnInvalid<>\n"
        "    >\n"
        ">";

    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", genGenerator().genSchemaOf(*this).genMainNamespace()},
    };

    opts.push_back(util::genProcessTemplate(Templ, repl));
}

void CommsStringField::commsAddLengthForcingOptInternal(GenStringsList& opts) const
{
    auto obj = genStringFieldParseObj();
    auto& detachedPrefixName = obj.parseDetachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return;
    }

    opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
}

} // namespace commsdsl2comms
