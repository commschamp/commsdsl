//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "commsdsl/Frame.h"

#include "Layer.h"

namespace commsdsl2comms
{

class Generator;
class Frame
{
public:

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Frame(Generator& gen, const commsdsl::Frame& obj)
      : m_generator(gen),
        m_dslObj(obj)
    {
    }

    const std::string name() const
    {
        return m_dslObj.name();
    }

    bool prepare();

    bool write();

    std::string getDefaultOptions(const std::string& base) const;
    std::string getBareMetalDefaultOptions(const std::string& base) const;
    std::string getDataViewDefaultOptions(const std::string& base) const;

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

    std::vector<std::string> getPseudoVersionLayers(const std::vector<std::string>& interfaceVersionFields) const;

private:
    using GetLayerOptionsFunc = std::string (Layer::*)(const std::string& base, const std::string& scope) const;

    bool writeProtocolDefinitionCommonFile();
    bool writeProtocol();
    bool writePluginTransportMessageHeader();
    bool writePluginTransportMessageSrc();
    bool writePluginHeader();

    std::string getDescription() const;
    std::string getIncludes() const;
    std::string getLayersDef() const;
    std::string getFrameDef() const;
    std::string getLayersAccess() const;
    std::string getLayersAccessDoc() const;
    std::string getInputMessages() const;
    std::string getInputMessagesDoc() const;

    bool hasIdLayer() const;
    unsigned calcBackPayloadOffset() const;
    std::string getOptions(GetLayerOptionsFunc func, const std::string& base) const;
    bool hasCommonDefinition() const;

    Generator& m_generator;
    commsdsl::Frame m_dslObj;
    std::string m_externalRef;
    std::vector<LayerPtr> m_layers;
};

using FramePtr = std::unique_ptr<Frame>;

inline
FramePtr createFrame(Generator& gen, const commsdsl::Frame& msg)
{
    return FramePtr(new Frame(gen, msg));
}

} // namespace commsdsl2comms
