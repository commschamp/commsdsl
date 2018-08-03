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
    std::string protClassName() const;
    std::string pluginClassName() const;

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
