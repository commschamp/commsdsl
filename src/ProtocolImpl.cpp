#include "ProtocolImpl.h"

#include <iostream>
#include <type_traits>

#include "XmlWrap.h"

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

    // TODO
    return true;
}

bool ProtocolImpl::validateSchema(::xmlNodePtr node)
{
    if (!m_schema) {
        return validateNewSchema(node);
    }

    // TODO
    return true;
}

bool ProtocolImpl::validateNewSchema(::xmlNodePtr node)
{
    m_schema.reset(new SchemaImpl(node, m_logger));
    return m_schema->processNode();
}

LogWrapper ProtocolImpl::logError()
{
    return bbmp::logError(m_logger);
}


} // namespace bbmp
