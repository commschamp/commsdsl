#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Message.h"

#include "Field.h"

namespace commsdsl2comms
{

class Generator;
class Message
{
public:
    using Sender = commsdsl::Message::Sender;
    
    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Message(Generator& gen, const commsdsl::Message& msg)
      : m_generator(gen),
        m_dslObj(msg)
    {
    }

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    bool prepare();

    bool doesExist() const;

    bool write();

    std::string getDefaultOptions() const;
    std::string getClientOptions() const;
    std::string getServerOptions() const;

    std::uintmax_t id() const
    {
        return m_dslObj.id();
    }

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

private:

    bool writeProtocol();
    bool writePluginHeader();
    bool writePluginSrc();
    const std::string& getDisplayName() const;
    std::string getDescription() const;
    std::string getFieldsClassesList() const;
    std::string getIncludes() const;
    std::string getBody() const;
    std::string getPublic() const;
    std::string getProtected() const;
    std::string getPrivate() const;
    std::string getFieldsAccess() const;
    std::string getLengthCheck() const;
    std::string getFieldsDef() const;
    std::string getNamespaceScope() const;
    std::string getNameFunc() const;
    std::string getReadFunc() const;
    std::string getRefreshFunc() const;
    std::string getExtraOptions() const;
    std::string getExtraPublic() const;

    bool mustImplementReadRefresh() const;
    bool isCustomizable() const;

    Generator& m_generator;
    commsdsl::Message m_dslObj;
    std::string m_externalRef;
    std::vector<FieldPtr> m_fields;
    std::string m_customRefresh;
};

using MessagePtr = std::unique_ptr<Message>;

inline
MessagePtr createMessage(Generator& gen, const commsdsl::Message& msg)
{
    return MessagePtr(new Message(gen, msg));
}

} // namespace commsdsl2comms
