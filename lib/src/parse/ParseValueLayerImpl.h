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

#include "ParseInterfaceImpl.h"
#include "ParseLayerImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseValueLayerImpl final : public ParseLayerImpl
{
    using Base = ParseLayerImpl;
public:
    using ParseInterfaces = std::vector<const ParseInterfaceImpl*>;
    using ParseInterfacesList = ParseValueLayer::ParseInterfaces;
    ParseValueLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

    ParseInterfacesList parseInterfacesList() const;

    const std::string& parseFieldName() const
    {
        return *m_fieldName;
    }

    std::size_t parseFieldIdx() const;

    bool parsePseudo() const
    {
        return m_pseudo;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool parseVerifyImpl(const ParseLayersList& layers) override;

private:
    bool parseUpdateInterfaces();
    bool parseUpdateFieldName();
    bool parseUpdatePseudo();

    ParseInterfaces m_interfaces;
    const std::string* m_fieldName = nullptr;
    bool m_pseudo = false;
};

} // namespace parse

} // namespace commsdsl
