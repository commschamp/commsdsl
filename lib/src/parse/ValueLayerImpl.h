//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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
#include "InterfaceImpl.h"

namespace commsdsl
{

namespace parse
{

class ValueLayerImpl final : public LayerImpl
{
    using Base = LayerImpl;
public:
    using Interfaces = std::vector<const InterfaceImpl*>;
    using InterfacesList = ValueLayer::Interfaces;
    ValueLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    InterfacesList interfacesList() const;

    const std::string& fieldName() const
    {
        return *m_fieldName;
    }

    std::size_t fieldIdx() const;

    bool pseudo() const
    {
        return m_pseudo;
    }


protected:
    virtual Kind kindImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool verifyImpl(const LayersList& layers) override;

private:
    bool updateInterfaces();
    bool updateFieldName();
    bool updatePseudo();

    Interfaces m_interfaces;
    const std::string* m_fieldName = nullptr;
    bool m_pseudo = false;
};

} // namespace parse

} // namespace commsdsl
