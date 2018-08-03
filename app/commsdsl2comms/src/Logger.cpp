#include "Logger.h"

#include <type_traits>
#include <iostream>

namespace commsdsl2comms
{

void Logger::log(commsdsl::ErrorLevel level, const std::string& msg)
{
    if (level < m_minLevel) {
        return;
    }

    if (m_warnAsErr && (level == commsdsl::ErrorLevel_Warning)) {
        m_hadWarning = true;
    }

    static const std::string PrefixMap[] = {
        "[DEBUG]: ",
        "[INFO]: ",
        "[WARNING]: ",
        "[ERROR]: "
    };

    static const std::size_t PrefixMapSize = std::extent<decltype(PrefixMap)>::value;

    static_assert(PrefixMapSize == commsdsl::ErrorLevel_NumOfValues, "Wrong map");
    if (commsdsl::ErrorLevel_NumOfValues <= level) {
        level = commsdsl::ErrorLevel_Error;
    }

    std::ostream* stream = &std::cerr;
    if (level < commsdsl::ErrorLevel_Warning) {
        stream = &std::cout;
    }

    *stream << PrefixMap[level] << msg << std::endl;
}

} // namespace commsdsl2comms
