#include "ProtocolImpl.h"

#include <iostream>
#include <type_traits>
#include <cassert>
#include <algorithm>

#include "XmlWrap.h"
#include "FieldImpl.h"
#include "EnumFieldImpl.h"

namespace bbmp
{

ProtocolImpl::ProtocolImpl()
  : m_logger(
        [this](ErrorLevel level, const std::string& msg)
        {
            if (m_errorReportCb) {
                m_errorReportCb(level, msg);
                return;
            }

            static const std::string Prefix[] = {
                "[DEBUG]: ",
                "[INFO]: ",
                "[WARNING]: ",
                "[ERROR]: "
            };

            static const std::size_t PrefixCount = std::extent<decltype(Prefix)>::value;
            static_assert(PrefixCount == ErrorLevel_NumOfValues, "Invalid map");

            std::cerr << Prefix[level] << msg << std::endl;
        }
    )
{
    xmlSetStructuredErrorFunc(this, &ProtocolImpl::cbXmlErrorFunc);
    m_logger.setMinLevel(m_minLevel);
}

bool ProtocolImpl::parse(const std::string& input)
{
    if (m_validated) {
        logError() << "Parsing extra files after validation is not allowed";
        return false;
    }

    XmlDocPtr doc(xmlParseFile(input.c_str()));
    if (!doc) {
        std::cerr << "ERROR: Failed to parse" << input << std::endl;
        return false;
    }

    m_docs.push_back(std::move(doc));
    return true;
}

bool ProtocolImpl::validate()
{
    if (m_validated) {
        return true;
    }

    if (m_docs.empty()) {
        logError() << "Cannot validate without any schema files";
        return false;
    }

    for (auto& d : m_docs) {
        if (!validateDoc(d.get())) {
            return false;
        }
    }

    m_namespacesList.clear();
    m_namespacesList.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        if (!n.second->finalise()) {
            return false;
        }

        m_namespacesList.emplace_back(n.second.get());
    }

    m_validated = true;
    return true;
}

Schema ProtocolImpl::schema() const
{
    if ((!m_validated) && (!m_schema)) {
        logError() << "Invalid access to schema object.";
        return Schema(nullptr);
    }

    return Schema(m_schema.get());
}

SchemaImpl& ProtocolImpl::schemaImpl()
{
    assert(m_schema);
    return *m_schema;
}

const SchemaImpl& ProtocolImpl::schemaImpl() const
{
    assert(m_schema);
    return *m_schema;
}

FieldImpl* ProtocolImpl::findField(const std::string& name)
{
    static_cast<void>(name);
    logError() << __FUNCTION__ << ": NYI";
    // TODO: find among namespaces
    return nullptr;
//    auto iter = m_fields.find(name);
//    if (iter == m_fields.end()) {
//        return nullptr;
//    }

    //    return iter->second.get();
}

bool ProtocolImpl::strToEnumValue(
    const std::string& ref,
    std::intmax_t& val,
    bool checkRef) const
{
    if (checkRef) {
        if (!common::isValidRefName(ref)) {
            return false;
        }
    }
    else {
        assert(common::isValidRefName(ref));
    }

    auto dotCount = static_cast<std::size_t>(std::count(ref.begin(), ref.end(), '.'));
    if (dotCount < 1U) {
        return false;
    }

    auto nameSepPos = ref.find_last_of('.');
    assert(nameSepPos != std::string::npos);
    assert(0U < nameSepPos);

    auto enumNameSepPos = ref.find_last_of('.', nameSepPos - 1);
    assert((enumNameSepPos == std::string::npos) || ((enumNameSepPos + 1U) < nameSepPos));
    std::string elemName(ref.begin() + nameSepPos + 1, ref.end());
    std::string enumName;
    std::string remNamespaces;
    if (enumNameSepPos == std::string::npos) {
        enumName.assign(ref.begin(), ref.begin() + nameSepPos);
    }
    else {
        enumName.assign(ref.begin() + enumNameSepPos + 1, ref.begin() + nameSepPos);
        remNamespaces.assign(ref.begin(), ref.begin() + enumNameSepPos);
    }


    const NamespaceImpl* ns = nullptr;
    do {
        if (remNamespaces.empty()) {
            auto iter = m_namespaces.find(common::emptyString());
            if (iter == m_namespaces.end()) {
                assert(0);
                return false;
            }

            ns = iter->second.get();
            assert(ns != nullptr);
            break;
        }

        std::size_t nsNamePos = 0;
        assert(enumNameSepPos != std::string::npos);
        while (nsNamePos < enumNameSepPos) {
            auto nextDotPos = ref.find_first_of('.', nsNamePos);
            assert(nextDotPos != std::string::npos);
            std::string nsName(ref.begin() + nsNamePos, ref.begin() + nextDotPos);
            if (nsName.empty()) {
                return false;
            }

            auto* nsMap = &m_namespaces;
            if (ns != nullptr) {
                nsMap = &(ns->namespacesMap());
            }

            auto iter = nsMap->find(nsName);
            if (iter == nsMap->end()) {
                return false;
            }

            assert(iter->second);
            ns = iter->second.get();
            nsNamePos = nextDotPos + 1;
        }

    } while (false);

    assert(ns != nullptr);
    auto* field = ns->findField(enumName);
    if (field == nullptr) {
        return false;
    }

    if (field->kind() != Field::Kind::Enum) {
        return false;
    }

    auto* enumField = static_cast<const EnumFieldImpl*>(field);
    auto& enumValues = enumField->values();
    auto enumValueIter = enumValues.find(elemName);
    if (enumValueIter == enumValues.end()) {
        return false;
    }

    val = enumValueIter->second;
    return true;
}

void ProtocolImpl::cbXmlErrorFunc(void* userData, xmlErrorPtr err)
{
    reinterpret_cast<ProtocolImpl*>(userData)->handleXmlError(err);
}

void ProtocolImpl::handleXmlError(xmlErrorPtr err)
{
    static const ErrorLevel Map[] = {
        /* XML_ERR_NONE */ ErrorLevel_Debug,
        /* XML_ERR_WARNING */ ErrorLevel_Warning,
        /* XML_ERR_ERROR */ ErrorLevel_Error,
        /* XML_ERR_FATAL */ ErrorLevel_Error
    };

    static_assert(XML_ERR_NONE == 0, "Invalid assumption");
    static_assert(XML_ERR_FATAL == 3, "Invalid assumption");

    ErrorLevel level = ErrorLevel_Error;
    if ((XML_ERR_NONE <= err->level) && (err->level <= XML_ERR_FATAL)) {
        level = Map[err->level];
    }

    m_logger.setCurrLevel(level);
    do {
        if (err == nullptr) {
            break;
        }

        if (err->file != nullptr) {
            m_logger << std::string(err->file) << ':';
        }

        if (err->line != 0) {
            m_logger << err->line << ": ";
        }

        m_logger << err->message;
    } while (false);
    m_logger.flush();
}

bool ProtocolImpl::validateDoc(::xmlDocPtr doc)
{
    auto* root = ::xmlDocGetRootElement(doc);
    if (root == nullptr) {
        logError() << "Failed to fine root element in the schema file \"" << doc->URL << "\"";
        return false;
    }

    static const std::string SchemaName("schema");
    std::string rootName(reinterpret_cast<const char*>(root->name));
    if (rootName != SchemaName) {
        logError() << "Root element of \"" << doc->URL << "\" is not \"" << SchemaName << '\"';
        return false;
    }

    if (!validateSchema(root)) {
        return false;
    }

    auto namespaces = XmlWrap::getChildren(root, common::nsStr());
    for (auto& n : namespaces) {
        NamespaceImplPtr ns(new NamespaceImpl(n, *this));
        if (!ns->parseProps()) {
            return false;
        }

        auto& nsName = ns->name();
        auto iter = m_namespaces.find(nsName);
        NamespaceImpl* nsToProcess = nullptr;
        NamespaceImpl* realNs = nullptr;
        if (iter == m_namespaces.end()) {
            m_namespaces.insert(std::make_pair(nsName, std::move(ns)));
            iter = m_namespaces.find(nsName);
            assert(iter != m_namespaces.end());
            nsToProcess = iter->second.get();
        }
        else {
            nsToProcess = ns.get();
            realNs = iter->second.get();
        }

        assert(iter->second);
        if (!nsToProcess->parseChildren(realNs)) {
            return false;
        }
    }

    auto& nsChildren = NamespaceImpl::supportedChildren();
    auto globalNsChildren = XmlWrap::getChildren(root, nsChildren);
    do {
        if (globalNsChildren.empty()) {
            break;
        }

        auto& globalNsPtr = m_namespaces[common::emptyString()]; // create if needed
        if (!globalNsPtr) {
            globalNsPtr.reset(new NamespaceImpl(nullptr, *this));
        }

        for (auto c : globalNsChildren) {
            if (!globalNsPtr->processChild(c)) {
                return false;
            }
        }

    } while (false);

    // TODO: store unexpected children
    return true;
}

bool ProtocolImpl::validateSchema(::xmlNodePtr node)
{
    SchemaImplPtr schema(new SchemaImpl(node, m_logger));
    if (!schema->processNode()) {
        return false;
    }

    if (!m_schema) {
        m_schema = std::move(schema);
        return true;
    }

    auto& props = schema->props();
    auto& origProps = m_schema->props();
    for (auto& p : props) {
        auto iter = origProps.find(p.first);
        if ((iter == origProps.end()) ||
            (iter->second != p.second)) {

            logError() << XmlWrap::logPrefix(node) <<
                "Value of \"" << p.first <<
                "\" property of \"" << node->name << "\" element differs from the first one.";
            return false;
        }
    }

    auto& attrs = schema->unknownAttributes();
    auto& origAttrs = m_schema->unknownAttributes();
    for (auto& a : attrs) {
        auto iter = origAttrs.find(a.first);
        if (iter == origAttrs.end()) {
            origAttrs.insert(a);
            continue;
        }

        if (iter->second == a.second) {
            continue;
        }

        logWarning() << XmlWrap::logPrefix(node) <<
            "Value of \"" << a.first <<
            "\" attribubes of \"" << node->name << "\" element differs from the previous one.";
    }

    auto& children = schema->unknownChiltren();
    auto& origChildren = m_schema->unknownChiltren();
    for (auto& c : children) {
        origChildren.push_back(c);
    }

    return true;
}

bool ProtocolImpl::validateNewSchema(::xmlNodePtr node)
{
    m_schema.reset(new SchemaImpl(node, m_logger));
    return m_schema->processNode();
}

LogWrapper ProtocolImpl::logError() const
{
    return bbmp::logError(m_logger);
}

LogWrapper ProtocolImpl::logWarning() const
{
    return bbmp::logWarning(m_logger);
}

} // namespace bbmp
