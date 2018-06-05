#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Message.h"
#include "commsdsl/Field.h"

//#include "Field.h"

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

    Generator& m_generator;
    commsdsl::Message m_dslObj;
    std::string m_externalRef;
};

using MessagePtr = std::unique_ptr<Message>;

inline
MessagePtr createMessage(Generator& gen, const commsdsl::Message& msg)
{
    return MessagePtr(new Message(gen, msg));
}

} // namespace commsdsl2comms
