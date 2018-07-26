#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Namespace.h"
#include "Message.h"
#include "Interface.h"
#include "Field.h"
#include "Frame.h"
#include "common.h"


namespace commsdsl2comms
{

class Generator;
class Namespace
{
public:
    using Ptr = std::unique_ptr<Namespace>;
    using NamespacesList = std::vector<Ptr>;
    using InterfacesList = std::vector<InterfacePtr>;
    using FieldsList = std::vector<FieldPtr>;
    using MessagesAccessList = std::vector<const Message*>;
    using InterfacesAccessList = std::vector<const Interface*>;

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Namespace(Generator& gen, const commsdsl::Namespace& dslObj)
      : m_generator(gen),
        m_dslObj(dslObj)
    {
    }

    const std::string& name() const;

    bool prepare();

    bool writeInterfaces();
    bool writeMessages();
    bool writeFrames();
    bool writeFields();

    std::string getDefaultOptions() const;

    MessagesAccessList getAllMessages() const;
    InterfacesAccessList getAllInterfaces() const;

    bool hasInterfaceDefined();

    const Field* findMessageIdField() const;
    const Field* findField(const std::string& externalRef, bool record);

    bool anyInterfaceHasVersion() const;

    common::StringsList pluginCommonSources() const;

    std::string externalRef() const;
    bool addDefaultInterface();

    bool hasFrame() const;

private:

    using MessagesList = std::vector<MessagePtr>;
    using FramesList = std::vector<FramePtr>;
    using AccessedFields = std::map<const Field*, bool>;

    bool prepareNamespaces();
    bool prepareFields();
    bool prepareInterfaces();
    bool prepareMessages();
    bool prepareFrames();
    void recordAccessedField(const Field* field);

    Generator& m_generator;
    commsdsl::Namespace m_dslObj;
    NamespacesList m_namespaces;
    FieldsList m_fields;
    InterfacesList m_interfaces;
    MessagesList m_messages;
    FramesList m_frames;
    AccessedFields m_accessedFields;
};

using NamespacePtr = Namespace::Ptr;

inline
NamespacePtr createNamespace(Generator& gen, const commsdsl::Namespace& dslObj)
{
    return NamespacePtr(new Namespace(gen, dslObj));
}

} // namespace commsdsl2comms
