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
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsListField::prepareImpl()
{
    bool result = Base::prepareImpl() && commsPrepare();
    if (result) {
        auto castField = 
            [](Field* field) -> CommsField*
            {
                if (field == nullptr) {
                    return nullptr;
                }

                auto commsField = dynamic_cast<CommsField*>(field);
                assert(commsField != nullptr);
                return commsField;
            };

        m_commsExternalElementField = castField(externalElementField());
        m_commsMemberElementField = castField(memberElementField());
        m_commsExternalCountPrefixField = castField(externalCountPrefixField());
        m_commsMemberCountPrefixField = castField(memberCountPrefixField());
        m_commsExternalLengthPrefixField = castField(externalLengthPrefixField());
        m_commsMemberLengthPrefixField = castField(memberLengthPrefixField());
        m_commsExternalElemLengthPrefixField = castField(externalElemLengthPrefixField());
        m_commsMemberElemLengthPrefixField = castField(memberElemLengthPrefixField());
        m_commsExternalTermSuffixField = castField(externalTermSuffixField());
        m_commsMemberTermSuffixField = castField(memberTermSuffixField());
    }

    if (!listDslObj().detachedTermSuffixFieldName().empty()) {
        generator().logger().error(
            "Detached termination suffix is currently not supported, "
            "please contact the developer and request this feature");
        return false;
    }
    
    return result;
}

bool CommsListField::writeImpl() const
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
    return util::strListToString(memberDefs, "\n", "");
}

CommsListField::IncludesList CommsListField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/ArrayList.h",
    };

    auto& gen = generator();
    auto addExternalFieldInclude = 
        [&result, &gen](const CommsField* commsField)
        {
            if (commsField == nullptr) {
                return;
            }

            result.push_back(comms::relHeaderPathFor(commsField->field(), gen));
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

    auto obj = listDslObj();
    if ((!obj.detachedCountPrefixFieldName().empty()) ||
        (!obj.detachedLengthPrefixFieldName().empty()) ||
        (!obj.detachedElemLengthPrefixFieldName().empty())) {
        result.insert(result.end(), {
            "<algorithm>",
            "<limits>"
        });
    }

    if (!obj.detachedElemLengthPrefixFieldName().empty()) {
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
    return util::strListToString(memberDefs, "\n", "");
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
        {"PROT_NAMESPACE", generator().schemaOf(*this).mainNamespace()},
        {"ELEMENT", commsDefElementInternal()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsListField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = listDslObj();
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
                        return f->field().dslObj().name() == sibName;
                    });

            if (iter == siblings.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                return;
            }

            auto repl = replacements;
            auto fieldPrefix = "field_" + repl["NAME"] + "()";
            auto sibPrefix = "field_" + comms::accessName((*iter)->field().dslObj().name()) + "()";
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
                
                preps.push_back(util::processTemplate(Templ, repl));
                return;
            }

            static const std::string Templ = 
                "if (#^#COND#$#) {\n"
                "    #^#LIST_FIELD#$#.#^#FUNC#$#(\n"
                "        static_cast<std::size_t>(#^#ACC_VALUE#$#));\n"        
                "}";

            repl["COND"] = util::strListToString(conditions, " &&\n", "");
            preps.push_back(util::processTemplate(Templ, repl));
            return;
        };

    util::ReplacementMap repl = {
        {"NAME", comms::accessName(dslObj().name())}
    };
    
    auto& countPrefix = obj.detachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        repl["FUNC"] = "forceReadElemCount";
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.detachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        repl["FUNC"] = "forceReadLength";
        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.detachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        repl["FUNC"] = "forceReadElemLength";
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (preps.empty()) {
        return strings::emptyString();
    }

    return util::strListToString(preps, "\n", "");
}

std::string CommsListField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = listDslObj();

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
                        return f->field().dslObj().name() == sibName;
                    });

            if (iter == siblings.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
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
            auto sibPrefix = "field_" + comms::accessName((*iter)->field().dslObj().name()) + "()";
            repl["ACC_VALUE"] = (*iter)->commsValueAccessStr(accRest, sibPrefix);
            repl["ACC_FIELD"] = (*iter)->commsFieldAccessStr(accRest, sibPrefix);
            
            refreshes.push_back(util::processTemplate(Templ, repl));
        };

    auto fieldPrefix = "field_" + comms::accessName(dslObj().name()) + "()";
    util::ReplacementMap repl = {
        {"NAME", comms::accessName(dslObj().name())},
        {"LIST_FIELD", commsFieldAccessStr(std::string(), fieldPrefix)},
    };

    auto& countPrefix = obj.detachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        static const std::string RealValueTempl = 
            "auto realValue = #^#LIST_FIELD#$#.value().size();";
        repl["REAL_VALUE"] = util::processTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "if (maxAllowedValue < realValue) {\n"
            "    #^#LIST_FIELD#$#.value().resize(maxAllowedValue);\n"
            "}";
        repl["PREFIX_VALUE"] = "maxAllowedValue";
        repl["ADJUST_LIST"] = util::processTemplate(AdjustListTempl, repl);
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.detachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        static const std::string RealValueTempl = 
            "auto realValue = #^#LIST_FIELD#$#.length();";
        repl["REAL_VALUE"] = util::processTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "while (maxAllowedValue < realValue) {\n"
            "    auto elemLen = #^#LIST_FIELD#$#.value().back().length();\n"
            "    #^#LIST_FIELD#$#.value().pop_back();\n"
            "    realValue -= elemLen;"
            "}";
        
        repl["PREFIX_VALUE"] = "realValue";
        repl["ADJUST_LIST"] = util::processTemplate(AdjustListTempl, repl);

        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.detachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        static const std::string RealValueTempl = 
            "std::size_t realValue =\n"
            "    #^#LIST_FIELD#$#.value().empty() ?\n"
            "        0U : #^#LIST_FIELD#$#.value()[0].length();";
        repl["REAL_VALUE"] = util::processTemplate(RealValueTempl, repl);

        static const std::string AdjustListTempl = 
            "COMMS_ASSERT(\n"
            "    (#^#LIST_FIELD#$#.value().empty()) ||\n"
            "    (#^#LIST_FIELD#$#.value()[0].length() < maxAllowedValue));";
        repl["PREFIX_VALUE"] = "maxAllowedValue";
        repl["ADJUST_LIST"] = util::processTemplate(AdjustListTempl, repl);
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (refreshes.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "bool updated = false;\n"
        "#^#UPDATES#$#\n"
        "return updated;\n";

    util::ReplacementMap finalRepl = {
        {"UPDATES", util::strListToString(refreshes, "\n", "")}
    };
    return util::processTemplate(Templ, finalRepl);
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

    return util::strListToString(elems, "\n", "");
}

CommsListField::StringsList CommsListField::commsExtraBareMetalDefaultOptionsImpl() const
{
    auto obj = listDslObj();
    auto fixedCount = obj.fixedCount();
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
    auto obj = listDslObj();
    if (obj.fixedCount() != 0U) {
        return CommsBase::commsMaxLengthImpl();
    }

    return comms::maxPossibleLength();
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

    return util::strListToString(opts, ",\n", "");
}

std::string CommsListField::commsDefElementInternal() const
{
    if (m_commsMemberElementField != nullptr) {
        auto str = "typename " + comms::className(name()) + strings::membersSuffixStr();

        if (comms::isGlobalField(*this)) {
            str += "<TOpt>";
        }

        str += "::";
        str += comms::className(m_commsMemberElementField->field().name());
        return str;
    }

    assert(m_commsExternalElementField != nullptr);
    return comms::scopeFor(m_commsExternalElementField->field(), generator()) + "<TOpt>";
}

void CommsListField::commsAddFixedLengthOptInternal(StringsList& opts) const
{
    auto obj = listDslObj();
    auto fixedCount = obj.fixedCount();
    if (fixedCount == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        util::numToString(static_cast<std::uintmax_t>(fixedCount)) +
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
        prefixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::className(m_commsMemberCountPrefixField->field().name());
    }
    else {
        assert(m_commsExternalCountPrefixField != nullptr);
        prefixName = comms::scopeFor(m_commsExternalCountPrefixField->field(), generator(), true, true);
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
        prefixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::className(m_commsMemberLengthPrefixField->field().name());
    }
    else {
        assert(m_commsExternalLengthPrefixField != nullptr);
        prefixName = comms::scopeFor(m_commsExternalLengthPrefixField->field(), generator(), true, true);
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
        prefixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::className(m_commsMemberElemLengthPrefixField->field().name());
    }
    else {
        assert(m_commsExternalElemLengthPrefixField != nullptr);
        prefixName = comms::scopeFor(m_commsExternalElemLengthPrefixField->field(), generator(), true, true);
        prefixName += "<TOpt> ";
    }

    std::string opt = "SequenceElemSerLengthFieldPrefix";
    if (listDslObj().elemFixedLength()) {
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
        suffixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            suffixName += "<TOpt>";
        }

        suffixName += "::" + comms::className(m_commsMemberTermSuffixField->field().name());
    }
    else {
        assert(m_commsExternalTermSuffixField != nullptr);
        suffixName = comms::scopeFor(m_commsExternalTermSuffixField->field(), generator(), true, true);
        suffixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceTerminationFieldSuffix<" + suffixName + '>');
}

void CommsListField::commsAddLengthForcingOptInternal(StringsList& opts) const
{
    auto obj = listDslObj();
    if (!obj.detachedCountPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceSizeForcingEnabled");
    }

    if (!obj.detachedLengthPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
    }

    if (!obj.detachedElemLengthPrefixFieldName().empty()) {
        opts.push_back("comms::option::def::SequenceElemLengthForcingEnabled");
    }
}

} // namespace commsdsl2comms
