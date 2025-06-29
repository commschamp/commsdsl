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

#include "CommsListField.h"

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

CommsListField::CommsListField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsListField::genPrepareImpl()
{
    bool result = Base::genPrepareImpl() && commsPrepare();
    if (result) {
        auto castField = 
            [](commsdsl::gen::GenField* field) -> CommsField*
            {
                if (field == nullptr) {
                    return nullptr;
                }

                auto commsField = dynamic_cast<CommsField*>(field);
                assert(commsField != nullptr);
                return commsField;
            };

        m_commsExternalElementField = castField(genExternalElementField());
        m_commsMemberElementField = castField(genMemberElementField());
        m_commsExternalCountPrefixField = castField(genExternalCountPrefixField());
        m_commsMemberCountPrefixField = castField(genMemberCountPrefixField());
        m_commsExternalLengthPrefixField = castField(genExternalLengthPrefixField());
        m_commsMemberLengthPrefixField = castField(genMemberLengthPrefixField());
        m_commsExternalElemLengthPrefixField = castField(genExternalElemLengthPrefixField());
        m_commsMemberElemLengthPrefixField = castField(genMemberElemLengthPrefixField());
        m_commsExternalTermSuffixField = castField(genExternalTermSuffixField());
        m_commsMemberTermSuffixField = castField(genMemberTermSuffixField());
    }

    if (!genListFieldParseObj().parseDetachedTermSuffixFieldName().empty()) {
        genGenerator().genLogger().genError(
            "Detached termination suffix is currently not supported, "
            "please contact the developer and request this feature");
        return false;
    }
    
    return result;
}

bool CommsListField::genWriteImpl() const
{
    return commsWrite();
}

CommsListField::IncludesList CommsListField::commsCommonIncludesImpl() const 
{
    IncludesList result;
    auto addIncludesFrom = 
        [&result](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            auto incsList = commsField->commsCommonIncludes();
            result.reserve(result.size() + incsList.size());
            std::move(incsList.begin(), incsList.end(), std::back_inserter(result));
        };

    addIncludesFrom(m_commsMemberElementField);
    addIncludesFrom(m_commsMemberCountPrefixField);
    addIncludesFrom(m_commsMemberLengthPrefixField);
    addIncludesFrom(m_commsMemberElemLengthPrefixField);
    addIncludesFrom(m_commsMemberTermSuffixField);
    return result;
}

std::string CommsListField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsListField::commsCommonMembersCodeImpl() const
{
    util::StringsList memberDefs;
    auto addMemberCode = 
        [&memberDefs](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            memberDefs.push_back(commsField->commsCommonCode());
        };

    addMemberCode(m_commsMemberElementField);
    addMemberCode(m_commsMemberCountPrefixField);
    addMemberCode(m_commsMemberLengthPrefixField);
    addMemberCode(m_commsMemberElemLengthPrefixField);
    addMemberCode(m_commsMemberTermSuffixField);
    return util::genStrListToString(memberDefs, "\n", "");
}

CommsListField::IncludesList CommsListField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/ArrayList.h",
    };

    auto& gen = genGenerator();
    auto addExternalFieldInclude = 
        [&result, &gen](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            result.push_back(comms::genRelHeaderPathFor(commsField->field(), gen));
        };

    addExternalFieldInclude(m_commsExternalElementField);        
    addExternalFieldInclude(m_commsExternalCountPrefixField);
    addExternalFieldInclude(m_commsExternalLengthPrefixField);
    addExternalFieldInclude(m_commsExternalElemLengthPrefixField);
    addExternalFieldInclude(m_commsExternalTermSuffixField);

    auto addIncludesFrom = 
        [&result](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            auto incsList = commsField->commsDefIncludes();
            result.reserve(result.size() + incsList.size());
            std::move(incsList.begin(), incsList.end(), std::back_inserter(result));
        };   

    addIncludesFrom(m_commsMemberElementField);
    addIncludesFrom(m_commsMemberCountPrefixField);
    addIncludesFrom(m_commsMemberLengthPrefixField);
    addIncludesFrom(m_commsMemberElemLengthPrefixField);
    addIncludesFrom(m_commsMemberTermSuffixField);

    auto obj = genListFieldParseObj();
    if ((!obj.parseDetachedCountPrefixFieldName().empty()) ||
        (!obj.parseDetachedLengthPrefixFieldName().empty()) ||
        (!obj.parseDetachedElemLengthPrefixFieldName().empty())) {
        result.insert(result.end(), {
            "<algorithm>",
            "<limits>"
        });
    }

    if (!obj.parseDetachedElemLengthPrefixFieldName().empty()) {
        result.push_back("comms/Assert.h");
    } 
    return result;
}

std::string CommsListField::commsDefMembersCodeImpl() const
{
    util::StringsList memberDefs;
    auto addMemberCode = 
        [&memberDefs](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            memberDefs.push_back(commsField->commsDefCode());
        };

    addMemberCode(m_commsMemberElementField);
    addMemberCode(m_commsMemberCountPrefixField);
    addMemberCode(m_commsMemberLengthPrefixField);
    addMemberCode(m_commsMemberElemLengthPrefixField);
    addMemberCode(m_commsMemberTermSuffixField);
    return util::genStrListToString(memberDefs, "\n", "");
}

std::string CommsListField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
    "comms::field::ArrayList<\n"
    "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "    #^#ELEMENT#$##^#COMMA#$#\n"
    "    #^#FIELD_OPTS#$#\n"
    ">";    

    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", genGenerator().genSchemaOf(*this).genMainNamespace()},
        {"ELEMENT", commsDefElementInternal()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsListField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genListFieldParseObj();
    util::StringsList preps;
    auto processPrefixFunc = 
        [this, &siblings, &preps](const std::string& prefixName, const util::ReplacementMap& replacements)
        {
            if (prefixName.empty()) {
                return;
            }

            auto sepPos = prefixName.find_first_of(".");
            std::string sibName = prefixName.substr(0, sepPos);
            std::string accRest;
            if (sepPos < prefixName.size()) {
                accRest = prefixName.substr(sepPos + 1);
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
                return;
            }

            auto repl = replacements;
            auto fieldPrefix = "field_" + repl["NAME"] + "()";
            auto sibPrefix = "field_" + comms::genAccessName((*iter)->field().genParseObj().parseName()) + "()";
            repl["LIST_FIELD"] = commsFieldAccessStr(std::string(), fieldPrefix);
            repl["ACC_VALUE"] = (*iter)->commsValueAccessStr(accRest, sibPrefix);
            auto conditions = commsCompOptChecks(std::string(), fieldPrefix);
            auto sibConditions = (*iter)->commsCompOptChecks(accRest, sibPrefix);
            
            if (!sibConditions.empty()) {
                std::move(sibConditions.begin(), sibConditions.end(), std::back_inserter(conditions));
            }

            if (conditions.empty()) {
                static const std::string Templ =
                    "#^#LIST_FIELD#$#.#^#FUNC#$#(\n"
                    "    static_cast<std::size_t>(#^#ACC_VALUE#$#));\n";
                
                preps.push_back(util::genProcessTemplate(Templ, repl));
                return;
            }

            static const std::string Templ = 
                "if (#^#COND#$#) {\n"
                "    #^#LIST_FIELD#$#.#^#FUNC#$#(\n"
                "        static_cast<std::size_t>(#^#ACC_VALUE#$#));\n"        
                "}";

            repl["COND"] = util::genStrListToString(conditions, " &&\n", "");
            preps.push_back(util::genProcessTemplate(Templ, repl));
            return;
        };

    util::ReplacementMap repl = {
        {"NAME", comms::genAccessName(genParseObj().parseName())}
    };
    
    auto& countPrefix = obj.parseDetachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        repl["FUNC"] = "forceReadElemCount";
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.parseDetachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        repl["FUNC"] = "forceReadLength";
        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.parseDetachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        repl["FUNC"] = "forceReadElemLength";
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (preps.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(preps, "\n", "");
}

std::string CommsListField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = genListFieldParseObj();

    util::StringsList refreshes;
    auto processPrefixFunc = 
        [&siblings, &refreshes](const std::string& prefixName, const util::ReplacementMap& replacements)
        {
            if (prefixName.empty()) {
                return;
            }

            auto sepPos = prefixName.find_first_of(".");
            std::string sibName = prefixName.substr(0, sepPos);
            std::string accRest;
            if (sepPos < prefixName.size()) {
                accRest = prefixName.substr(sepPos + 1);
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
                return;
            }

            static const std::string Templ = 
                "do {\n"
                "    auto expectedValue = static_cast<std::size_t>(#^#ACC_VALUE#$#);\n"
                "    #^#REAL_VALUE#$#\n"
                "    if (expectedValue == realValue) {\n"
                "        break;\n"
                "    }\n\n"
                "    using PrefixValueType = typename std::decay<decltype(#^#ACC_VALUE#$#)>::type;\n"
                "    static const auto MaxPrefixValue = static_cast<std::size_t>(std::numeric_limits<PrefixValueType>::max());\n"
                "    auto maxAllowedValue = std::min(MaxPrefixValue, realValue);\n"
                "    #^#ADJUST_LIST#$#\n"
                "    #^#ACC_FIELD#$#.setValue(#^#PREFIX_VALUE#$#);\n"
                "    updated = true;\n"
                "} while (false);\n";

            auto repl = replacements;
            auto sibPrefix = "field_" + comms::genAccessName((*iter)->field().genParseObj().parseName()) + "()";
            repl["ACC_VALUE"] = (*iter)->commsValueAccessStr(accRest, sibPrefix);
            repl["ACC_FIELD"] = (*iter)->commsFieldAccessStr(accRest, sibPrefix);
            
            refreshes.push_back(util::genProcessTemplate(Templ, repl));
        };

    auto fieldPrefix = "field_" + comms::genAccessName(genParseObj().parseName()) + "()";
    util::ReplacementMap repl = {
        {"NAME", comms::genAccessName(genParseObj().parseName())},
        {"LIST_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
    };

    auto& countPrefix = obj.parseDetachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        static const std::string RealValueTempl = 
            "auto realValue = #^#LIST_FIELD#$#.value().size();";
        repl["REAL_VALUE"] = util::genProcessTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "if (maxAllowedValue < realValue) {\n"
            "    #^#LIST_FIELD#$#.value().resize(maxAllowedValue);\n"
            "}";
        repl["PREFIX_VALUE"] = "maxAllowedValue";
        repl["ADJUST_LIST"] = util::genProcessTemplate(AdjustListTempl, repl);
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.parseDetachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        static const std::string RealValueTempl = 
            "auto realValue = #^#LIST_FIELD#$#.length();";
        repl["REAL_VALUE"] = util::genProcessTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "while (maxAllowedValue < realValue) {\n"
            "    auto elemLen = #^#LIST_FIELD#$#.value().back().length();\n"
            "    #^#LIST_FIELD#$#.value().pop_back();\n"
            "    realValue -= elemLen;"
            "}";
        
        repl["PREFIX_VALUE"] = "realValue";
        repl["ADJUST_LIST"] = util::genProcessTemplate(AdjustListTempl, repl);

        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.parseDetachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        static const std::string RealValueTempl = 
            "std::size_t realValue =\n"
            "    #^#LIST_FIELD#$#.value().empty() ?\n"
            "        0U : #^#LIST_FIELD#$#.value()[0].length();";
        repl["REAL_VALUE"] = util::genProcessTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "COMMS_ASSERT(\n"
            "    (#^#LIST_FIELD#$#.value().empty()) ||\n"
            "    (#^#LIST_FIELD#$#.value()[0].length() < maxAllowedValue));";
        repl["PREFIX_VALUE"] = "maxAllowedValue";
        repl["ADJUST_LIST"] = util::genProcessTemplate(AdjustListTempl, repl);
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (refreshes.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "bool updated = false;\n"
        "#^#UPDATES#$#\n"
        "return updated;\n";

    util::ReplacementMap finalRepl = {
        {"UPDATES", util::genStrListToString(refreshes, "\n", "")}
    };
    return util::genProcessTemplate(Templ, finalRepl);
}

bool CommsListField::commsIsLimitedCustomizableImpl() const
{
    return true;
}

bool CommsListField::commsIsVersionDependentImpl() const
{
    if ((m_commsMemberElementField != nullptr) && (m_commsMemberElementField->commsIsVersionDependent())) {
        return true;
    }

    if ((m_commsMemberCountPrefixField != nullptr) && (m_commsMemberCountPrefixField->commsIsVersionDependent())) {
        return true;
    }  

    if ((m_commsMemberLengthPrefixField != nullptr) && (m_commsMemberLengthPrefixField->commsIsVersionDependent())) {
        return true;
    }  

    if ((m_commsMemberElemLengthPrefixField != nullptr) && (m_commsMemberElemLengthPrefixField->commsIsVersionDependent())) {
        return true;
    }    

    if ((m_commsMemberTermSuffixField != nullptr) && (m_commsMemberTermSuffixField->commsIsVersionDependent())) {
        return true;
    }              

    return false;
}

std::string CommsListField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    util::StringsList elems;

    auto addStr = 
        [&elems](std::string&& str)
        {
            if (!str.empty()) {
                elems.push_back(std::move(str));
            }
        };

    assert(fieldOptsFunc != nullptr);
    if (m_commsMemberElementField != nullptr) {
        addStr((m_commsMemberElementField->*fieldOptsFunc)());
    }

    if (m_commsMemberCountPrefixField != nullptr) {
        addStr((m_commsMemberCountPrefixField->*fieldOptsFunc)());
    }

    if (m_commsMemberLengthPrefixField != nullptr) {
        addStr((m_commsMemberLengthPrefixField->*fieldOptsFunc)());
    }   

    if (m_commsMemberElemLengthPrefixField != nullptr) {
        addStr((m_commsMemberElemLengthPrefixField->*fieldOptsFunc)());
    }   

    if (m_commsMemberTermSuffixField != nullptr) {
        addStr((m_commsMemberTermSuffixField->*fieldOptsFunc)());
    }           

    return util::genStrListToString(elems, "\n", "");
}

CommsListField::StringsList CommsListField::commsExtraBareMetalDefaultOptionsImpl() const
{
    auto obj = genListFieldParseObj();
    auto fixedCount = obj.parseFixedCount();
    if (fixedCount != 0U) {
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

std::size_t CommsListField::commsMaxLengthImpl() const
{
    auto obj = genListFieldParseObj();
    if (obj.parseFixedCount() != 0U) {
        return CommsBase::commsMaxLengthImpl();
    }

    return comms::genMaxPossibleLength();
}

std::string CommsListField::commsSizeAccessStrImpl([[maybe_unused]] const std::string& accStr, const std::string& prefix) const
{
    assert(accStr.empty());
    return prefix + ".getValue().size()";
}

std::string CommsListField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddFixedLengthOptInternal(opts);
    commsAddCountPrefixOptInternal(opts);
    commsAddLengthPrefixOptInternal(opts);
    commsAddElemLengthPrefixOptInternal(opts);
    commsAddTermSuffixOptInternal(opts);
    commsAddLengthForcingOptInternal(opts);

    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsListField::commsDefElementInternal() const
{
    if (m_commsMemberElementField != nullptr) {
        auto str = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();

        if (comms::genIsGlobalField(*this)) {
            str += "<TOpt>";
        }

        str += "::";
        str += comms::genClassName(m_commsMemberElementField->field().genName());
        return str;
    }

    assert(m_commsExternalElementField != nullptr);
    return comms::genScopeFor(m_commsExternalElementField->field(), genGenerator()) + "<TOpt>";
}

void CommsListField::commsAddFixedLengthOptInternal(StringsList& opts) const
{
    auto obj = genListFieldParseObj();
    auto fixedCount = obj.parseFixedCount();
    if (fixedCount == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        util::genNumToString(static_cast<std::uintmax_t>(fixedCount)) +
        ">";
    opts.push_back(std::move(str));
}

void CommsListField::commsAddCountPrefixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalCountPrefixField == nullptr) && (m_commsMemberCountPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberCountPrefixField != nullptr) {
        prefixName = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();
        if (comms::genIsGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::genClassName(m_commsMemberCountPrefixField->field().genName());
    }
    else {
        assert(m_commsExternalCountPrefixField != nullptr);
        prefixName = comms::genScopeFor(m_commsExternalCountPrefixField->field(), genGenerator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSizeFieldPrefix<" + prefixName + '>');
}

void CommsListField::commsAddLengthPrefixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalLengthPrefixField == nullptr) && (m_commsMemberLengthPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberLengthPrefixField != nullptr) {
        prefixName = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();
        if (comms::genIsGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::genClassName(m_commsMemberLengthPrefixField->field().genName());
    }
    else {
        assert(m_commsExternalLengthPrefixField != nullptr);
        prefixName = comms::genScopeFor(m_commsExternalLengthPrefixField->field(), genGenerator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void CommsListField::commsAddElemLengthPrefixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalElemLengthPrefixField == nullptr) && (m_commsMemberElemLengthPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberElemLengthPrefixField != nullptr) {
        prefixName = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();
        if (comms::genIsGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::genClassName(m_commsMemberElemLengthPrefixField->field().genName());
    }
    else {
        assert(m_commsExternalElemLengthPrefixField != nullptr);
        prefixName = comms::genScopeFor(m_commsExternalElemLengthPrefixField->field(), genGenerator(), true, true);
        prefixName += "<TOpt> ";
    }

    std::string opt = "SequenceElemSerLengthFieldPrefix";
    if (genListFieldParseObj().parseElemFixedLength()) {
        opt = "SequenceElemFixedSerLengthFieldPrefix";
    }

    opts.push_back("comms::option::def::" + opt + "<" + prefixName + '>');    
}

void CommsListField::commsAddTermSuffixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalTermSuffixField == nullptr) && (m_commsMemberTermSuffixField == nullptr)) {
        return;
    }

    std::string suffixName;
    if (m_commsMemberTermSuffixField != nullptr) {
        suffixName = "typename " + comms::genClassName(genName()) + strings::genMembersSuffixStr();
        if (comms::genIsGlobalField(*this)) {
            suffixName += "<TOpt>";
        }

        suffixName += "::" + comms::genClassName(m_commsMemberTermSuffixField->field().genName());
    }
    else {
        assert(m_commsExternalTermSuffixField != nullptr);
        suffixName = comms::genScopeFor(m_commsExternalTermSuffixField->field(), genGenerator(), true, true);
        suffixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceTerminationFieldSuffix<" + suffixName + '>');
}

void CommsListField::commsAddLengthForcingOptInternal(StringsList& opts) const
{
    auto obj = genListFieldParseObj();
    if (!obj.parseDetachedCountPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceSizeForcingEnabled");
    }

    if (!obj.parseDetachedLengthPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
    }

    if (!obj.parseDetachedElemLengthPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceElemLengthForcingEnabled");
    }
}

} // namespace commsdsl2comms
