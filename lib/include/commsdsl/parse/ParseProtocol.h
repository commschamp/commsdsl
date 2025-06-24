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

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <limits>

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseErrorLevel.h"
#include "commsdsl/parse/ParseSchema.h"
#include "commsdsl/parse/ParseField.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class COMMSDSL_API ParseProtocol
{
public:
    using ErrorReportFunction = std::function<void (ParseErrorLevel, const std::string&)>;
    using SchemasList = std::vector<ParseSchema>;

    ParseProtocol();
    ~ParseProtocol();

    void parseSetErrorReportCallback(ErrorReportFunction&& cb);

    bool parse(const std::string& input);
    bool parseValidate();

    SchemasList parseSchemas() const;

    ParseSchema parseLastParsedSchema() const;

    static constexpr unsigned parseNotYetDeprecated() noexcept
    {
        return std::numeric_limits<unsigned>::max();
    }

    ParseField parseFindField(const std::string& externalRef) const;

    void parseAddExpectedExtraPrefix(const std::string& value);

    void parseSetMultipleSchemasEnabled(bool value);
    bool parseGetMultipleSchemasEnabled() const;

private:
    std::unique_ptr<ParseProtocolImpl> m_pImpl;
};

} // namespace parse

} // namespace commsdsl
