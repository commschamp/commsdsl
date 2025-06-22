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

#include <limits>

#include "commsdsl/parse/ParseProtocol.h"

namespace commsdsl
{

namespace parse
{

class ParseObject
{
public:
    enum class ObjKind
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

    ParseObject* getParent()
    {
        return m_parent;
    }

    const ParseObject* getParent() const
    {
        return m_parent;
    }

    void setParent(ParseObject* obj)
    {
        m_parent = obj;
    }

    ObjKind objKind() const
    {
        return objKindImpl();
    }

    unsigned getSinceVersion() const
    {
        return m_rState.m_sinceVersion;
    }

    void setSinceVersion(unsigned val)
    {
        m_rState.m_sinceVersion = val;
    }

    unsigned getDeprecated() const
    {
        return m_rState.m_deprecated;
    }

    bool isDeprecatedRemoved() const
    {
        return m_rState.m_deprecatedRemoved;
    }

protected:
    ParseObject() = default;
    ~ParseObject() = default;
    
    virtual ObjKind objKindImpl() const = 0;

    void setDeprecated(unsigned val)
    {
        m_rState.m_deprecated = val;
    }

    void setDeprecatedRemoved(bool val)
    {
        m_rState.m_deprecatedRemoved = val;
    }

    void reuseState(const ParseObject& other)
    {
        m_rState = other.m_rState;
    }


private:
    struct ReusableState
    {
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecated = ParseProtocol::notYetDeprecated();
        bool m_deprecatedRemoved = false;
    };

    ParseObject* m_parent = nullptr;
    ReusableState m_rState;
};

} // namespace parse

} // namespace commsdsl
