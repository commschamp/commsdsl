//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ParseOptCondImpl.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include "ParseBitfieldFieldImpl.h"
#include "ParseBundleFieldImpl.h"
#include "ParseLogger.h"
#include "ParseProtocolImpl.h"
#include "ParseRefFieldImpl.h"
#include "ParseSchemaImpl.h"
#include "ParseSetFieldImpl.h"
#include "parse_common.h"
#include "parse_util.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const char Esc = '\\';
const char Deref = common::siblingRefPrefix();
const char IfaceDeref = common::interfaceRefPrefix();

void parseDiscardNonSizeReferences(ParseFieldImpl::FieldRefInfosList& infos)
{
    infos.erase(
        std::remove_if(
            infos.begin(), infos.end(),
            [](auto& fieldInfo)
            {
                return fieldInfo.m_refType != ParseFieldImpl::FieldRefType_Size;
            }),
        infos.end());
}


void parseDiscardNonFieldReferences(ParseFieldImpl::FieldRefInfosList& infos)
{
    infos.erase(
        std::remove_if(
            infos.begin(), infos.end(),
            [](auto& fieldInfo)
            {
                return fieldInfo.m_refType != ParseFieldImpl::FieldRefType_Field;
            }),
        infos.end());
}

ParseOptCondExprImpl::OperandInfo parseDperandInfoInternal(const std::string& val)
{
    ParseOptCondExprImpl::OperandInfo result;
    if (val.empty()) {
        return result;
    }

    if ((val[0] != common::siblingRefPrefix()) && (val[0] != common::interfaceRefPrefix())) {
        result.m_type = ParseOptCondExprImpl::OperandType::Value;
        result.m_access = val;
        return result;
    }

    if (val[0] == common::siblingRefPrefix()) {
        result.m_type = ParseOptCondExprImpl::OperandType::SiblingRef;
    }
    else {
        assert(val[0] == common::interfaceRefPrefix());
        result.m_type = ParseOptCondExprImpl::OperandType::InterfaceRef;
    }

    auto restPos = 1U;
    do {
        if (val.size() <= 1U) {
            break;
        }

        if (val[restPos] == '#') {
            result.m_mode = ParseOptCondExprImpl::AccMode::Size;
            ++restPos;
            break;
        }   

        if (val[restPos] == '?') {
            result.m_mode = ParseOptCondExprImpl::AccMode::Exists;
            ++restPos;
            break;
        }             

    } while (false);

    result.m_access = val.substr(restPos);
    return result;
}

} // namespace

ParseOptCondImpl::ParseOptCondImpl() :
    m_condStr(common::condStr())
{
}

bool ParseOptCondExprImpl::parse(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol)
{
    auto& logger = protocol.parseLogger();
    if (expr.empty()) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "Invalid condition expression";
        return false;
    }

    assert(!parseHasUpdatedValue());
    return
        parseCheckComparison(expr, "!=", node, protocol) &&
        parseCheckComparison(expr, ">=", node, protocol) &&
        parseCheckComparison(expr, "<=", node, protocol) &&
        parseCheckComparison(expr, "=", node, protocol) &&
        parseCheckComparison(expr, ">", node, protocol) &&
        parseCheckComparison(expr, "<", node, protocol) &&
        parseCheckBool(expr, node, protocol) &&
        parseHasUpdatedValue();
}

ParseOptCondExprImpl::OperandInfo ParseOptCondExprImpl::parseLeftInfo() const
{
    return parseDperandInfoInternal(m_left);
}

ParseOptCondExprImpl::OperandInfo ParseOptCondExprImpl::parseRightInfo() const
{
    return parseDperandInfoInternal(m_right);
}

ParseOptCondImpl::Kind ParseOptCondExprImpl::parseKindImpl() const
{
    return Kind::Expr;
}

ParseOptCondImpl::Ptr ParseOptCondExprImpl::parseCloneImpl() const
{
    return Ptr(new ParseOptCondExprImpl(*this));
}

bool ParseOptCondExprImpl::parseVerifyImpl(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    if (m_left.empty()) {
        return parseVerifySingleElementCheck(fields, node, protocol);
    }

    return parseVerifyComparison(fields, node, protocol);
}

bool ParseOptCondExprImpl::parseHasInterfaceReferenceImpl() const
{
    return 
        ((!m_left.empty()) && (m_left[0] == IfaceDeref)) ||
        ((!m_right.empty()) && (m_right[0] == IfaceDeref));
}

bool ParseOptCondExprImpl::parseHasUpdatedValue()
{
    return (!m_left.empty()) ||
           (!m_right.empty()) ||
           (!m_op.empty());
}

bool ParseOptCondExprImpl::parseCheckComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, const ParseProtocolImpl& protocol)
{
    if (parseHasUpdatedValue()) {
        return true;
    }

    auto& logger = protocol.parseLogger();
    auto reportInvalidExprFunc =
        [node, &logger, &expr]()
        {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << expr << "\" string is not valid condition expression.";
        };

    std::size_t opPos = 0U;
    while (true) {
        opPos = expr.find(op, opPos);
        if (opPos == 0U) {
            reportInvalidExprFunc();
            return false;
        }

        if (opPos == std::string::npos) {
            return true;
        }

        if (expr[opPos - 1] != Esc) {
            break;
        }

        ++opPos;
    }

    auto leftEndPos = expr.find_last_not_of(' ', opPos - 1);
    if (leftEndPos == std::string::npos) {
        reportInvalidExprFunc();
        return false;
    }

    auto rightBegPos = expr.find_first_not_of(' ', opPos + op.size());
    if (rightBegPos == std::string::npos) {
        reportInvalidExprFunc();
        return false;
    }

    m_left.assign(expr.begin(), expr.begin() + leftEndPos + 1U);
    m_op = op;
    m_right.assign(expr.begin() + rightBegPos, expr.end());

    if (m_left.empty() || m_right.empty()) {
        reportInvalidExprFunc();
        return false;
    }

    if (m_left[0] == Deref) {
        return true;
    }

    if (m_left[0] == IfaceDeref) {
        if (!protocol.parseIsInterfaceFieldReferenceSupported()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "References to the interface fields are not supported in the selected " << common::dslVersionStr() << ".";
            return false;            
        }
        return true;
    }

    parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
        "Invalid \"" << condStr() << "\" expression, left side of "
        "comparison operator must dereference other field.";
    return false;
}

bool ParseOptCondExprImpl::parseCheckBool(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol)
{
    if (parseHasUpdatedValue()) {
        return true;
    }

    auto& logger = protocol.parseLogger();
    auto reportInvalidExprFunc =
        [node, &logger, &expr]()
        {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << expr << "\" string is not valid condition expression.";
        };

    assert(!expr.empty());
    if (expr[0] == Deref) {
        m_right = expr;
        return true;
    }

    if (expr[0] == IfaceDeref) {
        if (!protocol.parseIsInterfaceFieldReferenceSupported()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "References to the interface fields are not supported in the selected " << common::dslVersionStr() << ".";
            return false;            
        }        
        m_right = expr;
        return true;
    }    

    if (expr[0] != '!') {
        reportInvalidExprFunc();
        return false;
    }

    auto valPos = expr.find_first_not_of(' ', 1);
    if (valPos == std::string::npos) {
        reportInvalidExprFunc();
        return false;
    }

    do {
        if (expr[valPos] == Deref) {
            break;
        }

        if (expr[valPos] == IfaceDeref) {
            if (!protocol.parseIsInterfaceFieldReferenceSupported()) {
                parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                    "References to the interface fields are not supported in the selected " << common::dslVersionStr() << ".";
                return false;            
            }        
            break;
        }          

        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "Invalid \"" << condStr() << "\" expression, "
            "the check must dereference other field.";
        return false;        

    } while (false);

    m_op = std::string("!");
    m_right.assign(expr.begin() + valPos, expr.end());
    return true;
}

bool ParseOptCondExprImpl::parseVerifySingleElementCheck(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    if (m_right[0] == Deref) {
        return parseVerifySiblingSingleElementCheck(fields, node, protocol);
    }

    assert(m_right[0] == IfaceDeref);
    return parseVerifyInterfaceBitCheck(node, protocol);
}

bool ParseOptCondExprImpl::parseVerifySiblingSingleElementCheck(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == Deref);

    auto& logger = protocol.parseLogger();
    auto reportInvalidSiblingRef = 
        [node, &logger](const std::string& refStr)
        {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << refStr << "\" string is invalid sibling reference.";

        };

    auto info = ParseFieldImpl::parseProcessSiblingRef(fields, m_right.substr(1));
    if (info.m_field == nullptr) {
        reportInvalidSiblingRef(m_right);
        return false;
    }

    if (info.m_refType == ParseFieldImpl::FieldRefType_InnerValue) {
        assert(!info.m_valueName.empty());
        return true;
    }

    if (info.m_refType != ParseFieldImpl::FieldRefType_Exists) {
        reportInvalidSiblingRef(m_right);
        return false;
    }

    if (!protocol.parseIsExistsCheckInConditionalsSupported()) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "The optional mode check in the optional condition is not supported for the selected DSL version.";
        return false;
    }

    return true;
}

bool ParseOptCondExprImpl::parseVerifyInterfaceBitCheck(::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == IfaceDeref);

    auto& schema = protocol.parseCurrSchema();
    auto foundFields = schema.parseProcessInterfaceFieldRef(m_right.substr(1));
    auto hasValidRef = 
        std::any_of(
            foundFields.begin(), foundFields.end(),
            [](auto& info)
            {
                assert(info.m_field != nullptr);
                return 
                    (info.m_refType == ParseFieldImpl::FieldRefType_InnerValue) &&
                    (info.m_field->parseKind() == ParseFieldImpl::Kind::Set);
            });

    if (hasValidRef) {
        return true;
    }

    auto& logger = protocol.parseLogger();
    parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
        "The \"" << m_right << "\" string is expected to dereference existing bit in existing <" <<
        common::setStr() << "> field or <" << common::refStr() << "> to it in one of the <" << common::interfaceStr() << ">-es.";
    return false;
}

bool ParseOptCondExprImpl::parseVerifyComparison(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    if (m_left[0] == Deref) {
        return parseVerifySiblingComparison(fields, node, protocol);
    }

    assert(m_left[0] == IfaceDeref);
    return parseVerifyInterfaceComparison(fields, node, protocol);
}

bool ParseOptCondExprImpl::parseVerifySiblingComparison(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    assert(m_left[0] == Deref);

    auto& logger = protocol.parseLogger();
    auto reportInvalidSiblingRef = 
        [node, &logger](const std::string& refStr)
        {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << refStr << "\" string is expected to dereference existing sibling field.";

        };

    auto leftInfo = ParseFieldImpl::parseProcessSiblingRef(fields, m_left.substr(1));
    if (leftInfo.m_field == nullptr) {
        reportInvalidSiblingRef(m_left);
        return false;
    } 

    if (leftInfo.m_refType == ParseFieldImpl::FieldRefType_Size) {
        if (!protocol.parseIsSizeCompInConditionalsSupported()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The size comparison check in the optional condition is not supported for the selected DSL version.";
            return false;
        }   

        return parseVerifyValidSizeValueComparison();
    }

    if (leftInfo.m_refType != ParseFieldImpl::FieldRefType_Field) {
        reportInvalidSiblingRef(m_left);
        return false;
    }

    if (m_right[0] == Deref) {
        auto rightInfo = ParseFieldImpl::parseProcessSiblingRef(fields, m_right.substr(1));

        if ((rightInfo.m_field == nullptr) || 
            (rightInfo.m_refType != ParseFieldImpl::FieldRefType_Field)) {
            reportInvalidSiblingRef(m_right);
            return false;
        }         

        if (!leftInfo.m_field->parseIsComparableToField(*rightInfo.m_field)) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        auto& schema = protocol.parseCurrSchema();
        auto rightFields = schema.parseProcessInterfaceFieldRef(m_right.substr(1));
        parseDiscardNonFieldReferences(rightFields);

        if (rightFields.empty()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression for this condition.";        
            return false;
        }        

        for (auto& fieldInfo : rightFields) {
            assert(fieldInfo.m_field != nullptr);
            if (leftInfo.m_field->parseIsComparableToField(*fieldInfo.m_field)) {
                return true;
            }
        }

        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "The \"" << m_right << "\" string is expected to dereference existing field in any \"" <<
            common::interfaceStr() << "\"";            

        return false;
    }    

    if (!leftInfo.m_field->parseIsComparableToValue(m_right)) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }

    return true;
}

bool ParseOptCondExprImpl::parseVerifyInterfaceComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    assert(m_left[0] == IfaceDeref);

    auto& logger = protocol.parseLogger();
    auto& schema = protocol.parseCurrSchema();
    auto leftFields = schema.parseProcessInterfaceFieldRef(m_left.substr(1));

    auto leftSizeChecks = leftFields;
    parseDiscardNonSizeReferences(leftSizeChecks);

    if (!leftSizeChecks.empty()) {
        if (!protocol.parseIsSizeCompInConditionalsSupported()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The size comparison check in the optional condition is not supported for the selected DSL version.";
            return false;
        }  
                
        return parseVerifyValidSizeValueComparison();
    }

    parseDiscardNonFieldReferences(leftFields);

    if (leftFields.empty()) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "The \"" << m_left << "\" is not valid field dereference expression for this condition.";        
        return false;
    }

    auto& leftInfo = leftFields.front();
    if (leftInfo.m_refType != ParseFieldImpl::FieldRefType_Field) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << m_left << "\" string is expected to dereference existing interface field.";        
        return false;
    }    

    if (m_right[0] == Deref) {
        auto rightInfo = ParseFieldImpl::parseProcessSiblingRef(fields, m_right.substr(1));
        if ((rightInfo.m_field == nullptr) || 
            (rightInfo.m_refType != ParseFieldImpl::FieldRefType_Field)) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << m_right << "\" string is expected to dereference existing sibling field.";
            return false;
        }    

        bool hasComparable = 
            std::any_of(
                leftFields.begin(), leftFields.end(),
                [rightInfo](auto& fieldInfo)
                {
                    return fieldInfo.m_field->parseIsComparableToField(*rightInfo.m_field);
                });

        if (!hasComparable) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        auto rightFields = schema.parseProcessInterfaceFieldRef(m_right.substr(1));
        parseDiscardNonFieldReferences(rightFields);

        if (rightFields.empty()) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression for this condition.";        
            return false;
        }             

        bool hasComparable = 
            std::any_of(
                leftFields.begin(), leftFields.end(),
                [&rightFields](auto& leftFieldInfo)
                {
                    assert(leftFieldInfo.m_field != nullptr);
                    return std::any_of(
                        rightFields.begin(), rightFields.end(),
                        [&leftFieldInfo](auto& rightFieldInfo)
                        {
                            assert(rightFieldInfo.m_field != nullptr);
                            return leftFieldInfo.m_field->parseIsComparableToField(*rightFieldInfo.m_field);
                        });
                });  

        if (!hasComparable) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }    

        return true;
    }

    bool hasComparable = 
        std::any_of(
            leftFields.begin(), leftFields.end(),
            [this](auto& fieldInfo)
            {
                assert(fieldInfo.m_field != nullptr);
                return fieldInfo.m_field->parseIsComparableToValue(m_right);
            });    

    if (!hasComparable) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }

    return true;    
}

bool ParseOptCondExprImpl::parseVerifyValidSizeValueComparison() const
{
    try {
        [[maybe_unused]] auto val = std::stoll(m_right);
        return true;
    } catch (...) {
        // Do nothing
    }

    return false;
}

ParseOptCondListImpl::ParseOptCondListImpl(const ParseOptCondListImpl& other)
  : Base(other),
    m_type(other.m_type)
{
    m_conds.reserve(other.m_conds.size());
    for (auto& c : other.m_conds) {
        m_conds.push_back(c->parseClone());
    }
}

ParseOptCondListImpl::CondList ParseOptCondListImpl::parseCondList() const
{
    CondList result;
    result.reserve(m_conds.size());
    for (auto& c : m_conds) {
        assert(c);
        result.emplace_back(c.get());
    }
    return result;
}

bool ParseOptCondListImpl::parse(xmlNodePtr node, const ParseProtocolImpl& protocol)
{
    static const std::string CondMap[] = {
        common::andStr(),
        common::orStr()
    };

    static const std::size_t CondMapSize = std::extent<decltype(CondMap)>::value;

    static_assert(CondMapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");
    static_assert(0U == util::toUnsigned(Type::And), "Invalid map");
    static_assert(1U == util::toUnsigned(Type::Or), "Invalid map");

    auto& logger = protocol.parseLogger();
    std::string elemName(reinterpret_cast<const char*>(node->name));
    auto iter = std::find(std::begin(CondMap), std::end(CondMap), elemName);
    if (iter == std::end(CondMap)) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "Unknown condition type \"" << elemName << "\".";
        return false;
    }

    m_type = static_cast<decltype(m_type)>(iter - std::begin(CondMap));

    auto children = ParseXmlWrap::parseGetChildren(node);
    assert(m_conds.empty());
    for (auto c : children) {
        std::string childName(reinterpret_cast<const char*>(c->name));
        if (childName == condStr()) {
            std::string expr;
            if (!ParseXmlWrap::parseNodeValue(c, logger, expr, true)) {
                return false;
            }

            auto cond = std::make_unique<ParseOptCondExprImpl>();
            cond->parseOverrideCondStr(condStr());

            if (!cond->parse(expr, c, protocol)) {
                return false;
            }

            m_conds.push_back(std::move(cond));
            continue;
        }

        auto multiIter = std::find(std::begin(CondMap), std::end(CondMap), childName);
        if (multiIter == std::end(CondMap)) {
            parseLogError(logger) << ParseXmlWrap::parseLogPrefix(c) <<
                "Unknown element inside \"" << elemName << "\" condition bundling";
            return false;
        }

        auto multiCond = std::make_unique<ParseOptCondListImpl>();
        multiCond->parseOverrideCondStr(condStr());

        if (!multiCond->parse(c, protocol)) {
            return false;
        }

        m_conds.push_back(std::move(multiCond));
    }

    if (m_conds.size() < 2U) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
            "Condition bundling element \"" << elemName << "\" is expected to have at least "
            "2 conditions.";
        return false;
    }

    return true;
}

ParseOptCondImpl::Kind ParseOptCondListImpl::parseKindImpl() const
{
    return Kind::List;
}

ParseOptCondImpl::Ptr ParseOptCondListImpl::parseCloneImpl() const
{
    return Ptr(new ParseOptCondListImpl(*this));
}

bool ParseOptCondListImpl::parseVerifyImpl(const ParseOptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
{
    return std::all_of(
        m_conds.begin(), m_conds.end(),
        [&fields, node, &protocol](auto& c)
        {
            return c->parseVerify(fields, node, protocol);
        });
}

bool ParseOptCondListImpl::parseHasInterfaceReferenceImpl() const
{
    return std::any_of(
        m_conds.begin(), m_conds.end(),
        [](auto& c)
        {
            return c->parseHasInterfaceReference();
        });
}

} // namespace parse

} // namespace commsdsl
