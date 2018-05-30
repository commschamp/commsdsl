#pragma once

#include <vector>
#include <string>

#include "commsdsl/Protocol.h"
#include "Logger.h"

namespace commsdsl2comms
{

class Generator
{
public:
    using FilesList = std::vector<std::string>;

    Generator(Logger& logger)
      : m_logger(logger)
    {
    }

    bool generate(const FilesList& files);
private:
    Logger& m_logger;
    commsdsl::Protocol m_protocol;
};

} // namespace commsdsl2comms
