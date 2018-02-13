#include "ProtocolImpl.h"

#include <iostream>

namespace bbmp
{

ProtocolImpl::ProtocolImpl()
{
    xmlSetStructuredErrorFunc(this, &ProtocolImpl::cbXmlErrorFunc);
}

bool ProtocolImpl::parse(const std::string& input)
{
    XmlDocPtr doc(xmlParseFile(input.c_str()));
    if (!doc) {
        std::cerr << "ERROR: Failed to parse" << input << std::endl;
        return false;
    }

    m_docs.push_back(std::move(doc));
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

    std::string message = std::string(err->file) + ':' + std::to_string(err->line) + ": " + err->message;

    if (!m_errorReportCb) {
        std::cerr << message << std::endl;
        return;
    }

    m_errorReportCb(level, message);
}

} // namespace bbmp
