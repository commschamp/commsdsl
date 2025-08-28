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

#include "commsdsl/parse/ParseProtocol.h"

namespace commsdsl
{

namespace parse
{

class ParseObject
{
public:
    enum class ParseObjKind
    {
        Namespace,
        Field,
        Message,
        Interface,
        Frame,
        Layer,
        Schema,
        NumOfValues
    };

    ParseObject* parseGetParent()
    {
        return m_parent;
    }

    const ParseObject* parseGetParent() const
    {
        return m_parent;
    }

    void parseSetParent(ParseObject* obj)
    {
        m_parent = obj;
    }

    ParseObjKind parseObjKind() const
    {
        return parseObjKindImpl();
    }

    unsigned parseGetSinceVersion() const
    {
        return m_rState.m_sinceVersion;
    }

    void parseSetSinceVersion(unsigned val)
    {
        m_rState.m_sinceVersion = val;
    }

    unsigned parseGetDeprecated() const
    {
        return m_rState.m_deprecated;
    }

    bool parseIsDeprecatedRemoved() const
    {
        return m_rState.m_deprecatedRemoved;
    }

protected:
    ParseObject() = default;
    ~ParseObject() = default;
    
    virtual ParseObjKind parseObjKindImpl() const = 0;

    void parseSetDeprecated(unsigned val)
    {
        m_rState.m_deprecated = val;
    }

    void parseSetDeprecatedRemoved(bool val)
    {
        m_rState.m_deprecatedRemoved = val;
    }

    void parseReuseState(const ParseObject& other)
    {
        m_rState = other.m_rState;
    }


private:
    struct ParseReusableState
    {
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecated = ParseProtocol::parseNotYetDeprecated();
        bool m_deprecatedRemoved = false;
    };

    ParseObject* m_parent = nullptr;
    ParseReusableState m_rState;
};

} // namespace parse

} // namespace commsdsl
