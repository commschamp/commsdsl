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

#include "CommsVariantField.h"

#include "CommsBundleField.h"
#include "CommsGenerator.h"
#include "CommsIntField.h"
#include "CommsRefField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <set>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

namespace 
{

constexpr std::size_t MaxMembersSupportedByComms = 120;    

bool intIsValidPropKeyInternal(const CommsIntField& intField)
{
    auto obj = intField.commsGenField().genParseObj();
    if (!obj.parseIsFailOnInvalid()) {
        return false;
    }

    if (obj.parseIsPseudo()) {
        return false;
    }

    auto genIntFieldParseObj = commsdsl::parse::ParseIntField(obj);
    auto& validRanges = genIntFieldParseObj.parseValidRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != genIntFieldParseObj.parseDefaultValue()) {
        return false;
    }

    return true; 
}    

const CommsField* bundleGetValidPropKeyInternal(const CommsBundleField& bundle)
{
    auto& members = bundle.commsMembers();
    if (members.empty()) {
        return nullptr;
    }

    auto& first = members.front();
    if (first->commsGenField().genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Int) {
        return nullptr;
    }

    auto& keyField = static_cast<const CommsIntField&>(*first);
    if (!intIsValidPropKeyInternal(keyField)) {
        return nullptr;
    }

    // Valid only if there is no non-default read
    if (keyField.commsHasGeneratedReadCode()) {
        return nullptr;
    }

    return first;
}

std::string propKeyTypeInternal(const CommsField& field)
{
    assert(field.commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Int);

    auto& keyField = static_cast<const CommsIntField&>(field);
    return keyField.commsVariantPropKeyType();
}

std::string propKeyValueStrInternal(const CommsField& field)
{
    assert(field.commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Int);

    auto& keyField = static_cast<const CommsIntField&>(field);
    return keyField.commsVariantPropKeyValueStr();
}

bool propKeysEquivalent(const CommsField& first, const CommsField& second)
{
    assert(first.commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Int);
    assert(second.commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Int);

    return static_cast<const CommsIntField&>(first).commsVariantIsPropKeyEquivalent(static_cast<const CommsIntField&>(second));
}

const CommsField* getReferenceFieldInternal(const CommsField* field)
{
    while (field->commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Ref) {
        auto& refField = static_cast<const CommsRefField&>(*field);
        field = dynamic_cast<decltype(field)>(refField.genReferencedField());
        assert(field != nullptr);
    }

    return field;
}

} // namespace 
    

CommsVariantField::CommsVariantField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField parseObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, parseObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsVariantField::genPrepareImpl()
{
    return 
        Base::genPrepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsVariantField::genWriteImpl() const
{
    return commsWrite();
}

CommsVariantField::CommsIncludesList CommsVariantField::commsCommonIncludesImpl() const
{
    CommsIncludesList result = {
        "<type_traits>",
        "<utility>",
    };

    for (auto* m : m_commsMembers) {
        assert(m != nullptr);
        auto incList = m->commsCommonIncludes();
        result.reserve(result.size() + incList.size());
        std::move(incList.begin(), incList.end(), std::back_inserter(result));
    } 
    return result;
}

std::string CommsVariantField::commsCommonCodeBodyImpl() const
{
    static const std::string Templ = 
        "#^#MEMBER_NAME_MAP_DEF#$#\n"
        "#^#NAME_FUNC#$#\n"
        "#^#MEMBER_NAMES_FUNCS#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"MEMBER_NAME_MAP_DEF", commsCommonMemberNameMapInternal()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"MEMBER_NAMES_FUNCS", commsCommonMemberNameFuncsCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsCommonMembersCodeImpl() const
{
    util::GenStringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsCommonCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }

    return util::genStrListToString(membersCode, "\n", "");
}

CommsVariantField::CommsIncludesList CommsVariantField::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/Assert.h",
        "comms/CompileControl.h",
        "comms/field/Variant.h",
        "<tuple>"        
    };
    
    for (auto* m : m_commsMembers) {
        assert(m != nullptr);
        auto incList = m->commsDefIncludes();
        result.reserve(result.size() + incList.size());
        std::move(incList.begin(), incList.end(), std::back_inserter(result));
    } 
    return result;
}

std::string CommsVariantField::commsDefMembersCodeImpl() const
{
    static const std::string Templ = 
        "#^#MEMBERS_DEFS#$#\n"
        "/// @brief All members bundled in @b std::tuple.\n"
        "using All =\n"
        "    std::tuple<\n"
        "       #^#MEMBERS#$#\n"
        "    >;";

    util::GenStringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsDefCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }
    
    util::GenStringsList names;
    for (auto& fPtr : genMembers()) {
        assert(fPtr);
        names.push_back(comms::genClassName(fPtr->genParseObj().parseName()));
    }

    util::GenReplacementMap repl = {
        {"MEMBERS_DEFS", util::genStrListToString(membersCode, "\n", "")},
        {"MEMBERS", util::genStrListToString(names, ",\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Variant<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = genGenerator();
    auto parseObj = genVariantFieldParseObj();
    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", gen.genSchemaOf(*this).genMainNamespace()},
        {"CLASS_NAME", comms::genClassName(parseObj.parseName())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (comms::genIsGlobalField(*this)) {
        repl["MEMBERS_OPT"] = "<TOpt>";
    }

    return util::genProcessTemplate(Templ, repl);       
}

std::string CommsVariantField::commsDefConstructCodeImpl() const
{
    auto obj = genVariantFieldParseObj();
    auto idx = obj.parseDefaultMemberIdx();
    if (m_commsMembers.size() <= idx) {
        return strings::genEmptyString();
    }

    assert(idx < m_commsMembers.size());
    static const std::string Templ = 
        "initField_#^#NAME#$#();\n";
    
    util::GenReplacementMap repl = {
        {"NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefDestructCodeImpl() const
{
    static const std::string Templ = 
        "reset();\n";

    return Templ;
}

std::string CommsVariantField::commsDefPublicCodeImpl() const
{
    return 
        commsDefMemberNamesTypesInternal() + '\n' +
        commsDefAccessCodeInternal() + '\n' + 
        commsDefCopyCodeInternal() + '\n' + 
        commsDefFieldExecCodeInternal() + '\n' +
        commsDefSelectFieldCodeInternal() + '\n' +
        commsDefResetCodeInternal() + '\n' +
        commsDefCanWriteCodeInternal() + '\n' + 
        commsDefMemberNamesFuncsInternal();
}

std::string CommsVariantField::commsDefPrivateCodeImpl() const
{
    static const std::string Templ = 
        "template <std::size_t TIdx, typename TField, typename TFunc>\n"
        "static void memFieldDispatch(TField&& f, TFunc&& func)\n"
        "{\n"
        "    #ifdef _MSC_VER\n"
        "        func.operator()<TIdx>(std::forward<TField>(f)); // VS compiler\n"
        "    #else // #ifdef _MSC_VER\n"
        "        func.template operator()<TIdx>(std::forward<TField>(f)); // All other compilers\n"
        "    #endif // #ifdef _MSC_VER\n"
        "}\n";
    return Templ;
}

std::string CommsVariantField::commsDefReadFuncBodyImpl() const
{
    if (m_optimizedReadKey.empty()) {
        return strings::genEmptyString();
    }

    std::string keyFieldType;
    GenStringsList cases;
    bool hasDefault = false;
    for (auto* memPtr : m_commsMembers) {
        auto* m = getReferenceFieldInternal(memPtr);
        assert(m->commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Bundle);
        auto& bundle = static_cast<const CommsBundleField&>(*m);
        auto& bundleMembers = bundle.commsMembers();
        assert(!bundleMembers.empty());
        auto* first = bundleMembers.front();
        assert(first->commsGenField().genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Int);
        auto& keyField = static_cast<const CommsIntField&>(*first);
        auto bundleAccName = comms::genAccessName(memPtr->commsGenField().genParseObj().parseName());
        auto keyAccName = comms::genAccessName(keyField.commsGenField().genParseObj().parseName());

        if ((memPtr != m_commsMembers.back()) ||
            (keyField.commsVariantIsValidPropKey())) {
            auto valStr = keyField.commsVariantPropKeyValueStr();

            static const std::string Templ =
                "case #^#VAL#$#:\n"
                "    {\n"
                "        auto& field_#^#BUNDLE_NAME#$# = initField_#^#BUNDLE_NAME#$#();\n"
                "        COMMS_ASSERT(field_#^#BUNDLE_NAME#$#.field_#^#KEY_NAME#$#().getValue() == commonKeyField.getValue());\n"
                "        #^#VERSION_ASSIGN#$#\n"
                "        return field_#^#BUNDLE_NAME#$#.template readFrom<1>(iter, len);\n"
                "    }";

            util::GenReplacementMap repl = {
                {"VAL", std::move(valStr)},
                {"BUNDLE_NAME", bundleAccName},
                {"KEY_NAME", keyAccName},
            };

            if (m->commsIsVersionDependent()) {
                auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
                repl["VERSION_ASSIGN"] = std::move(assignStr);
            }
            cases.push_back(util::genProcessTemplate(Templ, repl));
            continue;
        }

        // Last "catch all" element
        assert(memPtr == m_commsMembers.back());

        static const std::string Templ =
            "default:\n"
            "    initField_#^#BUNDLE_NAME#$#().field_#^#KEY_NAME#$#().setValue(commonKeyField.getValue());\n"
            "    #^#VERSION_ASSIGN#$#\n"
            "    return accessField_#^#BUNDLE_NAME#$#().template readFrom<1>(iter, len);";

        util::GenReplacementMap repl = {
            {"BUNDLE_NAME", bundleAccName},
            {"KEY_NAME", keyAccName},
        };

        if (m->commsIsVersionDependent()) {
            auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
            repl["VERSION_ASSIGN"] = std::move(assignStr);
        }

        cases.push_back(util::genProcessTemplate(Templ, repl));
        hasDefault = true;
    }

    if (!hasDefault) {
        static const std::string DefaultBreakStr =
            "default:\n"
            "    break;";
        cases.push_back(DefaultBreakStr);
    }

    static const std::string Templ =
        "reset();\n"
        "#^#VERSION_DEP#$#\n"
        "using CommonKeyField=\n"
        "    #^#KEY_FIELD_TYPE#$#;\n"
        "CommonKeyField commonKeyField;\n\n"
        "auto origIter = iter;\n"
        "auto es = commonKeyField.read(iter, len);\n"
        "if (es != comms::ErrorStatus::Success) {\n"
        "    return es;\n"
        "}\n\n"
        "auto consumedLen = static_cast<std::size_t>(std::distance(origIter, iter));\n"
        "COMMS_ASSERT(consumedLen <= len);\n"
        "len -= consumedLen;\n\n"
        "switch (commonKeyField.getValue()) {\n"
        "    #^#CASES#$#\n"
        "};\n\n"
        "return comms::ErrorStatus::InvalidMsgData;\n";

    util::GenReplacementMap repl = {
        {"KEY_FIELD_TYPE", m_optimizedReadKey},
        {"CASES", util::genStrListToString(cases, "\n", "")},
    };

    if (commsIsVersionDependent()) {
        static const std::string CheckStr =
            "static_assert(Base::isVersionDependent(), \"The field must be recognised as version dependent\");";
        repl["VERSION_DEP"] = CheckStr;
    }

    return util::genProcessTemplate(Templ, repl);    
}

CommsVariantField::GenStringsList CommsVariantField::commsDefReadMsvcSuppressWarningsImpl() const
{
    return GenStringsList{"4702"};
}

std::string CommsVariantField::commsDefWriteFuncBodyImpl() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().write(iter, len);";
            
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return comms::ErrorStatus::Success;\n"
        ;

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefRefreshFuncBodyImpl() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().refresh();";
            
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return false;\n"
        ;

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefLengthFuncBodyImpl() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().length();";
            
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return 0U;\n"
        ;

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefValidFuncBodyImpl() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().valid();";
            
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return false;\n"
        ;

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

bool CommsVariantField::commsIsVersionDependentImpl() const
{
    return 
        std::any_of(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

std::string CommsVariantField::commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const
{
    assert(fieldOptsFunc != nullptr);
    util::GenStringsList elems;
    for (auto* m : m_commsMembers) {
        auto str = (m->*fieldOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }
    return util::genStrListToString(elems, "\n", "");
}

std::size_t CommsVariantField::commsMaxLengthImpl() const 
{
    return
        std::accumulate(
            m_commsMembers.begin(), m_commsMembers.end(), std::size_t(0),
            [](std::size_t soFar, auto* m)
            {
                return std::max(soFar, m->commsMaxLength());
            });
}

bool CommsVariantField::commsHasCustomLengthDeepImpl() const
{
    return 
        std::any_of(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsHasCustomLength(true);
            });
}

bool CommsVariantField::commsMustDefineDefaultConstructorImpl() const
{
    return true;
}

bool CommsVariantField::commsPrepareInternal()
{
    m_commsMembers = commsTransformFieldsList(genMembers());
    if (genGenerator().genSchemaOf(*this).genVersionDependentCode()) {
        auto sinceVersion = genParseObj().parseSinceVersion();
        for (auto* m : m_commsMembers) {
            assert(m != nullptr);
            if (sinceVersion < m->commsGenField().genParseObj().parseSinceVersion()) {
                genGenerator().genLogger().genError("Currently version dependent members of variant are not supported!");
                return false;
            }
        }
    }

    m_optimizedReadKey = commsOptimizedReadKeyInternal();
    return true;
}

std::string CommsVariantField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::GenStringsList opts;
    commsAddFieldDefOptions(opts);
    commsAddCustomReadOptInternal(opts);
    util::genAddToStrList("comms::option::def::HasCustomWrite", opts);
    util::genAddToStrList("comms::option::def::HasCustomRefresh", opts);        
    util::genAddToStrList("comms::option::def::VariantHasCustomResetOnDestruct", opts);        
    util::genAddToStrList("comms::option::def::HasVersionDependentMembers<" + util::genBoolToString(commsIsVersionDependentImpl()) + ">", opts);        
    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsVariantField::commsDefAccessCodeInternal() const
{
    if (m_commsMembers.size() <= MaxMembersSupportedByComms) {
        return commsDefAccessCodeByCommsInternal();
    }

    return commsDefAccessCodeGeneratedInternal();
}

std::string CommsVariantField::commsDefCopyCodeInternal() const
{
    GenStringsList copyCases;
    GenStringsList moveCases;
    GenStringsList eqCases;
    GenStringsList ltCases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string CopyTempl =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#() = other.accessField_#^#MEM_NAME#$#(); return *this;";

        static const std::string MoveTempl =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#() = std::move(other.accessField_#^#MEM_NAME#$#()); return *this;";

        static const std::string EqTempl =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#() == other.accessField_#^#MEM_NAME#$#();";

        static const std::string LtTempl =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#() < other.accessField_#^#MEM_NAME#$#();";

        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };

        copyCases.push_back(util::genProcessTemplate(CopyTempl, repl));
        moveCases.push_back(util::genProcessTemplate(MoveTempl, repl));
        eqCases.push_back(util::genProcessTemplate(EqTempl, repl));
        ltCases.push_back(util::genProcessTemplate(LtTempl, repl));
    }

    static const std::string Templ = 
        "/// @brief Copy constructor.\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#(const #^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& other) :\n"
        "    Base()\n"
        "{\n"
        "    *this = other;\n"
        "}\n\n"
        "/// @brief Move constructor.\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#(#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#&& other) : \n"
        "    Base()\n"
        "{\n"
        "    *this = std::move(other);\n"
        "}\n\n"
        "/// @brief Copy assignment operator.\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& operator=(const #^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& other)\n"
        "{\n"
        "    if (this == &other) {\n"
        "        return *this;\n"
        "    }\n\n"
        "    reset();\n\n"
        "    if (!other.currentFieldValid()) {\n"
        "        return *this;\n"
        "    }\n\n"        
        "    switch (other.currentField()) {\n"
        "        #^#COPY_CASES#$#\n"
        "        default: break;\n"
        "    }\n\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "    return *this;\n"
        "}\n\n"
        "/// @brief Move assignement operator.\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& operator=(#^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#&& other)\n"
        "{"
        "    if (this == &other) {\n"
        "        return *this;\n"
        "    }\n\n"
        "    reset();\n\n"        
        "    if (!other.currentFieldValid()) {\n"
        "        return *this;\n"
        "    }\n\n"
        "    switch (other.currentField()) {\n"
        "        #^#MOVE_CASES#$#\n"
        "        default: break;\n"
        "    }\n\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "    return *this;\n"
        "}\n\n"
        "/// @brief Equality comparison operator.\n"
        "bool operator==(const #^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& other) const\n"
        "{\n"
        "    if (this == &other) {\n"
        "        return true;\n"
        "    }\n\n"
        "    if (Base::currentFieldValid() != other.currentFieldValid()) {\n"
        "        return false;\n"
        "    }\n\n"
        "    if (!Base::currentFieldValid()) {\n\n"
        "        return true;\n"
        "    }\n\n"
        "    if (Base::currentField() != other.currentField()) {\n"
        "        return false;\n"
        "    }\n\n"
        "    switch(Base::currentField()) {\n"
        "        #^#EQ_CASES#$#\n"
        "        default: break;\n"
        "    }\n\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "    return false;\n"
        "}\n\n" 
        "/// @brief Inequality comparison operator.\n" 
        "bool operator!=(const #^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& other) const\n"
        "{\n" 
        "    return !(*this == other);\n"        
        "}\n\n"
        "/// @brief Order comparison operator.\n" 
        "bool operator<(const #^#CLASS_NAME#$##^#SUFFIX#$##^#ORIG#$#& other) const\n"
        "{\n" 
        "    if (!Base::currentFieldValid()) {\n"
        "        return (!other.currentFieldValid());\n"
        "    }\n\n"
        "    if (!other.currentFieldValid()) {\n"
        "        return false;\n"
        "    }\n\n"    
        "    if (Base::currentField() < other.currentField()) {\n"
        "        return true;\n"
        "    }\n\n"
        "    if (Base::currentField() != other.currentField()) {\n"
        "        return false;\n\n"
        "    }\n\n"
        "    if (this == &other) {\n"
        "        return false;\n"
        "    }\n"
        "    switch(Base::currentField()) {\n"
        "        #^#LT_CASES#$#\n"
        "        default: break;\n"
        "    }\n\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "    return false;\n"
        "}\n"            
        ;


    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"COPY_CASES", util::genStrListToString(copyCases, "\n", "")},
        {"MOVE_CASES", util::genStrListToString(moveCases, "\n", "")},
        {"EQ_CASES", util::genStrListToString(eqCases, "\n", "")},
        {"LT_CASES", util::genStrListToString(ltCases, "\n", "")},
    };

    if (commsIsExtended()) {
        repl["ORIG"] = strings::genOrigSuffixStr();
    }

    if (commsIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }    

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefAccessCodeByCommsInternal() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_VARIANT_MEMBERS_NAMES macro\n"
        "///     related to @b comms::field::Variant class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated values, types and access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_VARIANT_MEMBERS_NAMES(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    util::GenStringsList accessDocList;
    util::GenStringsList namesList;
    accessDocList.reserve(m_commsMembers.size());
    namesList.reserve(m_commsMembers.size());

    auto& gen = genGenerator();
    for (auto& mPtr : genMembers()) {
        namesList.push_back(comms::genAccessName(mPtr->genParseObj().parseName()));
        std::string accessStr =
            "///     @li @b FieldIdx_" +  namesList.back() + 
            " index, @b Field_" + namesList.back() + " type,\n"
            "///         @b initField_" + namesList.back() + "(), @b deinitField_" + namesList.back() +
            "() and @b accessField_" + namesList.back() + "() access functions -\n"
            "///         for " + comms::genScopeFor(*mPtr, gen) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    util::GenReplacementMap repl = {
        {"ACCESS_DOC", util::genStrListToString(accessDocList, "\n", "")},
        {"NAMES", util::genStrListToString(namesList, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefAccessCodeGeneratedInternal() const
{
    GenStringsList indicesList;
    GenStringsList accessList;
    indicesList.reserve(m_commsMembers.size());
    accessList.reserve(m_commsMembers.size());

    auto membersPrefix = comms::genClassName(genParseObj().parseName()) + strings::genMembersSuffixStr();
    auto docScope = membersPrefix + "::";
    auto typeScope = "typename " + membersPrefix + "<TOpt>::";
    for (auto& m : m_commsMembers) {
        auto accName = comms::genAccessName(m->commsGenField().genParseObj().parseName());
        auto className = comms::genClassName(m->commsGenField().genParseObj().parseName());

        indicesList.push_back("FieldIdx_" + accName);

        static const std::string AccTempl =
            "/// @brief Member type alias to #^#DOC_SCOPE#$#.\n"
            "using Field_#^#NAME#$# = #^#TYPE_SCOPE#$#;\n\n"
            "/// @brief Initialize as #^#DOC_SCOPE#$#\n"
            "Field_#^#NAME#$#& initField_#^#NAME#$#()\n"
            "{\n"
            "    return Base::template initField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n"    
           "/// @brief De-initialize #^#DOC_SCOPE#$#\n"
            "void deinitField_#^#NAME#$#()\n"
            "{\n"
            "    Base::template deinitField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n"                
            "/// @brief Access as #^#DOC_SCOPE#$#\n"
            "Field_#^#NAME#$#& accessField_#^#NAME#$#()\n"
            "{\n"
            "    return Base::template accessField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n"
            "/// @brief Access as #^#DOC_SCOPE#$# (const version)\n"
            "const Field_#^#NAME#$#& accessField_#^#NAME#$#() const\n"
            "{\n"
            "    return Base::template accessField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n";   

        util::GenReplacementMap accRepl = {
            {"DOC_SCOPE", docScope + className},
            {"TYPE_SCOPE", typeScope + className},
            {"NAME", accName}
        };
        accessList.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "/// @brief Allow access to internal fields by index.\n"
        "enum FieldIdx : unsigned\n"
        "{\n"
        "    #^#INDICES#$#,\n"
        "    FieldIdx_numOfValues"
        "};\n\n"
        "#^#ACCESS#$#\n";    

    util::GenReplacementMap repl = {
        {"INDICES", util::genStrListToString(indicesList, ",\n", "")},
        {"ACCESS", util::genStrListToString(accessList, "\n", "")},
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefFieldExecCodeInternal() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#:\n"
            "    memFieldDispatch<FieldIdx_#^#MEM_NAME#$#>(accessField_#^#MEM_NAME#$#(), std::forward<TFunc>(func));\n"
            "    break;";
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "/// @brief Optimized currentFieldExec functionality#^#VARIANT#$#.\n"
        "/// @details Replaces the currentFieldExec() member function defined\n"
        "///    by @b comms::field::Variant.\n"
        "template <typename TFunc>\n"
        "void currentFieldExec(TFunc&& func) #^#CONST#$#\n"
        "{\n"
        "    switch (Base::currentField()) {\n"
        "        #^#CASES#$#\n"
        "        default:\n"
        "            static constexpr bool Invalid_field_execution = false;\n"
        "            static_cast<void>(Invalid_field_execution);\n"
        "            COMMS_ASSERT(Invalid_field_execution);\n"
        "            break;\n"
        "    }\n"
        "}\n\n"
        "/// @brief The same as currentFieldExec() #^#VARIANT#$#\n"
        "/// @details Generated for backward comatibility, can be removed in the future.\n"
        "template <typename TFunc>\n"
        "void currFieldExec(TFunc&& func) #^#CONST#$#\n"
        "{\n"
        "    currentFieldExec(std::forward<TFunc>(func));\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };

    auto str = util::genProcessTemplate(Templ, repl);
    str += "\n";

    repl.insert({
        {"VARIANT", " (const variant)"},
        {"CONST", "const"}
    });
    str += util::genProcessTemplate(Templ, repl);
    return str;
}

std::string CommsVariantField::commsDefResetCodeInternal() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: deinitField_#^#MEM_NAME#$#(); return;";
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "/// @brief Optimized reset functionality.\n"
        "/// @details Replaces the reset() member function defined\n"
        "///    by @b comms::field::Variant.\n"
        "void reset()\n"
        "{\n"
        "    if (!Base::currentFieldValid()) {\n"
        "        return;\n"
        "    }\n\n"
        "    switch (Base::currentField()) {\n"
        "        #^#CASES#$#\n"
        "        default: break;\n"
        "    }\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "}\n";

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefCanWriteCodeInternal() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().canWrite();";
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "/// @brief Optimized check ability to write.\n"
        "/// @details Replaces the canWrite() member function defined\n"
        "///    by @b comms::field::Variant.\n"
        "bool canWrite() const\n"
        "{\n"
        "    if (!Base::currentFieldValid()) {\n"
        "        return true;\n"
        "    }\n\n"
        "    switch (Base::currentField()) {\n"
        "        #^#CASES#$#\n"
        "        default: break;\n"
        "    }\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "    return false;\n"
        "}\n";

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefSelectFieldCodeInternal() const
{
    GenStringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#(); return;";
        util::GenReplacementMap repl = {
            {"MEM_NAME", comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName())}
        };
        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "/// @brief Optimized runtime selection field functionality.\n"
        "/// @details Replaces the selectField() member function defined\n"
        "///    by @b comms::field::Variant.\n"
        "void selectField(std::size_t idx)\n"
        "{\n"
        "    if (Base::currentField() == idx) {\n"
        "        return;\n"
        "    }\n\n"
        "    reset();\n"
        "    switch (idx) {\n"
        "        #^#CASES#$#\n"
        "        default: break;\n"
        "    }\n"
        "    COMMS_ASSERT(false); // Should not be reached\n"
        "}\n";

    util::GenReplacementMap repl = {
        {"CASES", util::genStrListToString(cases, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}


void CommsVariantField::commsAddCustomReadOptInternal(GenStringsList& opts) const
{
    if (!m_optimizedReadKey.empty()) {
        util::genAddToStrList("comms::option::def::HasCustomRead", opts);
    }
}

std::string CommsVariantField::commsOptimizedReadKeyInternal() const
{
    std::string result;
    if (m_commsMembers.size() <= 1U) {
        return result;
    }

    const CommsField* propKey = nullptr;
    std::set<std::string> keyValues;

    for (auto* m : m_commsMembers) {
        const auto* memPtr = getReferenceFieldInternal(m);
        if (memPtr->commsGenField().genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Bundle) {
            return result;
        }

        auto& bundle = static_cast<const CommsBundleField&>(*memPtr);
        auto* propKeyTmp = bundleGetValidPropKeyInternal(bundle);
        bool validPropKey = (propKeyTmp != nullptr);
        if ((!validPropKey) && (m != m_commsMembers.back())) {
            return result;
        }

        if (!validPropKey) {
            // last "catch all" element
            continue;
        }

        assert(propKeyTmp != nullptr);
        auto insertResult = keyValues.insert(propKeyValueStrInternal(*propKeyTmp));
        if (!insertResult.second) {
            // The same key value has been inserted
            return result;
        }

        if (propKey == nullptr) {
            propKey = propKeyTmp;
            continue;
        }

        if (!propKeysEquivalent(*propKey, *propKeyTmp)) {
            return result;
        }
    }

    result = propKeyTypeInternal(*propKey);
    return result;
}

const std::string& CommsVariantField::commsCommonMemberNameMapInternal() const
{
    static const std::string Templ = 
        "/// @brief Single member name info entry\n"
        "using MemberNameInfo = const char*;\n\n"
        "/// @brief Type returned from @ref memberNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using MemberNamesMapInfo = std::pair<const MemberNameInfo*, std::size_t>;\n";

    return Templ;
}

std::string CommsVariantField::commsCommonMemberNameFuncsCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Retrieve name of the member\n"
        "static const char* memberName(std::size_t idx)\n"
        "{\n"
        "    auto namesMapInfo = memberNamesMap();\n"
        "    if (namesMapInfo.second <= idx) {\n"
        "        return nullptr;\n"
        "    }\n\n"
        "    return namesMapInfo.first[idx];\n"
        "}\n\n"
        "/// @brief Retrieve map of members names\n"
        "static MemberNamesMapInfo memberNamesMap()\n"
        "{\n"
        "    static const MemberNameInfo Map[] = {\n"
        "        #^#MEMBERS#$#\n"
        "    };\n"
        "    static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "    return std::make_pair(&Map[0], MapSize);\n"
        "}\n";

    auto membersPrefix = comms::genClassName(genParseObj().parseName()) + strings::genMembersSuffixStr() + strings::genCommonSuffixStr();
    util::GenStringsList names;
    for (auto& fPtr : genMembers()) {
        assert(fPtr);
        names.push_back(membersPrefix + "::" + comms::genClassName(fPtr->genParseObj().parseName()) + strings::genCommonSuffixStr() + "::name()");
    }

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(names, ",\n", "")}
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefMemberNamesTypesInternal() const
{
    static const std::string Templ = 
        "/// @brief Single member name info entry\n"
        "using MemberNameInfo = #^#COMMON_SCOPE#$#::MemberNameInfo;\n\n"
        "/// @brief Type returned from @ref memberNamesMap() member function.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::MemberNamesMapInfo.\n"
        "using MemberNamesMapInfo = #^#COMMON_SCOPE#$#::MemberNamesMapInfo;\n";

    util::GenReplacementMap repl = {
        {"COMMON_SCOPE", comms::genCommonScopeFor(*this, genGenerator())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefMemberNamesFuncsInternal() const
{
    static const std::string Templ = 
        "/// @brief Retrieve name of the member\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::memberName().\n"
        "static const char* memberName(std::size_t idx)\n"
        "{\n"
        "    return #^#COMMON_SCOPE#$#::memberName(idx);\n"
        "}\n\n"
        "/// @brief Retrieve name of the member\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::memberName().\n"
        "static const char* memberName(FieldIdx idx)\n"
        "{\n"
        "    return memberName(static_cast<std::size_t>(idx));\n"
        "}\n\n"
        "/// @brief Retrieve map of members names\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::memberNamesMap().\n"
        "static MemberNamesMapInfo memberNamesMap()\n"
        "{\n"
        "    return #^#COMMON_SCOPE#$#::memberNamesMap();\n"
        "}\n";    


    util::GenReplacementMap repl = {
        {"COMMON_SCOPE", comms::genCommonScopeFor(*this, genGenerator())}
    };
    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2comms
