//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtInterface.h"

#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtMessage final : public commsdsl::gen::Message
{
    using Base = commsdsl::gen::Message;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    explicit ToolsQtMessage(ToolsQtGenerator& generator, commsdsl::parse::ParseMessage dslObj, commsdsl::gen::Elem* parent);

    std::string toolsHeaderPath(const commsdsl::gen::Interface& iFace) const;
    StringsList toolsSourceFiles(const commsdsl::gen::Interface& iFace) const;
    std::string toolsClassScope(const commsdsl::gen::Interface& iFace) const;

    static ToolsQtMessage& cast(commsdsl::gen::Message& msg)
    {
        return static_cast<ToolsQtMessage&>(msg);
    }

    static const ToolsQtMessage& cast(const commsdsl::gen::Message& msg)
    {
        return static_cast<const ToolsQtMessage&>(msg);
    }    

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;
    std::string toolsRelPathInternal(const commsdsl::gen::Interface& iFace) const;
    IncludesList toolsHeaderIncludesInternal() const;
    std::string toolsHeaderCodeInternal() const;
    IncludesList toolsSrcIncludesInternal(const commsdsl::gen::Interface& iFace) const;
    std::string toolsSrcCodeInternal(const commsdsl::gen::Interface& iFace) const;

    bool m_exists = true;
};

} // namespace commsdsl2tools_qt
