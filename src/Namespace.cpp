#include "Namespace.h"

#include <cassert>
#include <algorithm>

namespace commsdsl2comms
{

bool Namespace::prepare()
{
    return
        prepareNamespaces() &&
        prepareMessages();
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

    // TODO: fields

    std::string messagesOpts;
    for (auto& m : m_messages) {
        addFunc(m->getDefaultOptions(), messagesOpts);
    }

    if (!messagesOpts.empty()) {
        static const std::string MessageWrapTempl =
            "struct message\n"
            "{\n"
            "    #^#MESSAGES_OPTS#$#\n"
            "};";

        common::ReplacementMap replacmenents;
        replacmenents.insert(std::make_pair("MESSAGES_OPTS", messagesOpts));
        messagesOpts = common::processTemplate(MessageWrapTempl, replacmenents);
    }

    common::ReplacementMap replacmenents;
    replacmenents.insert(std::make_pair("NAMESPACE_NAME", name()));
    replacmenents.insert(std::make_pair("NAMESPACES_OPTS", std::move(namespacesOpts)));
    replacmenents.insert(std::make_pair("MESSAGES_OPTS", std::move(messagesOpts)));

    static const std::string Templ =
        "struct #^#NAMESPACE_NAME#$#Fields\n"
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

}
