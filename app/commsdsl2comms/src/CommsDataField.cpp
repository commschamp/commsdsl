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

namespace 
{

std::string bytesToString(const std::vector<std::uint8_t> value)
{
    util::StringsList bytes;
    bytes.reserve(value.size());
    for (auto& b : value) {
        std::stringstream stream;
        stream << std::hex << "0x" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(b);
        bytes.push_back(stream.str());
    }

    std::string bytesStr = util::genStrListToString(bytes, ", ", "");
    return util::genStrMakeMultiline(bytesStr);    
}

} // namespace 
    

CommsDataField::CommsDataField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsDataField::genPrepareImpl()
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

bool CommsDataField::genWriteImpl() const
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
        return strings::genEmptyString();
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
        auto obj = genDataFieldParseObj();
        if (!obj.parseDefaultValue().empty()) {
            result.insert(result.end(), {
                "<iterator>",  
                "comms/util/assign.h",
            });            
        }   

        auto& validValues = obj.parseValidValues();
        if (!validValues.empty()) {
            result.insert(result.end(), {
                "<algorithm>",
                "<iterator>",
                "comms/util/ArrayView.h",
            });            
        }            

        if (m_commsMemberPrefixField != nullptr) {
            auto extraIncs = m_commsMemberPrefixField->commsDefIncludes();
            result.reserve(result.size() + extraIncs.size());
            std::move(extraIncs.begin(), extraIncs.end(), std::back_inserter(result));
            break;
        }

        if (m_commsExternalPrefixField != nullptr) {
            result.push_back(comms::genRelHeaderPathFor(m_commsExternalPrefixField->field(), genGenerator()));
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

std::string CommsDataField::commsDefMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::genEmptyString();
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
        {"PROT_NAMESPACE", genGenerator().genSchemaOf(*this).genMainNamespace()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsDataField::commsDefConstructCodeImpl() const
{
    auto obj = genDataFieldParseObj();
    auto& defaultValue = obj.parseDefaultValue();
    if (defaultValue.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "static const std::uint8_t Data[] = {\n"
        "    #^#BYTES#$#\n"
        "};\n"
        "comms::util::assign(Base::value(), std::begin(Data), std::end(Data));\n"
        ;

    util::ReplacementMap repl = {
        {"BYTES", bytesToString(defaultValue)}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsDataField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genDataFieldParseObj();
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
                return f->field().genParseObj().parseName() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    auto fieldPrefix = "field_" + comms::genAccessName(genParseObj().parseName()) + "()";
    auto sibPrefix = "field_" + comms::genAccessName((*iter)->field().genParseObj().parseName()) + "()";

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
        
        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ = 
        "if (#^#COND#$#) {\n"
        "    #^#DATA_FIELD#$#.forceReadLength(\n"
        "        static_cast<std::size_t>(#^#LEN_VALUE#$#));\n"        
        "}";

    repl["COND"] = util::genStrListToString(conditions, " &&\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsDataField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genDataFieldParseObj();
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
                return f->field().genParseObj().parseName() == sibName;
            });

    if (iter == siblings.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
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

    auto fieldPrefix = "field_" + comms::genAccessName(genParseObj().parseName()) + "()";
    auto sibPrefix = "field_" + comms::genAccessName((*iter)->field().genParseObj().parseName()) + "()";
    util::ReplacementMap repl = {
        {"LEN_VALUE", (*iter)->commsValueAccessStr(accRest, sibPrefix)},
        {"LEN_FIELD", (*iter)->commsFieldAccessStr(accRest, sibPrefix)},
        {"DATA_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsDataField::commsDefValidFuncBodyImpl() const
{
    auto& validValues = genDataFieldParseObj().parseValidValues();
    if (validValues.empty()) {
        return std::string();
    }

    util::StringsList defs;
    util::StringsList values;
    for (auto idx = 0U; idx < validValues.size(); ++idx) {
        auto& info = validValues[idx];
        if (!genGenerator().genDoesElementExist(info.m_sinceVersion, info.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string DefTempl = 
            "static const std::uint8_t #^#NAME#$#[] = {\n"
            "    #^#BYTES#$#\n"
            "};\n";

        auto name = "ValidValue" + std::to_string(idx);
        util::ReplacementMap defRepl = {
            {"NAME", name},
            {"BYTES", bytesToString(info.m_value)},
        };

        defs.push_back(util::genProcessTemplate(DefTempl, defRepl));
        values.push_back("MapElem(std::begin(" + name + "), std::end(" + name + "))");
    }

    static const std::string Templ = 
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "#^#DEFS#$#\n"
        "using MapElem = comms::util::ArrayView<std::uint8_t>;\n"
        "static const MapElem Map[] = {\n"
        "    #^#VALUES#$#\n"
        "};\n"
        "\n"
        "auto& val = Base::getValue();\n"
        "auto iter =\n"
        "    std::lower_bound(\n"
        "        std::begin(Map), std::end(Map), val,\n"
        "        [](const MapElem& first, const typename Base::ValueType& second)\n"
        "        {\n"
        "            return std::lexicographical_compare(first.begin(), first.end(), second.begin(), second.end());\n"
        "        });\n"
        "\n"
        "if ((iter == std::end(Map)) || (iter->size() != val.size())) {\n"
        "    return false;\n"
        "}\n"
        "\n"
        "return std::equal(val.begin(), val.end(), iter->begin());\n"
        ;

    util::ReplacementMap repl = {
        {"DEFS", util::genStrListToString(defs, "\n", "")},
        {"VALUES", util::genStrListToString(values, ",\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool CommsDataField::commsIsLimitedCustomizableImpl() const
{
    return true;
}

std::string CommsDataField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::genEmptyString();
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
    auto obj = genDataFieldParseObj();
    auto fixedLength = obj.parseFixedLength();
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
    auto obj = genDataFieldParseObj();
    if (obj.parseFixedLength() != 0U) {
        return CommsBase::commsMaxLengthImpl();
    }

    return comms::genMaxPossibleLength();    
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

    return util::genStrListToString(opts, ",\n", "");
}

void CommsDataField::commsAddFixedLengthOptInternal(StringsList& opts) const
{
    auto obj = genDataFieldParseObj();
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

void CommsDataField::commsAddLengthPrefixOptInternal(StringsList& opts) const
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

        prefixName += "::" + comms::genClassName(m_commsMemberPrefixField->field().genName());
    }
    else {
        assert(m_commsExternalPrefixField != nullptr);
        prefixName = comms::genScopeFor(m_commsExternalPrefixField->field(), genGenerator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void CommsDataField::commsAddLengthForcingOptInternal(StringsList& opts) const
{
    auto obj = genDataFieldParseObj();
    auto& detachedPrefixName = obj.parseDetachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return;
    }

    opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
}

} // namespace commsdsl2comms
