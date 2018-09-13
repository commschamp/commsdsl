#pragma once

#include <memory>
#include <map>
#include <string>
#include <cstdint>

#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"
#include "commsdsl/Message.h"
#include "commsdsl/Protocol.h"
#include "FieldImpl.h"

namespace commsdsl
{

class ProtocolImpl;
class MessageImpl : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<MessageImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using FieldsList = Message::FieldsList;
    using ContentsList = XmlWrap::ContentsList;
    using PlatformsList = Protocol::PlatformsList;
    using Sender = Message::Sender;

    MessageImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    MessageImpl(const MessageImpl&) = delete;
    MessageImpl(MessageImpl&&) = default;
    ~MessageImpl() = default;

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

    std::uintmax_t id() const
    {
        return m_id;
    }

    unsigned order() const
    {
        return m_order;
    }

    std::size_t minLength() const;

    std::size_t maxLength() const;

    FieldsList fieldsList() const;

    std::string externalRef() const;

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

    const PlatformsList& platforms() const
    {
        return m_platforms;
    }

    bool isCustomizable() const
    {
        return m_customizable;
    }

    Sender sender() const
    {
        return m_sender;
    }

protected:

    virtual ObjKind objKindImpl() const override final;

private:
    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const XmlWrap::NamesList& commonProps();
    static XmlWrap::NamesList allNames();

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateId();
    bool updateOrder();
    bool updateVersions();
    bool updatePlatforms();
    bool updateCustomizable();
    bool updateSender();
    bool copyFields();
    bool updateFields();
    void cloneFieldsFrom(const MessageImpl& other);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;

    const std::string* m_name = nullptr;
    const std::string* m_displayName = nullptr;
    const std::string* m_description = nullptr;
    std::uintmax_t m_id = 0;
    unsigned m_order = 0;
    std::vector<FieldImplPtr> m_fields;
    PlatformsList m_platforms;
    Sender m_sender = Sender::Both;
    bool m_customizable = false;
};

using MessageImplPtr = MessageImpl::Ptr;

} // namespace commsdsl
