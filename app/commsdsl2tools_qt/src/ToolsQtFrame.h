//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtLayer.h"

#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtFrame final : public commsdsl::gen::Frame
{
    using Base = commsdsl::gen::Frame;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using ToolsQtLayersList = std::vector<ToolsQtLayer*>;

    explicit ToolsQtFrame(ToolsQtGenerator& generator, commsdsl::parse::Frame dslObj, commsdsl::gen::Elem* parent);

    std::string toolsHeaderFilePath() const;
    StringsList toolsSourceFiles() const;

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteTransportMsgHeaderInternal() const;
    bool toolsWriteTransportMsgSrcInternal() const;
    std::string toolsTransportMessageHeaderFilePathInternal() const;
    std::string toolsTransportMessageSrcFilePathInternal() const;
    unsigned toolsCalcBackPayloadOffsetInternal() const;
    std::string toolsRelFilePath() const;

    ToolsQtLayersList m_toolsLayers;
};

} // namespace commsdsl2tools_qt
