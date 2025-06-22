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

#include "commsdsl/parse/ParseProtocol.h"

#include "ParseProtocolImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

ParseProtocol::ParseProtocol()
  : m_pImpl(new ParseProtocolImpl)
{
}

void ParseProtocol::setErrorReportCallback(ParseProtocol::ErrorReportFunction&& cb)
{
    m_pImpl->setErrorReportCallback(std::move(cb));
}

ParseProtocol::~ParseProtocol() = default;

bool ParseProtocol::parse(const std::string& input)
{
    return m_pImpl->parse(input);
}

bool ParseProtocol::validate()
{
    return m_pImpl->validate();
}

ParseProtocol::SchemasList ParseProtocol::schemas() const
{
    return m_pImpl->schemas();
}

ParseSchema ParseProtocol::lastParsedSchema() const
{
    return ParseSchema(&m_pImpl->currSchema());
}

ParseField ParseProtocol::findField(const std::string& externalRef) const
{
    return ParseField(m_pImpl->findField(externalRef));
}

void ParseProtocol::addExpectedExtraPrefix(const std::string& value)
{
    return m_pImpl->addExpectedExtraPrefix(value);
}

void ParseProtocol::setMultipleSchemasEnabled(bool value)
{
    m_pImpl->setMultipleSchemasEnabled(value);
}

bool ParseProtocol::getMultipleSchemasEnabled() const
{
    return m_pImpl->getMultipleSchemasEnabled();
}

} // namespace parse

} // namespace commsdsl
