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

    bool write();

private:

    bool writeProtocol();
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

    Generator& m_generator;
    commsdsl::Message m_dslObj;
    std::string m_externalRef;
    std::vector<FieldPtr> m_fields;
};

using MessagePtr = std::unique_ptr<Message>;

inline
MessagePtr createMessage(Generator& gen, const commsdsl::Message& msg)
{
    return MessagePtr(new Message(gen, msg));
}

} // namespace commsdsl2comms
