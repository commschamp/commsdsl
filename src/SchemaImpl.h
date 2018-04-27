#pragma once

#include "bbmp/Endian.h"

#include "XmlWrap.h"
#include "Logger.h"

namespace bbmp
{

class SchemaImpl
{
public:
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;

    SchemaImpl(::xmlNodePtr node, Logger& logger);

    bool processNode();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const
    {
        return m_name;
    }

    const std::string& description() const
    {
        return m_description;
    }

    unsigned id() const
    {
        return m_id;
    }

    unsigned version() const
    {
        return m_version;
    }

    Endian endian() const
    {
        return m_endian;
    }

    const PropsMap& unknownAttributes() const
    {
        return m_unknownAttrs;
    }

    PropsMap& unknownAttributes()
    {
        return m_unknownAttrs;
    }

    const ContentsList& unknownChiltren() const
    {
        return m_unknownChildren;
    }

    ContentsList& unknownChiltren()
    {
        return m_unknownChildren;
    }

private:

    bool updateStringProperty(const PropsMap& map, const std::string& name, std::string& prop);
    bool updateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop);
    bool updateEndianProperty(const PropsMap& map, const std::string& name, Endian& prop);

    ::xmlNodePtr m_node = nullptr;
    Logger& m_logger;

    PropsMap m_props;
    PropsMap m_unknownAttrs;
    ContentsList m_unknownChildren;
    std::string m_name;
    std::string m_description;
    unsigned m_id = 0U;
    unsigned m_version = 0;
    Endian m_endian = Endian_NumOfValues;


};

} // namespace bbmp
