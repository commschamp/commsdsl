#include "ProtocolImpl.h"

#include <iostream>

namespace bbmp
{

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

} // namespace bbmp
