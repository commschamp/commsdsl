#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Namespace.h"
#include "Message.h"
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
    using MessagesList = std::vector<MessagePtr>;

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

    bool writeMessages();

    std::string getDefaultOptions() const;

private:

    bool prepareNamespaces();
    bool prepareMessages();

    Generator& m_generator;
    commsdsl::Namespace m_dslObj;
    NamespacesList m_namespaces;
    MessagesList m_messages;
};

using NamespacePtr = Namespace::Ptr;

inline
NamespacePtr createNamespace(Generator& gen, const commsdsl::Namespace& dslObj)
{
    return NamespacePtr(new Namespace(gen, dslObj));
}

} // namespace commsdsl2comms
