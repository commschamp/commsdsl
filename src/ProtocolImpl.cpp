#include "ProtocolImpl.h"

#include <iostream>
#include <type_traits>
#include <cassert>

#include "XmlWrap.h"
#include "FieldImpl.h"

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
    m_logger << std::string(err->file) << ':' << err->line << ": " << err->message;
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


    using ProcessFunc = bool (ProtocolImpl::*)(::xmlNodePtr node);
    static const std::map<std::string, ProcessFunc> ParseFuncMap = {
        std::make_pair(common::fieldsStr(), &ProtocolImpl::processMultipleFields),
        std::make_pair(common::messageStr(), &ProtocolImpl::processMessage),
        std::make_pair(common::messagesStr(), &ProtocolImpl::processMultipleMessages),
        std::make_pair(common::frameStr(), &ProtocolImpl::processFrame),
        std::make_pair(common::framesStr(), &ProtocolImpl::processMultipleFrames),
    };

    auto childrenNodes = XmlWrap::getChildren(root);
    for (auto* c : childrenNodes) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = ParseFuncMap.find(cName);
        if (iter == ParseFuncMap.end()) {
            continue;
        }

        auto func = iter->second;
        bool result = (this->*func)(c);
        if (!result) {
            return false;
        }
    }

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

bool ProtocolImpl::processMultipleFields(::xmlNodePtr node)
{
    auto childrenNodes = XmlWrap::getChildren(node);
    for (auto* c : childrenNodes) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto field = FieldImpl::create(cName, c, *this);
        if (!field) {
            logError() << XmlWrap::logPrefix(c) << "Invalid field type \"" << cName << "\"";
            return false;
        }

        if (!field->parse()) {
            logError() << XmlWrap::logPrefix(c) << "Parsing of \"" << cName << "\" has failed.";
            return false;
        }

        auto& name = field->name();
        if (name.empty()) {
            logError() << XmlWrap::logPrefix(c) << "Field \"" << cName << "\" doesn't have any name.";
            return false;
        }

        auto iter = m_fields.find(name);
        if (iter != m_fields.end()) {
            logError() << XmlWrap::logPrefix(c) << "Field with name \"" << name << "\" has been already defined at " <<
                          iter->second->getNode()->doc->URL << ":" << iter->second->getNode()->line << '.';
            return false;
        }

        m_fields.insert(std::make_pair(name, std::move(field)));
    }

    return true;
}

bool ProtocolImpl::processMessage(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool ProtocolImpl::processMultipleMessages(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool ProtocolImpl::processFrame(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool ProtocolImpl::processMultipleFrames(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
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
