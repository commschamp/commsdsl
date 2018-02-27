#pragma once

#include <memory>

#include "XmlWrap.h"
#include "Logger.h"

namespace bbmp
{

class ProtocolImpl;
class FieldImpl
{
public:
    using Ptr = std::unique_ptr<FieldImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ProtocolImpl& protocol);

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

    std::size_t length() const
    {
        return lengthImpl();
    }

protected:
    FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    ProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;

    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const;
    virtual bool parseImpl();
    virtual std::size_t lengthImpl() const = 0;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);

private:

    bool updateName();
    bool updateDescription();
    bool updateDisplayName();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;

    const std::string* m_name = nullptr;
    const std::string* m_displayName = nullptr;
    const std::string* m_description = nullptr;
    PropsMap m_unknownAttrs;
    ContentsList m_unknownChildren;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace bbmp
