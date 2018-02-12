#pragma once

#include <string>
#include <memory>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

namespace bbmp
{

class ProtocolImpl
{
public:
    bool parse(const std::string& input);

private:
    struct XmlDocFree
    {
        void operator()(::xmlDocPtr p) const
        {
            ::xmlFree(p);
        }
    };

    using XmlDocPtr = std::unique_ptr<::xmlDoc, XmlDocFree>;
    using DocsList = std::vector<XmlDocPtr>;

    DocsList m_docs;
};

} // namespace bbmp
