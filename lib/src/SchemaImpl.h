#pragma once

#include "commsdsl/Endian.h"

#include "XmlWrap.h"

namespace commsdsl
{

class ProtocolImpl;
class SchemaImpl
{
public:
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;

    SchemaImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    bool processNode();

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

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

    bool nonUniqueMsgIdAllowed() const
    {
        return m_nonUniqueMsgIdAllowed;
    }

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& extraAttributes()
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildrenElements() const
    {
        return m_extraChildren;
    }

    ContentsList& extraChildrenElements()
    {
        return m_extraChildren;
    }

private:

    bool updateStringProperty(const PropsMap& map, const std::string& name, std::string& prop);
    bool updateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop);
    bool updateEndianProperty(const PropsMap& map, const std::string& name, Endian& prop);
    bool updateBooleanProperty(const PropsMap& map, const std::string& name, bool& prop);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
    std::string m_name;
    std::string m_description;
    unsigned m_id = 0U;
    unsigned m_version = 0;
    Endian m_endian = Endian_NumOfValues;
    bool m_nonUniqueMsgIdAllowed = false;
};

} // namespace commsdsl
