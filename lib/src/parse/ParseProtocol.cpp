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

void ParseProtocol::parseSetErrorReportCallback(ParseProtocol::ErrorReportFunction&& cb)
{
    m_pImpl->parseSetErrorReportCallback(std::move(cb));
}

ParseProtocol::~ParseProtocol() = default;

bool ParseProtocol::parse(const std::string& input)
{
    return m_pImpl->parse(input);
}

bool ParseProtocol::parseValidate()
{
    return m_pImpl->parseValidate();
}

ParseProtocol::SchemasList ParseProtocol::parseSchemas() const
{
    return m_pImpl->parseSchemas();
}

ParseSchema ParseProtocol::parseLastParsedSchema() const
{
    return ParseSchema(&m_pImpl->parseCurrSchema());
}

ParseField ParseProtocol::parseFindField(const std::string& externalRef) const
{
    return ParseField(m_pImpl->parseFindField(externalRef));
}

void ParseProtocol::parseAddExpectedExtraPrefix(const std::string& value)
{
    return m_pImpl->parseAddExpectedExtraPrefix(value);
}

void ParseProtocol::parseSetMultipleSchemasEnabled(bool value)
{
    m_pImpl->parseSetMultipleSchemasEnabled(value);
}

bool ParseProtocol::parseGetMultipleSchemasEnabled() const
{
    return m_pImpl->parseGetMultipleSchemasEnabled();
}

} // namespace parse

} // namespace commsdsl
