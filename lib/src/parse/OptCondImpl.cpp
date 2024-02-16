//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "OptCondImpl.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include "BitfieldFieldImpl.h"
#include "BundleFieldImpl.h"
#include "Logger.h"
#include "ProtocolImpl.h"
#include "RefFieldImpl.h"
#include "SchemaImpl.h"
#include "SetFieldImpl.h"
#include "common.h"
#include "util.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const char Esc = '\\';
const char Deref = common::siblingRefPrefix();
const char IfaceDeref = common::interfaceRefPrefix();

void discardNonSizeReferences(FieldImpl::FieldRefInfosList& infos)
{
    infos.erase(
        std::remove_if(
            infos.begin(), infos.end(),
            [](auto& fieldInfo)
            {
                return fieldInfo.m_refType != FieldImpl::FieldRefType_Size;
            }),
        infos.end());
}


void discardNonFieldReferences(FieldImpl::FieldRefInfosList& infos)
{
    infos.erase(
        std::remove_if(
            infos.begin(), infos.end(),
            [](auto& fieldInfo)
            {
                return fieldInfo.m_refType != FieldImpl::FieldRefType_Field;
            }),
        infos.end());
}

OptCondExprImpl::OperandInfo operandInfoInternal(const std::string& val)
{
    OptCondExprImpl::OperandInfo result;
    if (val.empty()) {
        return result;
    }

    if ((val[0] != common::siblingRefPrefix()) && (val[0] != common::interfaceRefPrefix())) {
        result.m_type = OptCondExprImpl::OperandType::Value;
        result.m_access = val;
        return result;
    }

    if (val[0] == common::siblingRefPrefix()) {
        result.m_type = OptCondExprImpl::OperandType::SiblingRef;
    }
    else {
        assert(val[0] == common::interfaceRefPrefix());
        result.m_type = OptCondExprImpl::OperandType::InterfaceRef;
    }

    auto restPos = 1U;
    do {
        if (val.size() <= 1U) {
            break;
        }

        if (val[restPos] == '#') {
            result.m_mode = OptCondExprImpl::AccMode::Size;
            ++restPos;
            break;
        }   

        if (val[restPos] == '?') {
            result.m_mode = OptCondExprImpl::AccMode::Exists;
            ++restPos;
            break;
        }             

    } while (false);

    result.m_access = val.substr(restPos);
    return result;
}

} // namespace

OptCondImpl::OptCondImpl() :
    m_condStr(common::condStr())
{
}

bool OptCondExprImpl::parse(const std::string& expr, ::xmlNodePtr node, const ProtocolImpl& protocol)
{
    auto& logger = protocol.logger();
    if (expr.empty()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid condition expression";
        return false;
    }

    assert(!hasUpdatedValue());
    return
        checkComparison(expr, "!=", node, protocol) &&
        checkComparison(expr, ">=", node, protocol) &&
        checkComparison(expr, "<=", node, protocol) &&
        checkComparison(expr, "=", node, protocol) &&
        checkComparison(expr, ">", node, protocol) &&
        checkComparison(expr, "<", node, protocol) &&
        checkBool(expr, node, protocol) &&
        hasUpdatedValue();
}

OptCondExprImpl::OperandInfo OptCondExprImpl::leftInfo() const
{
    return operandInfoInternal(m_left);
}

OptCondExprImpl::OperandInfo OptCondExprImpl::rightInfo() const
{
    return operandInfoInternal(m_right);
}

OptCondImpl::Kind OptCondExprImpl::kindImpl() const
{
    return Kind::Expr;
}

OptCondImpl::Ptr OptCondExprImpl::cloneImpl() const
{
    return Ptr(new OptCondExprImpl(*this));
}

bool OptCondExprImpl::verifyImpl(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    if (m_left.empty()) {
        return verifySingleElementCheck(fields, node, protocol);
    }

    return verifyComparison(fields, node, protocol);
}

bool OptCondExprImpl::hasUpdatedValue()
{
    return (!m_left.empty()) ||
           (!m_right.empty()) ||
           (!m_op.empty());
}

bool OptCondExprImpl::checkComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, const ProtocolImpl& protocol)
{
    if (hasUpdatedValue()) {
        return true;
    }

    auto& logger = protocol.logger();
    auto reportInvalidExprFunc =
        [node, &logger, &expr]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
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
        if (!protocol.isInterfaceFieldReferenceSupported()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "References to the interface fields are not supported in the selected " << common::dslVersionStr() << ".";
            return false;            
        }
        return true;
    }

    logError(logger) << XmlWrap::logPrefix(node) <<
        "Invalid \"" << condStr() << "\" expression, left side of "
        "comparison operator must dereference other field.";
    return false;
}

bool OptCondExprImpl::checkBool(const std::string& expr, ::xmlNodePtr node, const ProtocolImpl& protocol)
{
    if (hasUpdatedValue()) {
        return true;
    }

    auto& logger = protocol.logger();
    auto reportInvalidExprFunc =
        [node, &logger, &expr]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << expr << "\" string is not valid condition expression.";
        };

    assert(!expr.empty());
    if (expr[0] == Deref) {
        m_right = expr;
        return true;
    }

    if (expr[0] == IfaceDeref) {
        if (!protocol.isInterfaceFieldReferenceSupported()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
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
            if (!protocol.isInterfaceFieldReferenceSupported()) {
                logError(logger) << XmlWrap::logPrefix(node) <<
                    "References to the interface fields are not supported in the selected " << common::dslVersionStr() << ".";
                return false;            
            }        
            break;
        }          

        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << condStr() << "\" expression, "
            "the check must dereference other field.";
        return false;        

    } while (false);

    m_op = "!";
    m_right.assign(expr.begin() + valPos, expr.end());
    return true;
}

bool OptCondExprImpl::verifySingleElementCheck(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    if (m_right[0] == Deref) {
        return verifySiblingSingleElementCheck(fields, node, protocol);
    }

    assert(m_right[0] == IfaceDeref);
    return verifyInterfaceBitCheck(node, protocol);
}

bool OptCondExprImpl::verifySiblingSingleElementCheck(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == Deref);

    auto& logger = protocol.logger();
    auto reportInvalidSiblingRef = 
        [node, &logger](const std::string& refStr)
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << refStr << "\" string is invalid sibling reference.";

        };

    auto info = FieldImpl::processSiblingRef(fields, m_right.substr(1));
    if (info.m_field == nullptr) {
        reportInvalidSiblingRef(m_right);
        return false;
    }

    if (info.m_refType == FieldImpl::FieldRefType_InnerValue) {
        assert(!info.m_valueName.empty());
        return true;
    }

    if (info.m_refType != FieldImpl::FieldRefType_Exists) {
        reportInvalidSiblingRef(m_right);
        return false;
    }

    if (!protocol.isExistsCheckInConditionalsSupported()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The optional mode check in the optional condition is not supported for the selected DSL version.";
        return false;
    }

    return true;
}

bool OptCondExprImpl::verifyInterfaceBitCheck(::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == IfaceDeref);

    auto& schema = protocol.currSchema();
    auto foundFields = schema.processInterfaceFieldRef(m_right.substr(1));
    auto hasValidRef = 
        std::any_of(
            foundFields.begin(), foundFields.end(),
            [](auto& info)
            {
                assert(info.m_field != nullptr);
                return 
                    (info.m_refType == FieldImpl::FieldRefType_InnerValue) &&
                    (info.m_field->kind() == FieldImpl::Kind::Set);
            });

    if (hasValidRef) {
        return true;
    }

    auto& logger = protocol.logger();
    logError(logger) << XmlWrap::logPrefix(node) <<
        "The \"" << m_right << "\" string is expected to dereference existing bit in existing <" <<
        common::setStr() << "> field or <" << common::refStr() << "> to it in one of the <" << common::interfaceStr() << ">-es.";
    return false;
}

bool OptCondExprImpl::verifyComparison(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    if (m_left[0] == Deref) {
        return verifySiblingComparison(fields, node, protocol);
    }

    assert(m_left[0] == IfaceDeref);
    return verifyInterfaceComparison(fields, node, protocol);
}

bool OptCondExprImpl::verifySiblingComparison(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    assert(m_left[0] == Deref);

    auto& logger = protocol.logger();
    auto reportInvalidSiblingRef = 
        [node, &logger](const std::string& refStr)
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << refStr << "\" string is expected to dereference existing sibling field.";

        };

    auto leftInfo = FieldImpl::processSiblingRef(fields, m_left.substr(1));
    if (leftInfo.m_field == nullptr) {
        reportInvalidSiblingRef(m_left);
        return false;
    } 

    if (leftInfo.m_refType == FieldImpl::FieldRefType_Size) {
        if (!protocol.isSizeCompInConditionalsSupported()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The size comparison check in the optional condition is not supported for the selected DSL version.";
            return false;
        }   

        return verifyValidSizeValueComparison();
    }

    if (leftInfo.m_refType != FieldImpl::FieldRefType_Field) {
        reportInvalidSiblingRef(m_left);
        return false;
    }

    if (m_right[0] == Deref) {
        auto rightInfo = FieldImpl::processSiblingRef(fields, m_right.substr(1));

        if ((rightInfo.m_field == nullptr) || 
            (rightInfo.m_refType != FieldImpl::FieldRefType_Field)) {
            reportInvalidSiblingRef(m_right);
            return false;
        }         

        if (!leftInfo.m_field->isComparableToField(*rightInfo.m_field)) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        auto& schema = protocol.currSchema();
        auto rightFields = schema.processInterfaceFieldRef(m_right.substr(1));
        discardNonFieldReferences(rightFields);

        if (rightFields.empty()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression for this condition.";        
            return false;
        }        

        for (auto& fieldInfo : rightFields) {
            assert(fieldInfo.m_field != nullptr);
            if (leftInfo.m_field->isComparableToField(*fieldInfo.m_field)) {
                return true;
            }
        }

        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_right << "\" string is expected to dereference existing field in any \"" <<
            common::interfaceStr() << "\"";            

        return false;
    }    

    if (!leftInfo.m_field->isComparableToValue(m_right)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }

    return true;
}

bool OptCondExprImpl::verifyInterfaceComparison(const FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    assert(m_left[0] == IfaceDeref);

    auto& logger = protocol.logger();
    auto& schema = protocol.currSchema();
    auto leftFields = schema.processInterfaceFieldRef(m_left.substr(1));

    auto leftSizeChecks = leftFields;
    discardNonSizeReferences(leftSizeChecks);

    if (!leftSizeChecks.empty()) {
        if (!protocol.isSizeCompInConditionalsSupported()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The size comparison check in the optional condition is not supported for the selected DSL version.";
            return false;
        }  
                
        return verifyValidSizeValueComparison();
    }

    discardNonFieldReferences(leftFields);

    if (leftFields.empty()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_left << "\" is not valid field dereference expression for this condition.";        
        return false;
    }

    auto& leftInfo = leftFields.front();
    if (leftInfo.m_refType != FieldImpl::FieldRefType_Field) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_left << "\" string is expected to dereference existing interface field.";        
        return false;
    }    

    if (m_right[0] == Deref) {
        auto rightInfo = FieldImpl::processSiblingRef(fields, m_right.substr(1));
        if ((rightInfo.m_field == nullptr) || 
            (rightInfo.m_refType != FieldImpl::FieldRefType_Field)) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" string is expected to dereference existing sibling field.";
            return false;
        }    

        bool hasComparable = 
            std::any_of(
                leftFields.begin(), leftFields.end(),
                [rightInfo](auto& fieldInfo)
                {
                    return fieldInfo.m_field->isComparableToField(*rightInfo.m_field);
                });

        if (!hasComparable) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        auto rightFields = schema.processInterfaceFieldRef(m_right.substr(1));
        discardNonFieldReferences(rightFields);

        if (rightFields.empty()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
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
                            return leftFieldInfo.m_field->isComparableToField(*rightFieldInfo.m_field);
                        });
                });  

        if (!hasComparable) {
            logError(logger) << XmlWrap::logPrefix(node) <<
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
                return fieldInfo.m_field->isComparableToValue(m_right);
            });    

    if (!hasComparable) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }

    return true;    
}

bool OptCondExprImpl::verifyValidSizeValueComparison() const
{
    try {
        [[maybe_unused]] auto val = std::stoll(m_right);
        return true;
    } catch (...) {
        // Do nothing
    }

    return false;
}

OptCondListImpl::OptCondListImpl(const OptCondListImpl& other)
  : Base(other),
    m_type(other.m_type)
{
    m_conds.reserve(other.m_conds.size());
    for (auto& c : other.m_conds) {
        m_conds.push_back(c->clone());
    }
}

OptCondListImpl::CondList OptCondListImpl::condList() const
{
    CondList result;
    result.reserve(m_conds.size());
    for (auto& c : m_conds) {
        assert(c);
        result.emplace_back(c.get());
    }
    return result;
}

bool OptCondListImpl::parse(xmlNodePtr node, const ProtocolImpl& protocol)
{
    static const std::string CondMap[] = {
        common::andStr(),
        common::orStr()
    };

    static const std::size_t CondMapSize = std::extent<decltype(CondMap)>::value;

    static_assert(CondMapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");
    static_assert(0U == util::toUnsigned(Type::And), "Invalid map");
    static_assert(1U == util::toUnsigned(Type::Or), "Invalid map");

    auto& logger = protocol.logger();
    std::string elemName(reinterpret_cast<const char*>(node->name));
    auto iter = std::find(std::begin(CondMap), std::end(CondMap), elemName);
    if (iter == std::end(CondMap)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Unknown condition type \"" << elemName << "\".";
        return false;
    }

    m_type = static_cast<decltype(m_type)>(iter - std::begin(CondMap));

    auto children = XmlWrap::getChildren(node);
    assert(m_conds.empty());
    for (auto c : children) {
        std::string childName(reinterpret_cast<const char*>(c->name));
        if (childName == condStr()) {
            std::string expr;
            if (!XmlWrap::parseNodeValue(c, logger, expr, true)) {
                return false;
            }

            auto cond = std::make_unique<OptCondExprImpl>();
            cond->overrideCondStr(condStr());

            if (!cond->parse(expr, c, protocol)) {
                return false;
            }

            m_conds.push_back(std::move(cond));
            continue;
        }

        auto multiIter = std::find(std::begin(CondMap), std::end(CondMap), childName);
        if (multiIter == std::end(CondMap)) {
            logError(logger) << XmlWrap::logPrefix(c) <<
                "Unknown element inside \"" << elemName << "\" condition bundling";
            return false;
        }

        auto multiCond = std::make_unique<OptCondListImpl>();
        multiCond->overrideCondStr(condStr());

        if (!multiCond->parse(c, protocol)) {
            return false;
        }

        m_conds.push_back(std::move(multiCond));
    }

    if (m_conds.size() < 2U) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Condition bundling element \"" << elemName << "\" is expected to have at least "
            "2 conditions.";
        return false;
    }

    return true;
}

OptCondImpl::Kind OptCondListImpl::kindImpl() const
{
    return Kind::List;
}

OptCondImpl::Ptr OptCondListImpl::cloneImpl() const
{
    return Ptr(new OptCondListImpl(*this));
}

bool OptCondListImpl::verifyImpl(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    return std::all_of(
        m_conds.begin(), m_conds.end(),
        [&fields, node, &protocol](auto& c)
        {
            return c->verify(fields, node, protocol);
        });
}



} // namespace parse

} // namespace commsdsl
