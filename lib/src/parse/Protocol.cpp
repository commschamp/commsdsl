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

#include "commsdsl/parse/Protocol.h"

#include "ProtocolImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

Protocol::Protocol()
  : m_pImpl(new ProtocolImpl)
{
}

void Protocol::setErrorReportCallback(Protocol::ErrorReportFunction&& cb)
{
    m_pImpl->setErrorReportCallback(std::move(cb));
}

Protocol::~Protocol() = default;

bool Protocol::parse(const std::string& input)
{
    return m_pImpl->parse(input);
}

bool Protocol::validate()
{
    return m_pImpl->validate();
}

Protocol::SchemasList Protocol::schemas() const
{
    return m_pImpl->schemas();
}

Schema Protocol::lastParsedSchema() const
{
    return Schema(&m_pImpl->currSchema());
}

Field Protocol::findField(const std::string& externalRef) const
{
    return Field(m_pImpl->findField(externalRef));
}

void Protocol::addExpectedExtraPrefix(const std::string& value)
{
    return m_pImpl->addExpectedExtraPrefix(value);
}

} // namespace parse

} // namespace commsdsl
