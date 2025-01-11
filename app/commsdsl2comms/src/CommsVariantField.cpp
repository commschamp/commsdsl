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
    auto obj = intField.field().dslObj();
    if (!obj.isFailOnInvalid()) {
        return false;
    }

    if (obj.isPseudo()) {
        return false;
    }

    auto intDslObj = commsdsl::parse::IntField(obj);
    auto& validRanges = intDslObj.validRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != intDslObj.defaultValue()) {
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
    if (first->field().dslObj().kind() != commsdsl::parse::Field::Kind::Int) {
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
    assert(field.field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);

    auto& keyField = static_cast<const CommsIntField&>(field);
    return keyField.commsVariantPropKeyType();
}

std::string propKeyValueStrInternal(const CommsField& field)
{
    assert(field.field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);

    auto& keyField = static_cast<const CommsIntField&>(field);
    return keyField.commsVariantPropKeyValueStr();
}

bool propKeysEquivalent(const CommsField& first, const CommsField& second)
{
    assert(first.field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);
    assert(second.field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);

    return static_cast<const CommsIntField&>(first).commsVariantIsPropKeyEquivalent(static_cast<const CommsIntField&>(second));
}

const CommsField* getReferenceFieldInternal(const CommsField* field)
{
    while (field->field().dslObj().kind() == commsdsl::parse::Field::Kind::Ref) {
        auto& refField = static_cast<const CommsRefField&>(*field);
        field = dynamic_cast<decltype(field)>(refField.referencedField());
        assert(field != nullptr);
    }

    return field;
}

} // namespace 
    

CommsVariantField::CommsVariantField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsVariantField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsVariantField::writeImpl() const
{
    return commsWrite();
}

CommsVariantField::IncludesList CommsVariantField::commsCommonIncludesImpl() const
{
    IncludesList result = {
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

    util::ReplacementMap repl = {
        {"MEMBER_NAME_MAP_DEF", commsCommonMemberNameMapInternal()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"MEMBER_NAMES_FUNCS", commsCommonMemberNameFuncsCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsCommonMembersCodeImpl() const
{
    util::StringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsCommonCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }

    return util::strListToString(membersCode, "\n", "");
}

CommsVariantField::IncludesList CommsVariantField::commsDefIncludesImpl() const
{
    IncludesList result = {
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

    util::StringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsDefCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }
    
    util::StringsList names;
    for (auto& fPtr : members()) {
        assert(fPtr);
        names.push_back(comms::className(fPtr->dslObj().name()));
    }

    util::ReplacementMap repl = {
        {"MEMBERS_DEFS", util::strListToString(membersCode, "\n", "")},
        {"MEMBERS", util::strListToString(names, ",\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Variant<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = generator();
    auto dslObj = variantDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.schemaOf(*this).mainNamespace()},
        {"CLASS_NAME", comms::className(dslObj.name())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (comms::isGlobalField(*this)) {
        repl["MEMBERS_OPT"] = "<TOpt>";
    }

    return util::processTemplate(Templ, repl);       
}

std::string CommsVariantField::commsDefConstructCodeImpl() const
{
    auto obj = variantDslObj();
    auto idx = obj.defaultMemberIdx();
    if (m_commsMembers.size() <= idx) {
        return strings::emptyString();
    }

    assert(idx < m_commsMembers.size());
    static const std::string Templ = 
        "initField_#^#NAME#$#();\n";
    
    util::ReplacementMap repl = {
        {"NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
    };

    return util::processTemplate(Templ, repl);
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
        return strings::emptyString();
    }

    std::string keyFieldType;
    StringsList cases;
    bool hasDefault = false;
    for (auto* memPtr : m_commsMembers) {
        auto* m = getReferenceFieldInternal(memPtr);
        assert(m->field().dslObj().kind() == commsdsl::parse::Field::Kind::Bundle);
        auto& bundle = static_cast<const CommsBundleField&>(*m);
        auto& bundleMembers = bundle.commsMembers();
        assert(!bundleMembers.empty());
        auto* first = bundleMembers.front();
        assert(first->field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);
        auto& keyField = static_cast<const CommsIntField&>(*first);
        auto bundleAccName = comms::accessName(memPtr->field().dslObj().name());
        auto keyAccName = comms::accessName(keyField.field().dslObj().name());

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

            util::ReplacementMap repl = {
                {"VAL", std::move(valStr)},
                {"BUNDLE_NAME", bundleAccName},
                {"KEY_NAME", keyAccName},
            };

            if (m->commsIsVersionDependent()) {
                auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
                repl["VERSION_ASSIGN"] = std::move(assignStr);
            }
            cases.push_back(util::processTemplate(Templ, repl));
            continue;
        }

        // Last "catch all" element
        assert(memPtr == m_commsMembers.back());

        static const std::string Templ =
            "default:\n"
            "    initField_#^#BUNDLE_NAME#$#().field_#^#KEY_NAME#$#().setValue(commonKeyField.getValue());\n"
            "    #^#VERSION_ASSIGN#$#\n"
            "    return accessField_#^#BUNDLE_NAME#$#().template readFrom<1>(iter, len);";

        util::ReplacementMap repl = {
            {"BUNDLE_NAME", bundleAccName},
            {"KEY_NAME", keyAccName},
        };

        if (m->commsIsVersionDependent()) {
            auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
            repl["VERSION_ASSIGN"] = std::move(assignStr);
        }

        cases.push_back(util::processTemplate(Templ, repl));
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

    util::ReplacementMap repl = {
        {"KEY_FIELD_TYPE", m_optimizedReadKey},
        {"CASES", util::strListToString(cases, "\n", "")},
    };

    if (commsIsVersionDependent()) {
        static const std::string CheckStr =
            "static_assert(Base::isVersionDependent(), \"The field must be recognised as version dependent\");";
        repl["VERSION_DEP"] = CheckStr;
    }

    return util::processTemplate(Templ, repl);    
}

CommsVariantField::StringsList CommsVariantField::commsDefReadMsvcSuppressWarningsImpl() const
{
    return StringsList{"4702"};
}

std::string CommsVariantField::commsDefWriteFuncBodyImpl() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().write(iter, len);";
            
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return comms::ErrorStatus::Success;\n"
        ;

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefRefreshFuncBodyImpl() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().refresh();";
            
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return false;\n"
        ;

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefLengthFuncBodyImpl() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().length();";
            
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return 0U;\n"
        ;

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefValidFuncBodyImpl() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().valid();";
            
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ =
        "switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return false;\n"
        ;

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };
    
    return util::processTemplate(Templ, repl);
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

std::string CommsVariantField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    assert(fieldOptsFunc != nullptr);
    util::StringsList elems;
    for (auto* m : m_commsMembers) {
        auto str = (m->*fieldOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }
    return util::strListToString(elems, "\n", "");
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
    m_commsMembers = commsTransformFieldsList(members());
    if (generator().schemaOf(*this).versionDependentCode()) {
        auto sinceVersion = dslObj().sinceVersion();
        for (auto* m : m_commsMembers) {
            assert(m != nullptr);
            if (sinceVersion < m->field().dslObj().sinceVersion()) {
                generator().logger().error("Currently version dependent members of variant are not supported!");
                return false;
            }
        }
    }

    m_optimizedReadKey = commsOptimizedReadKeyInternal();
    return true;
}

std::string CommsVariantField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::StringsList opts;
    commsAddFieldDefOptions(opts);
    commsAddCustomReadOptInternal(opts);
    util::addToStrList("comms::option::def::HasCustomWrite", opts);
    util::addToStrList("comms::option::def::HasCustomRefresh", opts);        
    util::addToStrList("comms::option::def::VariantHasCustomResetOnDestruct", opts);        
    util::addToStrList("comms::option::def::HasVersionDependentMembers<" + util::boolToString(commsIsVersionDependentImpl()) + ">", opts);        
    return util::strListToString(opts, ",\n", "");
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
    StringsList copyCases;
    StringsList moveCases;
    StringsList eqCases;
    StringsList ltCases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string CopyTempl =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#() = other.accessField_#^#MEM_NAME#$#(); return *this;";

        static const std::string MoveTempl =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#() = std::move(other.accessField_#^#MEM_NAME#$#()); return *this;";

        static const std::string EqTempl =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#() == other.accessField_#^#MEM_NAME#$#();";

        static const std::string LtTempl =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#() < other.accessField_#^#MEM_NAME#$#();";

        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };

        copyCases.push_back(util::processTemplate(CopyTempl, repl));
        moveCases.push_back(util::processTemplate(MoveTempl, repl));
        eqCases.push_back(util::processTemplate(EqTempl, repl));
        ltCases.push_back(util::processTemplate(LtTempl, repl));
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


    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"COPY_CASES", util::strListToString(copyCases, "\n", "")},
        {"MOVE_CASES", util::strListToString(moveCases, "\n", "")},
        {"EQ_CASES", util::strListToString(eqCases, "\n", "")},
        {"LT_CASES", util::strListToString(ltCases, "\n", "")},
    };

    if (commsIsExtended()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    if (commsIsVersionOptional()) {
        repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    }    

    return util::processTemplate(Templ, repl);
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

    util::StringsList accessDocList;
    util::StringsList namesList;
    accessDocList.reserve(m_commsMembers.size());
    namesList.reserve(m_commsMembers.size());

    auto& gen = generator();
    for (auto& mPtr : members()) {
        namesList.push_back(comms::accessName(mPtr->dslObj().name()));
        std::string accessStr =
            "///     @li @b FieldIdx_" +  namesList.back() + 
            " index, @b Field_" + namesList.back() + " type,\n"
            "///         @b initField_" + namesList.back() + "(), @b deinitField_" + namesList.back() +
            "() and @b accessField_" + namesList.back() + "() access functions -\n"
            "///         for " + comms::scopeFor(*mPtr, gen) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    util::ReplacementMap repl = {
        {"ACCESS_DOC", util::strListToString(accessDocList, "\n", "")},
        {"NAMES", util::strListToString(namesList, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefAccessCodeGeneratedInternal() const
{
    StringsList indicesList;
    StringsList accessList;
    indicesList.reserve(m_commsMembers.size());
    accessList.reserve(m_commsMembers.size());

    auto membersPrefix = comms::className(dslObj().name()) + strings::membersSuffixStr();
    auto docScope = membersPrefix + "::";
    auto typeScope = "typename " + membersPrefix + "<TOpt>::";
    for (auto& m : m_commsMembers) {
        auto accName = comms::accessName(m->field().dslObj().name());
        auto className = comms::className(m->field().dslObj().name());

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

        util::ReplacementMap accRepl = {
            {"DOC_SCOPE", docScope + className},
            {"TYPE_SCOPE", typeScope + className},
            {"NAME", accName}
        };
        accessList.push_back(util::processTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "/// @brief Allow access to internal fields by index.\n"
        "enum FieldIdx : unsigned\n"
        "{\n"
        "    #^#INDICES#$#,\n"
        "    FieldIdx_numOfValues"
        "};\n\n"
        "#^#ACCESS#$#\n";    

    util::ReplacementMap repl = {
        {"INDICES", util::strListToString(indicesList, ",\n", "")},
        {"ACCESS", util::strListToString(accessList, "\n", "")},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefFieldExecCodeInternal() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#:\n"
            "    memFieldDispatch<FieldIdx_#^#MEM_NAME#$#>(accessField_#^#MEM_NAME#$#(), std::forward<TFunc>(func));\n"
            "    break;";
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
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

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    auto str = util::processTemplate(Templ, repl);
    str += "\n";

    repl.insert({
        {"VARIANT", " (const variant)"},
        {"CONST", "const"}
    });
    str += util::processTemplate(Templ, repl);
    return str;
}

std::string CommsVariantField::commsDefResetCodeInternal() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: deinitField_#^#MEM_NAME#$#(); return;";
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
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

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefCanWriteCodeInternal() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: return accessField_#^#MEM_NAME#$#().canWrite();";
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
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

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefSelectFieldCodeInternal() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#: initField_#^#MEM_NAME#$#(); return;";
        util::ReplacementMap repl = {
            {"MEM_NAME", comms::accessName(m_commsMembers[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
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

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}


void CommsVariantField::commsAddCustomReadOptInternal(StringsList& opts) const
{
    if (!m_optimizedReadKey.empty()) {
        util::addToStrList("comms::option::def::HasCustomRead", opts);
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
        if (memPtr->field().dslObj().kind() != commsdsl::parse::Field::Kind::Bundle) {
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

    auto membersPrefix = comms::className(dslObj().name()) + strings::membersSuffixStr() + strings::commonSuffixStr();
    util::StringsList names;
    for (auto& fPtr : members()) {
        assert(fPtr);
        names.push_back(membersPrefix + "::" + comms::className(fPtr->dslObj().name()) + strings::commonSuffixStr() + "::name()");
    }

    util::ReplacementMap repl = {
        {"MEMBERS", util::strListToString(names, ",\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefMemberNamesTypesInternal() const
{
    static const std::string Templ = 
        "/// @brief Single member name info entry\n"
        "using MemberNameInfo = #^#COMMON_SCOPE#$#::MemberNameInfo;\n\n"
        "/// @brief Type returned from @ref memberNamesMap() member function.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::MemberNamesMapInfo.\n"
        "using MemberNamesMapInfo = #^#COMMON_SCOPE#$#::MemberNamesMapInfo;\n";

    util::ReplacementMap repl = {
        {"COMMON_SCOPE", comms::commonScopeFor(*this, generator())}
    };

    return util::processTemplate(Templ, repl);
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


    util::ReplacementMap repl = {
        {"COMMON_SCOPE", comms::commonScopeFor(*this, generator())}
    };
    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2comms
