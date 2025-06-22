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

#pragma once

#include <memory>
#include <vector>

#include "commsdsl/parse/ParseOptCond.h"
#include "ParseXmlWrap.h"
#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseOptCondImpl
{
public:
    using Ptr = std::unique_ptr<ParseOptCondImpl>;
    using Kind = ParseOptCond::Kind;
    using FieldsList = std::vector<ParseFieldImplPtr>;

    ParseOptCondImpl();
    ParseOptCondImpl(const ParseOptCondImpl&) = default;
    ParseOptCondImpl(ParseOptCondImpl&&) = default;
    virtual ~ParseOptCondImpl() = default;

    Kind kind() const
    {
        return kindImpl();
    }

    Ptr clone() const
    {
        return cloneImpl();
    }

    bool verify(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
    {
        return verifyImpl(fields, node, protocol);
    }

    void overrideCondStr(const std::string& str)
    {
        m_condStr = str;
    }

    bool hasInterfaceReference() const
    {
        return hasInterfaceReferenceImpl();
    }

protected:
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const = 0;
    virtual bool hasInterfaceReferenceImpl() const = 0;

    const std::string& condStr() const
    {
        return m_condStr;
    }

private:
    std::string m_condStr;    
};

class ParseOptCondExprImpl final: public ParseOptCondImpl
{
public:
    using OperandType = ParseOptCondExpr::OperandType;
    using AccMode = ParseOptCondExpr::AccMode;
    using OperandInfo = ParseOptCondExpr::OperandInfo;

    ParseOptCondExprImpl() = default;
    ParseOptCondExprImpl(const ParseOptCondExprImpl&) = default;
    ParseOptCondExprImpl(ParseOptCondExprImpl&&) = default;

    bool parse(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol);

    const std::string& left() const
    {
        return m_left;
    }

    const std::string& op() const
    {
        return m_op;
    }

    const std::string& right() const
    {
        return m_right;
    }

    OperandInfo leftInfo() const;
    OperandInfo rightInfo() const;

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool hasInterfaceReferenceImpl() const override;

private:
    bool hasUpdatedValue();
    bool checkComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool checkBool(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool verifySingleElementCheck(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifySiblingSingleElementCheck(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifyInterfaceBitCheck(::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifyComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifySiblingComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifyInterfaceComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool verifyValidSizeValueComparison() const;

    std::string m_left;
    std::string m_op;
    std::string m_right;
};

class ParseOptCondListImpl final : public ParseOptCondImpl
{
    using Base = ParseOptCondImpl;
public:
    using Type = ParseOptCondList::Type;
    using List = std::vector<Ptr>;
    using CondList = ParseOptCondList::CondList;

    ParseOptCondListImpl() = default;
    ParseOptCondListImpl(const ParseOptCondListImpl& other);
    ParseOptCondListImpl(ParseOptCondListImpl&&) = default;

    Type type() const
    {
        return m_type;
    }

    CondList condList() const;

    bool parse(::xmlNodePtr node, const ParseProtocolImpl& protocol);



protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool hasInterfaceReferenceImpl() const override;

private:
    List m_conds;
    Type m_type = Type::NumOfValues;
};

using ParseOptCondImplPtr = ParseOptCondImpl::Ptr;

} // namespace parse

} // namespace commsdsl
