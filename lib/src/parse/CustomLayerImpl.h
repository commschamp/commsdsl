//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "LayerImpl.h"

namespace commsdsl
{

namespace parse
{

class CustomLayerImpl final : public LayerImpl
{
    using Base = LayerImpl;
public:
    using Kind = Layer::Kind;

    CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    Kind semanticLayerType() const
    {
        return m_sematicLayerType;
    }

    const std::string& checksumFromLayer() const
    {
        return m_checksumFromLayer;
    }

    const std::string& checksumUntilLayer() const
    {
        return m_checksumUntilLayer;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual bool parseImpl() override;
    virtual bool verifyImpl(const LayersList& layers) override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;

private:
    bool updateIdReplacement();
    bool updateSemanticLayerType();
    bool updateChecksumFrom();
    bool updateChecksumUntil();
    bool verifyChecksumInternal(const LayersList& layers);

    Kind m_sematicLayerType = Kind::Custom;
    std::string m_checksumFromLayer;
    std::string m_checksumUntilLayer;
};

} // namespace parse

} // namespace commsdsl
