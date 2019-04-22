//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "Interface.h"
#include "Frame.h"

namespace commsdsl2comms
{

class Generator;
class Plugin
{
public:
    using Ptr = std::unique_ptr<Plugin>;

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Plugin(
        Generator& gen,
        const std::string& frame,
        const std::string& interface,
        const std::string& name,
        const std::string& description)
      : m_generator(gen),
        m_frame(frame),
        m_interface(interface),
        m_name(name),
        m_description(description)
    {
    }

    bool prepare();
    bool write();

    const std::string& name() const
    {
        return m_name;
    }

    const std::string& adjustedName() const;

private:

    bool writeProtocolHeader();
    bool writeProtocolSrc();
    bool writePluginHeader();
    bool writePluginSrc();
    bool writePluginJson();
    bool writePluginConfig();
    std::string protClassName() const;
    std::string pluginClassName() const;
    std::string pluginId() const;

    Generator& m_generator;
    std::string m_frame;
    std::string m_interface;
    std::string m_name;
    std::string m_description;

    const Frame* m_framePtr = nullptr;
    const Interface* m_interfacePtr = nullptr;
};

using PluginPtr = Plugin::Ptr;

inline
PluginPtr createPlugin(
    Generator& gen,
    const std::string& frame,
    const std::string& interface,
    const std::string& name,
    const std::string& description)
{
    return PluginPtr(new Plugin(gen, frame, interface, name, description));
}

} // namespace commsdsl2comms
