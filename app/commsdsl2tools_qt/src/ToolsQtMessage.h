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

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtMessage final : public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using ToolsIncludesList = GenStringsList;

    explicit ToolsQtMessage(ToolsQtGenerator& generator, ParseMessage parseObj, GenElem* parent);

    std::string toolsHeaderPath(const commsdsl::gen::GenInterface& iFace) const;
    GenStringsList toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsClassScope(const commsdsl::gen::GenInterface& iFace) const;

    static ToolsQtMessage& toolsCast(commsdsl::gen::GenMessage& msg)
    {
        return static_cast<ToolsQtMessage&>(msg);
    }

    static const ToolsQtMessage& toolsCast(const commsdsl::gen::GenMessage& msg)
    {
        return static_cast<const ToolsQtMessage&>(msg);
    }    

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;
    std::string toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const;
    ToolsIncludesList toolsHeaderIncludesInternal() const;
    std::string toolsHeaderCodeInternal() const;
    ToolsIncludesList toolsSrcIncludesInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsSrcCodeInternal(const commsdsl::gen::GenInterface& iFace) const;

    bool m_exists = true;
};

} // namespace commsdsl2tools_qt
