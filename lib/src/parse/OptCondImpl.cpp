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

//#include <iostream>

namespace commsdsl
{

namespace parse
{

namespace
{

const char Esc = '\\';
const char Deref = common::siblingRefPrefix();
const char IfaceDeref = common::interfaceRefPrefix();

FieldImpl* findSiblingField(
    const OptCondImpl::FieldsList& fields,
    const std::string& name,
    std::size_t& remPos)
{
    auto dotPos = name.find_first_of('.', remPos);
    std::string fieldName(name, remPos, dotPos - remPos);
    if (fieldName.empty()) {
        return nullptr;
    }

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&fieldName](auto& f)
            {
                return f->name() == fieldName;
            });

    if (iter == fields.end()) {
        return nullptr;
    }

    if (dotPos == std::string::npos) {
        remPos = dotPos;
        return iter->get();
    }

    remPos = dotPos + 1;
    auto redirectFunc =
        [&name, &remPos](const auto& f)
        {
            auto& members = f.members();
            return findSiblingField(members, name, remPos);
        };

    assert(*iter != nullptr);

    const auto* derefField = (*iter).get();
    while (derefField->kind() == FieldImpl::Kind::Ref) {
        auto& refField = static_cast<const RefFieldImpl&>(*derefField);
        derefField = refField.fieldImpl();
        assert(derefField != nullptr);
    }

    auto fieldKind = derefField->kind();
    if (fieldKind == FieldImpl::Kind::Bundle) {
        return redirectFunc(static_cast<const BundleFieldImpl&>(*derefField));
    }

    if (fieldKind == FieldImpl::Kind::Bitfield) {
        return redirectFunc(static_cast<const BitfieldFieldImpl&>(*derefField));
    }

    return iter->get();
}

using FoundFieldInfo = std::pair<const FieldImpl*, std::size_t>;
using FoundFieldInfosList = std::vector<FoundFieldInfo>;

FoundFieldInfosList findInterfaceFields(const ProtocolImpl& protocol, const std::string& name, std::size_t remPos)
{
    FoundFieldInfosList result;
    do {
        auto dotPos = name.find_first_of('.', remPos);
        std::string fieldName(name, remPos, dotPos - remPos);
        if (fieldName.empty()) {
            break;
        }

        if (dotPos == std::string::npos) {
            remPos = dotPos;
        }
        else {
            remPos = dotPos + 1U;
        }
        
        auto& schema = protocol.currSchema();
        auto allInterfaces = schema.allImplInterfaces();
        for (auto* i : allInterfaces) {
            auto fields = i->allImplFields();
            auto iter = 
                std::find_if(
                    fields.begin(), fields.end(),
                    [&fieldName](auto* f)
                    {
                        return fieldName == f->name();
                    });

            if (iter == fields.end()) {
                continue;
            }

            auto remName = name.substr(remPos);;
            auto redirectFunc =
                [&result, &remName, remPos](const auto& f)
                {
                    auto& members = f.members();
                    std::size_t remPosTmp = 0U;
                    auto* field = findSiblingField(members, remName, remPosTmp);
                    if (field == nullptr) {
                        return;
                    }

                    result.push_back(std::make_pair(field, remPos + remPosTmp));
                };            

            const auto* derefField = (*iter);
            while (derefField->kind() == FieldImpl::Kind::Ref) {
                auto& refField = static_cast<const RefFieldImpl&>(*derefField);
                derefField = refField.fieldImpl();
                assert(derefField != nullptr);
            }

            auto fieldKind = derefField->kind();
            if (fieldKind == FieldImpl::Kind::Bundle) {
                redirectFunc(static_cast<const BundleFieldImpl&>(*derefField));
                continue;
            }

            if (fieldKind == FieldImpl::Kind::Bitfield) {
                redirectFunc(static_cast<const BitfieldFieldImpl&>(*derefField));
                continue;
            }            

            result.push_back(std::make_pair(*iter, remPos));
        }

    } while (false);

    return result;
}

void discardNonFullReference(FoundFieldInfosList& fieldsInfos, std::size_t maxSize)
{
    fieldsInfos.erase(
        std::remove_if(
            fieldsInfos.begin(), fieldsInfos.end(),
            [maxSize](auto& fieldInfo)
            {
                return (fieldInfo.second < maxSize);
            }),
        fieldsInfos.end());
}

// void discardNonFieldReferences(FieldImpl::FieldRefInfosList& infos)
// {
//     infos.erase(
//         std::remove_if(
//             infos.begin(), infos.end(),
//             [](auto& fieldInfo)
//             {
//                 return !fieldInfo.m_valueName.empty();
//             }),
//         infos.end());
// }

} // namespace

bool OptCondExprImpl::parse(const std::string& expr, ::xmlNodePtr node, const ProtocolImpl& protocol)
{
    auto& logger = protocol.logger();
    if (expr.empty()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression";
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
        return verifyBitCheck(fields, node, protocol);
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
    auto reportInvalidExrFunc =
        [node, &logger]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Invalid \"" << common::condStr() << "\" expression";
        };

    std::size_t opPos = 0U;
    while (true) {
        opPos = expr.find(op, opPos);
        if (opPos == 0U) {
            reportInvalidExrFunc();
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
        reportInvalidExrFunc();
        return false;
    }

    auto rightBegPos = expr.find_first_not_of(' ', opPos + op.size());
    if (rightBegPos == std::string::npos) {
        reportInvalidExrFunc();
        return false;
    }

    m_left.assign(expr.begin(), expr.begin() + leftEndPos + 1U);
    m_op = op;
    m_right.assign(expr.begin() + rightBegPos, expr.end());

    if (m_left.empty() || m_right.empty()) {
        reportInvalidExrFunc();
        return false;
    }

    if ((m_left[0] != Deref) &&
        (m_left[0] != IfaceDeref)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression, left side of "
            "comparison operator must dereference other field.";
        return false;
    }
    

    return true;
}

bool OptCondExprImpl::checkBool(const std::string& expr, ::xmlNodePtr node, const ProtocolImpl& protocol)
{
    if (hasUpdatedValue()) {
        return true;
    }

    auto& logger = protocol.logger();
    auto reportInvalidExrFunc =
        [node, &logger]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Invalid \"" << common::condStr() << "\" expression";
        };

    assert(!expr.empty());
    if ((expr[0] == Deref) ||
        (expr[0] == IfaceDeref)) {
        m_right = expr;
        return true;
    }

    if (expr[0] != '!') {
        reportInvalidExrFunc();
        return false;
    }

    auto valPos = expr.find_first_not_of(' ', 1);
    if (valPos == std::string::npos) {
        reportInvalidExrFunc();
        return false;
    }

    if ((expr[valPos] != Deref) &&
        (expr[valPos] != IfaceDeref)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression, "
            "the check must dereference other field.";
        return false;
    }

    m_op = "!";
    m_right.assign(expr.begin() + valPos, expr.end());
    return true;
}

bool OptCondExprImpl::verifyBitCheck(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    if (m_right[0] == Deref) {
        return verifySiblingBitCheck(fields, node, protocol);
    }

    assert(m_right[0] == IfaceDeref);
    return verifyInterfaceBitCheck(node, protocol);
}

bool OptCondExprImpl::verifySiblingBitCheck(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == Deref);

    std::size_t remPos = 1;
    auto field = findSiblingField(fields, m_right, remPos);
    do {
        if (field == nullptr) {
            break;
        }

        std::string bitName(m_right, remPos);
        if (!field->isBitCheckable(bitName)) {
            break;
        }

        return true;
    } while (false);

    auto& logger = protocol.logger();
    logError(logger) << XmlWrap::logPrefix(node) <<
        "The \"" << m_right << "\" string is expected to dereference existing bit in existing <" <<
        common::setStr() << "> field or <" << common::refStr() << "> to it.";
    return false;
}

bool OptCondExprImpl::verifyInterfaceBitCheck(::xmlNodePtr node, const ProtocolImpl& protocol) const
{
    assert(!m_right.empty());
    assert(m_right[0] == IfaceDeref);

    std::size_t remPos = 1;
    auto fields = findInterfaceFields(protocol, m_right, remPos);

    for (auto& fInfo : fields) {
            std::string bitName(m_right, fInfo.second);
        if (fInfo.first->isBitCheckable(bitName)) {
            return true;
        }
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
    std::size_t remPos = 1;
    auto field = findSiblingField(fields, m_left, remPos);
    if (field == nullptr) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_left << "\" string is expected to dereference existing field in the containing \"" <<
            common::bundleStr() << "\" or \"" << common::messageStr() << "\"";
        return false;
    }

    if (remPos < m_left.size()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_left << "\" is not valid field dereference expression.";        
        return false;
    }

    if (m_right[0] == Deref) {
        std::size_t rightRemPos = 1U;
        auto rightField = findSiblingField(fields, m_right, rightRemPos);
        if (rightField == nullptr) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" string is expected to dereference existing field in the containing \"" <<
                common::bundleStr() << "\" or \"" << common::messageStr() << "\"";
            return false;
        }

        if (rightRemPos < m_right.size()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression.";        
            return false;
        }        

        if (!field->isComparableToField(*rightField)) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        std::size_t rightRemPos = 1U;
        auto allFields = findInterfaceFields(protocol, m_right, rightRemPos);
        discardNonFullReference(allFields, m_right.size());

        if (allFields.empty()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression.";        
            return false;
        }        

        for (auto& fieldInfo : allFields) {
            assert(fieldInfo.first != nullptr);
            if (field->isComparableToField(*fieldInfo.first)) {
                return true;
            }
        }

        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_right << "\" string is expected to dereference existing field in any \"" <<
            common::interfaceStr() << "\"";            

        return false;
    }    

    if (!field->isComparableToValue(m_right)) {
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
    std::size_t remPos = 1;
    auto allFields = findInterfaceFields(protocol, m_left, remPos);
    discardNonFullReference(allFields, m_left.size());

    if (allFields.empty()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_left << "\" is not valid field dereference expression.";        
        return false;
    }

    if (m_right[0] == Deref) {
        std::size_t rightRemPos = 1U;
        auto* rightField = findSiblingField(fields, m_right, rightRemPos);
        if (rightField == nullptr) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" string is expected to dereference existing field in the containing \"" <<
                common::bundleStr() << "\" or \"" << common::messageStr() << "\"";
            return false;
        }

        if (rightRemPos < m_right.size()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression.";        
            return false;
        }        

        bool hasComparable = 
            std::any_of(
                allFields.begin(), allFields.end(),
                [rightField](auto& fieldInfo)
                {
                    return fieldInfo.first->isComparableToField(*rightField);
                });

        if (!hasComparable) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (m_right[0] == IfaceDeref) {
        std::size_t rightRemPos = 1;
        auto allRightFields = findInterfaceFields(protocol, m_right, rightRemPos);        
        discardNonFullReference(allRightFields, m_right.size());

        if (allRightFields.empty()) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" is not valid field dereference expression.";        
            return false;
        }             

        bool hasComparable = 
            std::any_of(
                allFields.begin(), allFields.end(),
                [&allRightFields](auto& leftFieldInfo)
                {
                    assert(leftFieldInfo.first != nullptr);
                    return std::any_of(
                        allRightFields.begin(), allRightFields.end(),
                        [&leftFieldInfo](auto& rightFieldInfo)
                        {
                            assert(rightFieldInfo.first != nullptr);
                            return leftFieldInfo.first->isComparableToField(*rightFieldInfo.first);
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
            allFields.begin(), allFields.end(),
            [this](auto& fieldInfo)
            {
                return fieldInfo.first->isComparableToValue(m_right);
            });    

    if (!hasComparable) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }

    return true;    
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
        if (childName == common::condStr()) {
            std::string expr;
            if (!XmlWrap::parseNodeValue(c, logger, expr, true)) {
                return false;
            }

            auto cond = std::make_unique<OptCondExprImpl>();
            if (!cond->parse(expr, c, protocol)) {
                return false;
            }

            m_conds.push_back(std::move(cond));
            continue;
        }

        auto multiIter = std::find(std::begin(CondMap), std::end(CondMap), childName);
        if (multiIter == std::end(CondMap)) {
            logError(logger) << XmlWrap::logPrefix(c) <<
                "Unknown element inside \"" << elemName << "\' condition bundling";
            return false;
        }

        auto multiCond = std::make_unique<OptCondListImpl>();
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
