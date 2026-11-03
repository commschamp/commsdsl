//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

#include "CommsLayer.h"

#include "commsdsl/gen/GenFrame.h"

#include <vector>
#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsFrame final: public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using ParseFrame = commsdsl::parse::ParseFrame;

    using GenElem = commsdsl::gen::GenElem;

    using CommsLayersList = CommsLayer::CommsLayersList;

    explicit CommsFrame(CommsGenerator& generator, ParseFrame parseObj, GenElem* parent);
    virtual ~CommsFrame();

    [[nodiscard]] std::string commsDefaultOptions() const;
    [[nodiscard]] std::string commsDataViewDefaultOptions() const;
    [[nodiscard]] std::string commsBareMetalDefaultOptions() const;
    [[nodiscard]] std::string commsMsgFactoryDefaultOptions() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using CommsLayerOptsFunc = std::string (CommsLayer::*)() const;

    [[nodiscard]] bool commsWriteCommonInternal() const;
    [[nodiscard]] bool commsWriteDefInternal() const;
    [[nodiscard]] std::string commsCommonIncludesInternal() const;
    [[nodiscard]] std::string commsCommonBodyInternal() const;
    [[nodiscard]] std::string commsDefIncludesInternal() const;
    [[nodiscard]] std::string commsDefLayersDefInternal() const;
    [[nodiscard]] std::string commsDefFrameBaseInternal() const;
    [[nodiscard]] std::string commsDefInputMessagesDocInternal() const;
    [[nodiscard]] std::string commsDefInputMessagesParamInternal() const;
    [[nodiscard]] std::string commsDefAccessDocInternal() const;
    [[nodiscard]] std::string commsDefAccessListInternal() const;
    [[nodiscard]] std::string commsDefProtectedInternal() const;
    [[nodiscard]] std::string commsDefPrivateInternal() const;
    [[nodiscard]] std::string commsCustomizationOptionsInternal(
        CommsLayerOptsFunc layerOptsFunc,
        bool hasBase) const;

    CommsLayersList m_commsLayers;
    bool m_hasIdLayer = false;
    bool m_hasCommonCode = false;
};

} // namespace commsdsl2comms
