#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Namespace.h"
#include "Message.h"
#include "Interface.h"
#include "common.h"

//#include "Field.h"

namespace commsdsl2comms
{

class Generator;
class Namespace
{
public:
    using Ptr = std::unique_ptr<Namespace>;
    using NamespacesList = std::vector<Ptr>;
    using InterfacesList = std::vector<InterfacePtr>;
    using MessagesAccessList = std::vector<const Message*>;

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Namespace(Generator& gen, const commsdsl::Namespace& dslObj)
      : m_generator(gen),
        m_dslObj(dslObj)
    {
    }

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    bool prepare();

    bool writeInterfaces();
    bool writeMessages();

    std::string getDefaultOptions() const;

    MessagesAccessList getAllMessages() const;

    bool hasInterfaceDefined();

private:

    using MessagesList = std::vector<MessagePtr>;

    bool prepareNamespaces();
    bool prepareInterfaces();
    bool prepareMessages();

    Generator& m_generator;
    commsdsl::Namespace m_dslObj;
    NamespacesList m_namespaces;
    InterfacesList m_interfaces;
    MessagesList m_messages;
};

using NamespacePtr = Namespace::Ptr;

inline
NamespacePtr createNamespace(Generator& gen, const commsdsl::Namespace& dslObj)
{
    return NamespacePtr(new Namespace(gen, dslObj));
}

} // namespace commsdsl2comms
