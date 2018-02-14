#pragma once

#include "XmlWrap.h"
#include "Logger.h"

namespace bbmp
{

class SchemaImpl
{
public:
    SchemaImpl(::xmlNodePtr node, Logger& logger);

    bool processNode();

private:
    ::xmlNodePtr m_node = nullptr;
    Logger& m_logger;
};

} // namespace bbmp
