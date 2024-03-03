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

#include "CommsOptionalField.h"

#include "CommsGenerator.h"
#include "CommsInterface.h"

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

namespace 
{

const std::string getFieldAccessPrefixInternal(const CommsField& field)
{
    if (comms::isInterfaceShallowMemberField(field.field())) {
        static const std::string Str("Base::transportField_");
        return Str;        
    }

    static const std::string Str("field_");
    return Str;
}

const std::string getFieldTypePrefixInternal(const CommsField& field)
{
    if (comms::isInterfaceShallowMemberField(field.field())) {
        static const std::string Str("Base::TransportField_");
        return Str;        
    }

    static const std::string Str("Field_");
    return Str;
}

const CommsField* findInterfaceFieldInternal(const commsdsl::gen::Generator& generator, const std::string refStr)
{
    if (refStr.empty()) {
        return nullptr;
    }

    auto dotPos = refStr.find(".");
    auto fieldName = refStr.substr(0, dotPos);
    std::string restRefStr;

    if (dotPos < refStr.size()) {
        restRefStr = refStr.substr(dotPos + 1);
    }

    auto& schema = generator.currentSchema();
    auto interfaces = schema.getAllInterfaces();
    for (auto* i : interfaces) {
        auto& commsFields = CommsInterface::cast(i)->commsFields();
        auto iter = 
            std::find_if(
                commsFields.begin(), commsFields.end(),
                [&fieldName, &restRefStr](auto* f)
                {
                    if (f->field().dslObj().name() != fieldName) {
                        return false;
                    }

                    return f->commsVerifyInnerRef(restRefStr);
                });

        if (iter != commsFields.end()) {
            return *iter;
        }    
    }

    return nullptr;
}

bool hasInterfaceReferenceInternal(const commsdsl::parse::OptCond& cond)
{
    if (cond.kind() == commsdsl::parse::OptCond::Kind::Expr) {
        commsdsl::parse::OptCondExpr exprCond(cond);
        auto& left = exprCond.left();
        auto& right = exprCond.right();

        return 
            ((!left.empty()) && (left[0] == strings::interfaceFieldRefPrefix())) ||
            ((!right.empty()) && (right[0] == strings::interfaceFieldRefPrefix()));
    }

    if ((cond.kind() != commsdsl::parse::OptCond::Kind::List)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return false;
    }    

    commsdsl::parse::OptCondList listCond(cond);
    auto conditions = listCond.conditions();

    return 
        std::any_of(
            conditions.begin(), conditions.end(),
            [](auto& c)
            {
                return hasInterfaceReferenceInternal(c);
            });
}

} // namespace 
    

CommsOptionalField::CommsOptionalField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

std::string CommsOptionalField::commsDslCondToString(
    const CommsGenerator& generator, 
    const CommsFieldsList& siblings, 
    const commsdsl::parse::OptCond& cond,
    bool bracketsWrap) 
{
    if (cond.kind() == commsdsl::parse::OptCond::Kind::Expr) {
        auto findSiblingFieldFunc =
            [&siblings](const std::string& name) -> const CommsField*
            {
                auto iter =
                    std::find_if(
                        siblings.begin(), siblings.end(),
                        [&name](auto& f)
                        {
                            return f->field().dslObj().name() == name;
                        });

                if (iter == siblings.end()) {
                    return nullptr;
                }

                return *iter;
            };

        auto opFunc =
            [](const std::string& val) -> const std::string& {
                if (val == "=") {
                    static const std::string Str = "==";
                    return Str;
                }
                return val;
            };

        commsdsl::parse::OptCondExpr exprCond(cond);
        auto leftInfo = exprCond.leftInfo();
        auto& op = opFunc(exprCond.op());
        auto rightInfo = exprCond.rightInfo();

        using OperandType = commsdsl::parse::OptCondExpr::OperandType;
        using AccMode = commsdsl::parse::OptCondExpr::AccMode;
        if (leftInfo.m_type != OperandType::Invalid) {
            assert(!op.empty());
            assert(rightInfo.m_type != OperandType::Invalid);

            auto leftSepPos = leftInfo.m_access.find(".");
            std::string leftFieldName(leftInfo.m_access, 0, leftSepPos);

            const CommsField* leftField = nullptr;
            if (leftInfo.m_type == OperandType::InterfaceRef) {
                leftField = findInterfaceFieldInternal(generator, leftInfo.m_access);
            }
            else if (leftInfo.m_type == OperandType::SiblingRef) {
                leftField = findSiblingFieldFunc(leftFieldName);
            }
            
            if (leftField == nullptr) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                return strings::emptyString();
            }

            std::string remLeft;
            if (leftSepPos < leftInfo.m_access.size()) {
                remLeft = leftInfo.m_access.substr(leftSepPos + 1);
            }

            assert(leftInfo.m_mode != AccMode::Exists);
            assert(rightInfo.m_mode != AccMode::Size);
            assert(rightInfo.m_mode != AccMode::Exists);            

            if (leftInfo.m_mode == AccMode::Size) {
                return commsDslCondToStringFieldSizeCompInternal(leftField, remLeft, op, rightInfo.m_access);
            }    

            if (rightInfo.m_type == OperandType::Value) {
                return commsDslCondToStringFieldValueCompInternal(leftField, remLeft, op, rightInfo.m_access);
            }

            auto rigthSepPos = rightInfo.m_access.find(".");
            std::string rightFieldName(rightInfo.m_access, 0, rigthSepPos);

            const CommsField* rightField = nullptr;
            if (rightInfo.m_type == OperandType::InterfaceRef) {
                rightField = findInterfaceFieldInternal(generator, rightInfo.m_access);
            }
            else if (rightInfo.m_type == OperandType::SiblingRef) {
                rightField = findSiblingFieldFunc(rightFieldName);
            }
            
            if (rightField == nullptr) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                return strings::emptyString();
            }

            std::string remRight;
            if (rigthSepPos < rightInfo.m_access.size()) {
                remRight = rightInfo.m_access.substr(rigthSepPos + 1);
            }            

            return commsDslCondToStringFieldFieldCompInternal(leftField, remLeft, op, rightField, remRight);
        }

        // Reference to bit in "set".
        if ((rightInfo.m_type != OperandType::InterfaceRef) &&
            (rightInfo.m_type != OperandType::SiblingRef)) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::emptyString();
        }

        auto rightSepPos = rightInfo.m_access.find(".");
        std::string rightFieldName(rightInfo.m_access, 0, rightSepPos);

        const CommsField* rightField = nullptr;
        if (rightInfo.m_type == OperandType::InterfaceRef) {
            rightField = findInterfaceFieldInternal(generator, rightInfo.m_access);
        }
        else if (rightInfo.m_type == OperandType::SiblingRef) {
            rightField = findSiblingFieldFunc(rightFieldName);
        }        

        if (rightField == nullptr) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::emptyString();
        }

        assert(rightInfo.m_mode != AccMode::Size);

        std::string remRight;
        if (rightSepPos < rightInfo.m_access.size()) {
            remRight = rightInfo.m_access.substr(rightSepPos + 1);
        }        

        if (rightInfo.m_mode == AccMode::Exists) {
            return commsDslCondToStringFieldExistsCompInternal(rightField, remRight, op);
        }          

        auto rightAccName = comms::accessName(rightField->field().dslObj().name());
        auto checks = rightField->commsCompOptChecks(remRight, getFieldAccessPrefixInternal(*rightField) + rightAccName + "()");
        checks.push_back(op + getFieldAccessPrefixInternal(*rightField) + rightAccName + "()" + rightField->commsValueAccessStr(remRight));

        return util::strListToString(checks, " &&\n", "");
    }

    if ((cond.kind() != commsdsl::parse::OptCond::Kind::List)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::emptyString();
    }

    commsdsl::parse::OptCondList listCond(cond);
    auto type = listCond.type();

    static const std::string AndOp = " &&\n";
    static const std::string OrOp = " ||\n";

    auto* op = &AndOp;
    if (type == commsdsl::parse::OptCondList::Type::Or) {
        op = &OrOp;
    }
    else {
        assert(type == commsdsl::parse::OptCondList::Type::And);
    }

    auto conditions = listCond.conditions();
    std::string condTempl;
    util::ReplacementMap repl;
    if (bracketsWrap) {
        condTempl += '(';
    }

    unsigned count = 0U;
    for (auto& c : conditions) {
        auto condVal = commsDslCondToString(generator, siblings, c, true);
        if (condVal.empty()) {
            continue;
        }

        if (0U < count) {
            condTempl += ' ';
        }

        ++count;

        auto condStr = "COND" + std::to_string(count);
        repl[condStr] = std::move(condVal);
        condTempl += "(#^#";
        condTempl += condStr;
        condTempl += "#$#)";
        if (&c != &conditions.back()) {
            condTempl += *op;
        }
    }

    if (bracketsWrap) {
        condTempl += ')';
    }

    return util::processTemplate(condTempl, repl);
}

bool CommsOptionalField::prepareImpl()
{
    bool result = 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsCheckCondSupportedInternal();

    if (result) {
        m_commsExternalField = dynamic_cast<CommsField*>(externalField());
        m_commsMemberField = dynamic_cast<CommsField*>(memberField());
    }

    return result;
}

bool CommsOptionalField::writeImpl() const
{
    return commsWrite();
}

CommsOptionalField::IncludesList CommsOptionalField::commsCommonIncludesImpl() const 
{
    IncludesList result;
    if (m_commsMemberField != nullptr) {
        result = m_commsMemberField->commsCommonIncludes();
    }

    return result;
}

std::string CommsOptionalField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsOptionalField::commsCommonMembersCodeImpl() const
{
    if (m_commsMemberField != nullptr) {
        return m_commsMemberField->commsCommonCode();
    }

    return strings::emptyString();
}

CommsOptionalField::IncludesList CommsOptionalField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/Optional.h"
    };

    if (m_commsExternalField != nullptr) {
        result.push_back(comms::relHeaderPathFor(m_commsExternalField->field(), generator()));
        return result;
    }

    assert(m_commsMemberField != nullptr);
    auto incList = m_commsMemberField->commsDefIncludes();
    result.reserve(result.size() + incList.size());
    std::move(incList.begin(), incList.end(), std::back_inserter(result));
    return result;
}

std::string CommsOptionalField::commsDefMembersCodeImpl() const
{
    if (m_commsMemberField != nullptr) {
        return m_commsMemberField->commsDefCode();
    }

    return strings::emptyString();
}

std::string CommsOptionalField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Optional<\n"
        "    #^#FIELD_REF#$##^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"FIELD_REF", commsDefFieldRefInternal()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsOptionalField::commsDefBundledReadPrepareFuncBodyImpl([[maybe_unused]] const CommsFieldsList& siblings) const
{
    auto c = optionalDslObj().cond();
    if (!c.valid()) {
        return strings::emptyString();
    }

    return "refresh_" + comms::accessName(dslObj().name()) + "();\n";
}

std::string CommsOptionalField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto c = optionalDslObj().cond();
    if (!c.valid()) {
        return strings::emptyString();
    }

    auto cond = commsDslCondToStringInternal(siblings, c);
    if (cond.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "auto mode = comms::field::OptionalMode::Missing;\n"
        "if (#^#COND#$#) {\n"
        "    mode = comms::field::OptionalMode::Exists;\n"
        "}\n\n"
        "if (field_#^#NAME#$#()#^#FIELD_ACC#$#.getMode() == mode) {\n"
        "    return false;\n"
        "}\n\n"
        "field_#^#NAME#$#()#^#FIELD_ACC#$#.setMode(mode);\n"
        "return true;\n";

    util::ReplacementMap repl {
        {"NAME", comms::accessName(name())},
        {"COND", std::move(cond)}
    };

    if (commsIsVersionOptional()) {
        repl["FIELD_ACC"] = ".field()";
    }

    return util::processTemplate(Templ, repl);    
}

bool CommsOptionalField::commsIsVersionDependentImpl() const
{
    return (m_commsMemberField != nullptr) && (m_commsMemberField->commsIsVersionDependent());
}

std::string CommsOptionalField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    if (m_commsMemberField == nullptr) {
        return strings::emptyString();
    }

    assert(fieldOptsFunc != nullptr);
    return (m_commsMemberField->*fieldOptsFunc)();
}

std::size_t CommsOptionalField::commsMaxLengthImpl() const
{
    if (m_commsExternalField != nullptr) {
        return m_commsExternalField->commsMaxLength();
    }

    assert(m_commsMemberField != nullptr);
    return m_commsMemberField->commsMaxLength();
}

std::string CommsOptionalField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsValueAccessStrImpl(accStr, prefix);
    }

    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    auto remAccStr = commsMemberAccessStringInternal(accStr);
    return field->commsValueAccessStr(remAccStr, prefix + ".field()");
}

std::string CommsOptionalField::commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsSizeAccessStrImpl(accStr, prefix);
    }

    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    auto remAccStr = commsMemberAccessStringInternal(accStr);
    return field->commsSizeAccessStr(remAccStr, prefix + ".field()");
}

void CommsOptionalField::commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const
{
    checks.push_back(prefix + ".doesExist()");

    if (accStr.empty()) {
        CommsBase::commsCompOptChecksImpl(accStr, checks, prefix);
        return;
    }

    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    auto remAccStr = commsMemberAccessStringInternal(accStr);
    field->commsCompOptChecks(remAccStr, checks, prefix + ".field()");
}

std::string CommsOptionalField::commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompValueCastTypeImpl(accStr, prefix);
    }

    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    auto remAccStr = commsMemberAccessStringInternal(accStr);
    return field->commsCompValueCastType(remAccStr, prefix + "Field::");
}

std::string CommsOptionalField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }

    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    auto remAccStr = commsMemberAccessStringInternal(accStr);
    return field->commsCompPrepValueStr(remAccStr, value);
}

bool CommsOptionalField::commsCheckCondSupportedInternal() const
{
    auto obj = optionalDslObj();
    auto cond = obj.cond();

    if ((!cond.valid()) || (comms::isMessageShallowMemberField(*this))) {
        return true;
    }    

    if (hasInterfaceReferenceInternal(cond)) {
        generator().logger().error(
            "Referencing interface member fields from within " + comms::scopeFor(*this, generator()) + 
            " is not supported, only direct member fields of the message can use such conditions.");
        return false;
    }

    return true;
}

std::string CommsOptionalField::commsDefFieldRefInternal() const
{
    if (m_commsExternalField != nullptr) {
            std::string templOpt;
        if (!comms::isInterfaceDeepMemberField(*this)) {
            templOpt = "TOpt";
        }

        return comms::scopeFor(m_commsExternalField->field(), generator()) + '<' + templOpt + '>';
    }

    assert(m_commsMemberField != nullptr);
    auto ref = "typename " + comms::className(name()) + strings::membersSuffixStr();
    if (comms::isGlobalField(*this)) {
        ref += "<TOpt>";
    }

    ref += "::" + comms::className(m_commsMemberField->field().name());    
    return ref;
}

std::string CommsOptionalField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddModeOptInternal(opts);
    commsAddMissingOnReadFailOptInternal(opts);
    commsAddMissingOnInvalidOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

void CommsOptionalField::commsAddModeOptInternal(StringsList& opts) const
{
    static const std::string Map[] = {
        strings::emptyString(),
        "comms::option::def::ExistsByDefault",
        "comms::option::def::MissingByDefault"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::OptionalField::Mode::NumOfValues), "Invalid map");

    auto obj = optionalDslObj();
    auto mode = obj.defaultMode();
    auto idx = static_cast<std::size_t>(mode);
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        idx = 0U;
    }

    if (Map[idx].empty()) {
        return;
    }

    opts.push_back(Map[idx]);
}

void CommsOptionalField::commsAddMissingOnReadFailOptInternal(StringsList& opts) const
{
    auto obj = optionalDslObj();
    if (!obj.missingOnReadFail()) {
        return;
    }

    opts.push_back("comms::option::def::MissingOnReadFail");
}

void CommsOptionalField::commsAddMissingOnInvalidOptInternal(StringsList& opts) const
{
    auto obj = optionalDslObj();
    if (!obj.missingOnInvalid()) {
        return;
    }

    opts.push_back("comms::option::def::MissingOnInvalid");
    if (commsHasCustomValid()) {
        commsAddFieldTypeOption(opts);
    }
}

std::string CommsOptionalField::commsDslCondToStringInternal(
    const CommsFieldsList& siblings, 
    const commsdsl::parse::OptCond& cond,
    bool bracketsWrap) const 
{
    return commsDslCondToString(CommsGenerator::cast(generator()), siblings, cond, bracketsWrap);
}

std::string CommsOptionalField::commsMemberAccessStringInternal(const std::string& accStr) const
{
    auto sepPos = accStr.find('.');
    std::string remAccStr;
    if (sepPos < accStr.size()) {
        remAccStr = accStr.substr(sepPos + 1);
    }

#ifndef NDEBUG    
    auto* field = m_commsExternalField;
    if (field == nullptr) {
        field = m_commsMemberField;
    }

    assert(field != nullptr);
    assert(accStr.substr(0, sepPos) == field->field().dslObj().name());
#endif    

    return remAccStr;
}

std::string CommsOptionalField::commsDslCondToStringFieldValueCompInternal(
    const CommsField* field, 
    const std::string& accStr,
    const std::string& op, 
    const std::string& value)
{
    auto accName = comms::accessName(field->field().dslObj().name());
    auto prefix = getFieldAccessPrefixInternal(*field) + accName + "()";
    auto optConds = field->commsCompOptChecks(accStr, prefix);
    auto valueStr = field->commsCompPrepValueStr(accStr, value);
    auto typeCast = field->commsCompValueCastType(accStr);
    if (!typeCast.empty()) {
        valueStr = "static_cast<typename " + getFieldTypePrefixInternal(*field) + accName + "::" + typeCast + ">(" + valueStr + ")";
    }

    auto expr = prefix + field->commsValueAccessStr(accStr) + ' ' + op + ' ' + valueStr;

    if (optConds.empty()) {
        return expr;
    }

    static const std::string Templ = 
        "#^#COND#$# &&\n"
        "(#^#EXPR#$#)";
    
    util::ReplacementMap repl = {
        {"COND", util::strListToString(optConds, " &&\n", "")},
        {"EXPR", std::move(expr)},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsOptionalField::commsDslCondToStringFieldFieldCompInternal(
    const CommsField* leftField, 
    const std::string& leftAccStr,
    const std::string& op, 
    const CommsField* rightField, 
    const std::string& rightAccStr)
{
    auto leftAccName = comms::accessName(leftField->field().dslObj().name());
    auto leftPrefix = getFieldAccessPrefixInternal(*leftField) + leftAccName + "()";
    auto rightAccName = comms::accessName(rightField->field().dslObj().name());
    auto rightPrefix = getFieldAccessPrefixInternal(*rightField) + rightAccName + "()";

    auto optConds = leftField->commsCompOptChecks(leftAccStr, leftPrefix);
    rightField->commsCompOptChecks(rightAccStr, optConds, rightPrefix);
    auto valueStr = rightPrefix + rightField->commsValueAccessStr(rightAccStr);
    auto typeCast = leftField->commsCompValueCastType(leftAccStr);
    if (!typeCast.empty()) {
        auto accName = comms::accessName(leftField->field().dslObj().name());
        valueStr = "static_cast<typename Field_" + accName + "::" + typeCast + ">(" + valueStr + ")";
    }

    auto expr = leftPrefix + leftField->commsValueAccessStr(leftAccStr) + ' ' + op + ' ' + valueStr;

    if (optConds.empty()) {
        return expr;
    }

    static const std::string Templ = 
        "#^#COND#$# &&\n"
        "(#^#EXPR#$#)";
    
    util::ReplacementMap repl = {
        {"COND", util::strListToString(optConds, " &&\n", "")},
        {"EXPR", std::move(expr)},
    };

    return util::processTemplate(Templ, repl);    
}

std::string CommsOptionalField::commsDslCondToStringFieldSizeCompInternal(
    const CommsField* field, 
    const std::string& accStr,
    const std::string& op, 
    const std::string& value)
{
    auto accName = comms::accessName(field->field().dslObj().name());
    auto prefix = getFieldAccessPrefixInternal(*field) + accName + "()";
    auto optConds = field->commsCompOptChecks(accStr, prefix);

    auto sizeAccStr = prefix + field->commsSizeAccessStr(accStr);

    std::string expr;
    bool emptyCheck = 
        (op == "==") && 
        (value == "0");

    bool notEmptyCheck = 
        (op == "!=") && 
        (value == "0");        
        

    if (emptyCheck) {
        expr = util::strReplace(sizeAccStr, ".size()", ".empty()");
    }
    else if (notEmptyCheck) {
        expr = "!" + util::strReplace(sizeAccStr, ".size()", ".empty()");
    }
    else {
        auto valueStr = "static_cast<std::size_t>(" + value + ")";
        expr = sizeAccStr + ' ' + op + ' ' + valueStr;
    }

    if (optConds.empty()) {
        return expr;
    }

    static const std::string Templ = 
        "#^#COND#$# &&\n"
        "(#^#EXPR#$#)";
    
    util::ReplacementMap repl = {
        {"COND", util::strListToString(optConds, " &&\n", "")},
        {"EXPR", std::move(expr)},
    };

    return util::processTemplate(Templ, repl);    
}

std::string CommsOptionalField::commsDslCondToStringFieldExistsCompInternal(
    const CommsField* field, 
    const std::string& accStr,
    const std::string& op)
{

    auto accName = comms::accessName(field->field().dslObj().name());
    auto prefix = getFieldAccessPrefixInternal(*field) + accName + "()";
    auto optConds = field->commsCompOptChecks(accStr, prefix);

    if (optConds.empty()) {
        return strings::emptyString();
    }

    auto condsStr = util::strListToString(optConds, " &&\n", "");
    if (op.empty()) {
        return condsStr;
    }
    

    static const std::string Templ = 
        "!(#^#COND#$#)";
    
    util::ReplacementMap repl = {
        {"COND", std::move(condsStr)},
    };

    return util::processTemplate(Templ, repl);       
}

} // namespace commsdsl2comms
