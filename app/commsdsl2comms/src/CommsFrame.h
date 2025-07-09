//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsLayer.h"

#include "commsdsl/gen/GenFrame.h"

#include <vector>
#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsFrame final: public commsdsl::gen::GenFrame
{
    using Base = commsdsl::gen::GenFrame;
public:
    using CommsLayersList = CommsLayer::CommsLayersList;

    explicit CommsFrame(CommsGenerator& generator, commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent);
    virtual ~CommsFrame();

    std::string commsDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;
    std::string commsMsgFactoryDefaultOptions() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using LayerOptsFunc = std::string (CommsLayer::*)() const;

    bool commsWriteCommonInternal() const;  
    bool commsWriteDefInternal() const;  
    std::string commsCommonIncludesInternal() const;
    std::string commsCommonBodyInternal() const;
    std::string commsDefIncludesInternal() const;
    std::string commsDefLayersDefInternal() const;
    std::string commsDefFrameBaseInternal() const;
    std::string commsDefInputMessagesDocInternal() const;
    std::string commsDefInputMessagesParamInternal() const;
    std::string commsDefAccessDocInternal() const;
    std::string commsDefAccessListInternal() const;
    std::string commsDefProtectedInternal() const;
    std::string commsDefPrivateInternal() const;
    std::string commsCustomizationOptionsInternal(
        LayerOptsFunc layerOptsFunc,
        bool hasBase) const;    
    
    CommsLayersList m_commsLayers;  
    bool m_hasIdLayer = false;
    bool m_hasCommonCode = false;
};

} // namespace commsdsl2comms
