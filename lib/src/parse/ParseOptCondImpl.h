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

#include "commsdsl/parse/ParseOptCond.h"
#include "ParseFieldImpl.h"
#include "ParseXmlWrap.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseOptCondImpl
{
public:
    using ParsePtr = std::unique_ptr<ParseOptCondImpl>;
    using ParseKind = ParseOptCond::ParseKind;
    using ParseFieldsList = std::vector<ParseFieldImplPtr>;

    ParseOptCondImpl();
    ParseOptCondImpl(const ParseOptCondImpl&) = default;
    ParseOptCondImpl(ParseOptCondImpl&&) = default;
    virtual ~ParseOptCondImpl() = default;

    ParseKind parseKind() const
    {
        return parseKindImpl();
    }

    ParsePtr parseClone() const
    {
        return parseCloneImpl();
    }

    bool parseVerify(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const
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
    virtual ParseKind parseKindImpl() const = 0;
    virtual ParsePtr parseCloneImpl() const = 0;
    virtual bool parseVerifyImpl(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const = 0;
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
    using ParseOperandType = ParseOptCondExpr::ParseOperandType;
    using ParseAccMode = ParseOptCondExpr::ParseAccMode;
    using ParseOperandInfo = ParseOptCondExpr::ParseOperandInfo;

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

    ParseOperandInfo parseLeftInfo() const;
    ParseOperandInfo parseRightInfo() const;

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual bool parseVerifyImpl(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool parseHasInterfaceReferenceImpl() const override;

private:
    bool parseHasUpdatedValue();
    bool parseCheckComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool parseCheckBool(const std::string& expr, ::xmlNodePtr node, const ParseProtocolImpl& protocol);
    bool parseVerifySingleElementCheck(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifySiblingSingleElementCheck(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyInterfaceBitCheck(::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyComparison(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifySiblingComparison(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyInterfaceComparison(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const;
    bool parseVerifyValidSizeValueComparison() const;

    std::string m_left;
    std::string m_op;
    std::string m_right;
};

class ParseOptCondListImpl final : public ParseOptCondImpl
{
    using Base = ParseOptCondImpl;
public:
    using ParseType = ParseOptCondList::ParseType;
    using ParseList = std::vector<ParsePtr>;
    using ParseCondList = ParseOptCondList::ParseCondList;

    ParseOptCondListImpl() = default;
    ParseOptCondListImpl(const ParseOptCondListImpl& other);
    ParseOptCondListImpl(ParseOptCondListImpl&&) = default;

    ParseType parseType() const
    {
        return m_type;
    }

    ParseCondList parseCondList() const;

    bool parse(::xmlNodePtr node, const ParseProtocolImpl& protocol);



protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual bool parseVerifyImpl(const ParseFieldsList& fields, ::xmlNodePtr node, const ParseProtocolImpl& protocol) const override;
    virtual bool parseHasInterfaceReferenceImpl() const override;

private:
    ParseList m_conds;
    ParseType m_type = ParseType::NumOfValues;
};

using ParseOptCondImplPtr = ParseOptCondImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
