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

class ParseCustomLayerImpl final : public ParseLayerImpl
{
    using Base = ParseLayerImpl;
    
public:
    using ParseKind = ParseLayer::ParseKind;

    ParseCustomLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

    ParseKind parseSemanticLayerType() const
    {
        return m_sematicLayerType;
    }

    const std::string& parseChecksumFromLayer() const
    {
        return m_checksumFromLayer;
    }

    const std::string& parseChecksumUntilLayer() const
    {
        return m_checksumUntilLayer;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual bool parseImpl() override;
    virtual bool parseVerifyImpl(const ParseLayersList& layers) override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;

private:
    bool parseUpdateIdReplacement();
    bool parseUpdateSemanticLayerType();
    bool parseUpdateChecksumFrom();
    bool parseUpdateChecksumUntil();
    bool parseVerifyChecksumInternal(const ParseLayersList& layers);

    ParseKind m_sematicLayerType = ParseKind::Custom;
    std::string m_checksumFromLayer;
    std::string m_checksumUntilLayer;
};

} // namespace parse

} // namespace commsdsl
