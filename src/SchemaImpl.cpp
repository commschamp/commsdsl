#include "SchemaImpl.h"

namespace bbmp
{

SchemaImpl::SchemaImpl(::xmlNodePtr node, Logger& logger)
  : m_node(node),
    m_logger(logger)
{
}

bool SchemaImpl::processNode()
{
    static const XmlWrap::NamesList Names = {
        "package",
        "id",
        "version",
        "endian",
        "description"
    };

    auto props = XmlWrap::parseNodeProps(m_node);
    if (!XmlWrap::parseChildrenAsProps(m_node, Names, m_logger, props)) {
        return false;
    }

    // TODO: validate contents
    return true;
}

} // namespace bbmp
