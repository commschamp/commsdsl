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

#include "ToolsQtLayer.h"

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtFrame final : public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using ParseFrame = commsdsl::parse::ParseFrame;

    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    using ToolsQtLayersList = std::vector<ToolsQtLayer*>;

    explicit ToolsQtFrame(ToolsQtGenerator& generator, ParseFrame parseObj, GenElem* parent);

    std::string toolsHeaderFilePath(const commsdsl::gen::GenInterface& iFace) const;
    GenStringsList toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsClassScope(const commsdsl::gen::GenInterface& iFace) const;

    static const ToolsQtFrame* toolsCast(const commsdsl::gen::GenFrame* val)
    {
        return static_cast<const ToolsQtFrame*>(val);
    }

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

private:
    bool toolsWriteProtTransportMsgHeaderInternal() const;
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;
    bool toolsWriteTransportMsgHeaderInternal() const;
    bool toolsWriteTransportMsgSrcInternal() const;
    unsigned toolsCalcBackPayloadOffsetInternal() const;
    std::string toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsProtTransportMsgDefInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsProtTransportMsgHeaderExtraIncInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsProtTransportMsgReadFuncInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsTransportMsgHeaderDefInternal() const;
    std::string toolsTransportMsgSrcDefInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsFrameHeaderDefInternal() const;
    std::string toolsFrameSrcDefInternal(const commsdsl::gen::GenInterface& iFace) const;

    ToolsQtLayersList m_toolsLayers;
};

} // namespace commsdsl2tools_qt
