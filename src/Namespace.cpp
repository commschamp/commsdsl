#include "Namespace.h"

#include <cassert>
#include <algorithm>

namespace commsdsl2comms
{

bool Namespace::prepare()
{
    return
        prepareNamespaces() &&
        prepareFields() &&
        prepareInterfaces() &&
        prepareMessages();
}

bool Namespace::writeInterfaces()
{
    return
        std::all_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& ptr)
            {
                return ptr->writeInterfaces();
            }) &&
        std::all_of(
            m_interfaces.begin(), m_interfaces.end(),
            [](auto& ptr)
            {
                return ptr->write();
            });
}

bool Namespace::writeMessages()
{
    return
        std::all_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& ptr)
            {
                return ptr->writeMessages();
            }) &&
        std::all_of(
            m_messages.begin(), m_messages.end(),
            [](auto& ptr)
            {
                return ptr->write();
    });
}

bool Namespace::writeFields()
{
    for (auto& n : m_namespaces) {
        if (!n->writeFields()) {
            return false;
        }
    }

    for (auto* f : m_accessedFields) {
        if (!f->writeProtocolDefinition()) {
            return false;
        }
    }

    return true;
}

std::string Namespace::getDefaultOptions() const
{

    auto addFunc =
        [](const std::string& str, std::string& result)
        {
            if (str.empty()) {
                return;
            }

            if (!result.empty()) {
                result += '\n';
            }

            result += str;
        };

    std::string namespacesOpts;
    for (auto& n : m_namespaces) {
        addFunc(n->getDefaultOptions(), namespacesOpts);
    }

    std::string fieldsOpts;
    for (auto& f : m_fields) {
        addFunc(f->getDefaultOptions(), fieldsOpts);
    }

    std::string messagesOpts;
    for (auto& m : m_messages) {
        addFunc(m->getDefaultOptions(), messagesOpts);
    }

    if (!fieldsOpts.empty()) {
        static const std::string FieldsWrapTempl =
            "struct field\n"
            "{\n"
            "    #^#FIELDS_OPTS#$#\n"
            "}; // struct field\n";

        common::ReplacementMap replacmenents;
        replacmenents.insert(std::make_pair("FIELDS_OPTS", fieldsOpts));
        fieldsOpts = common::processTemplate(FieldsWrapTempl, replacmenents);
    }

    if (!messagesOpts.empty()) {
        static const std::string MessageWrapTempl =
            "struct message\n"
            "{\n"
            "    #^#MESSAGES_OPTS#$#\n"
            "}; // struct message\n";

        common::ReplacementMap replacmenents;
        replacmenents.insert(std::make_pair("MESSAGES_OPTS", messagesOpts));
        messagesOpts = common::processTemplate(MessageWrapTempl, replacmenents);
    }

    common::ReplacementMap replacmenents;
    replacmenents.insert(std::make_pair("NAMESPACE_NAME", name()));
    replacmenents.insert(std::make_pair("NAMESPACES_OPTS", std::move(namespacesOpts)));
    replacmenents.insert(std::make_pair("MESSAGES_OPTS", std::move(messagesOpts)));
    replacmenents.insert(std::make_pair("FIELDS_OPTS", std::move(fieldsOpts)));

    static const std::string Templ =
        "struct #^#NAMESPACE_NAME#$#\n"
        "{\n"
        "    #^#NAMESPACES_OPTS#$#\n"
        "    #^#FIELDS_OPTS#$#\n"
        "    #^#MESSAGES_OPTS#$#\n"
        "};\n";

    static const std::string GlobalTempl =
        "#^#FIELDS_OPTS#$#\n"
        "#^#MESSAGES_OPTS#$#\n";

    auto* templ = &Templ;
    if (name().empty()) {
        templ = &GlobalTempl;
    }

    return common::processTemplate(*templ, replacmenents);
}

Namespace::MessagesAccessList Namespace::getAllMessages() const
{
    MessagesAccessList result;
    for (auto& n : m_namespaces) {
        auto list = n->getAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_messages.size());
    for (auto& m : m_messages) {
        result.emplace_back(m.get());
    }

    return result;
}

bool Namespace::hasInterfaceDefined()
{
    bool defined =
        std::any_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& n)
            {
                return n->hasInterfaceDefined();
            });

    if (defined) {
        return true;
    }

    return !m_interfaces.empty();
}

const Field* Namespace::findField(const std::string& externalRef)
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    if (nsName.empty()) {
        auto fieldIter =
            std::lower_bound(
                m_fields.begin(), m_fields.end(), externalRef,
                [&externalRef](auto& f, auto& n)
                {
                    return f->name() < n;
                });

        if ((fieldIter == m_fields.end()) || ((*fieldIter)->name() != externalRef)) {
            return nullptr;
        }

        recordAccessedField(fieldIter->get());
        return fieldIter->get();
    }

    auto nsIter =
        std::lower_bound(
            m_namespaces.begin(), m_namespaces.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->name() < n;
            });

    if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
        return nullptr;
    }

    auto fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->findField(remStr);
}

bool Namespace::prepareNamespaces()
{
    auto namespaces = m_dslObj.namespaces();
    m_namespaces.reserve(namespaces.size());
    for (auto& n : namespaces) {
        auto ptr = createNamespace(m_generator, n);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }

        m_namespaces.push_back(std::move(ptr));
    }

    return true;
}

bool Namespace::prepareFields()
{
    auto fields = m_dslObj.fields();
    m_fields.reserve(fields.size());
    for (auto& dslObj : fields) {
        auto ptr = Field::create(m_generator, dslObj);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }

        m_fields.push_back(std::move(ptr));
    }

    return true;
}

bool Namespace::prepareInterfaces()
{
    auto interfaces = m_dslObj.interfaces();
    m_interfaces.reserve(interfaces.size());
    for (auto& dslObj : interfaces) {
        auto ptr = createInterface(m_generator, dslObj);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }

        m_interfaces.push_back(std::move(ptr));
    }

    return true;
}

bool Namespace::prepareMessages()
{
    auto messages = m_dslObj.messages();
    m_messages.reserve(messages.size());
    for (auto& dslObj : messages) {
        auto ptr = createMessage(m_generator, dslObj);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }

        m_messages.push_back(std::move(ptr));
    }

    return true;
}

void Namespace::recordAccessedField(const Field* field)
{
    auto iter = m_accessedFields.find(field);
    if (iter != m_accessedFields.end()) {
        return;
    }

    m_accessedFields.insert(field);
}

}
