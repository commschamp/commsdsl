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

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <limits>

#include "commsdsl/CommsdslApi.h"
#include "ErrorLevel.h"
#include "Schema.h"
#include "Field.h"

namespace commsdsl
{

namespace parse
{

class ProtocolImpl;
class COMMSDSL_API Protocol
{
public:
    using ErrorReportFunction = std::function<void (ErrorLevel, const std::string&)>;
    using SchemasList = std::vector<Schema>;

    Protocol();
    ~Protocol();

    void setErrorReportCallback(ErrorReportFunction&& cb);

    bool parse(const std::string& input);
    bool validate();

    SchemasList schemas() const;

    Schema lastParsedSchema() const;

    static constexpr unsigned notYetDeprecated() noexcept
    {
        return std::numeric_limits<unsigned>::max();
    }

    Field findField(const std::string& externalRef) const;

    void addExpectedExtraPrefix(const std::string& value);

    void setMultipleSchemasEnabled(bool value);
    bool getMultipleSchemasEnabled() const;

private:
    std::unique_ptr<ProtocolImpl> m_pImpl;
};

} // namespace parse

} // namespace commsdsl
