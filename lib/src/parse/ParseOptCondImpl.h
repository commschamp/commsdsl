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

    Kind parseKind() const
    {
        return parseKindImpl();
    }

    Ptr parseClone() const
    {
        return parseCloneImpl();
    }

    bool parseVerify(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
    {
        return parseVerifyImpl(fields, node, protocol);
    }

    void parseOverrideCondStr(const std::string& str)
    {
        m_condStr = str;
    }

    bool parseHasInterfaceReference() const
    {
        return parseHasInterfaceReferenceImpl();
    }

protected:
    virtual Kind parseKindImpl() const = 0;
    virtual Ptr parseCloneImpl() const = 0;
    virtual bool parseVerifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const = 0;
    virtual bool parseHasInterfaceReferenceImpl() const = 0;

    const std::string& parseCondStr() const
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

    const std::string& parseLeft() const
    {
        return m_left;
    }

    const std::string& parseOp() const
    {
        return m_op;
    }

    const std::string& parseRight() const
    {
        return m_right;
    }

    OperandInfo parseLeftInfo() const;
    OperandInfo parseRightInfo() const;

protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual bool parseVerifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool parseHasInterfaceReferenceImpl() const override;

private:
    bool parseHasUpdatedValue();
    bool parseCheckComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool parseCheckBool(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool parseVerifySingleElementCheck(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifySiblingSingleElementCheck(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyInterfaceBitCheck(::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifySiblingComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyInterfaceComparison(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyValidSizeValueComparison() const;

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

    Type parseType() const
    {
        return m_type;
    }

    CondList parseCondList() const;

    bool parse(::xmlNodePtr node, const ParseProtocolImpl& protocol);



protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual bool parseVerifyImpl(const FieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool parseHasInterfaceReferenceImpl() const override;

private:
    List m_conds;
    Type m_type = Type::NumOfValues;
};

using ParseOptCondImplPtr = ParseOptCondImpl::Ptr;

} // namespace parse

} // namespace commsdsl
