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

#include "ParseLayerImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseChecksumLayerImpl final : public ParseLayerImpl
{
    using Base = ParseLayerImpl;
public:
    using ParseAlg = ParseChecksumLayer::ParseAlg;

    ParseChecksumLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

    ParseAlg parseAlg() const
    {
        return m_alg;
    }

    const std::string& parseAlgName() const
    {
        return *m_algName;
    }

    const std::string& parseFrom() const
    {
        return *m_from;
    }

    const std::string& parseUntil() const
    {
        return *m_until;
    }

    bool parseVerifyBeforeRead() const
    {
        return m_verifyBeforeRead;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool parseVerifyImpl(const ParseLayersList& layers) override;

private:
    bool parseUpdateAlg();
    bool parseUpdateFrom();
    bool parseUpdateUntil();
    bool parseUpdateVerifyBeforeRead();

    ParseAlg m_alg = ParseAlg::NumOfValues;
    const std::string* m_algName = nullptr;
    const std::string* m_from = nullptr;
    const std::string* m_until = nullptr;
    bool m_verifyBeforeRead = false;
};

} // namespace parse

} // namespace commsdsl
