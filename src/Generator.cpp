#include "Generator.h"

namespace commsdsl2comms
{

bool Generator::generate(const Generator::FilesList& files)
{
    for (auto& f : files) {
        m_logger.log(commsdsl::ErrorLevel_Info, "Parsing " + f);
        if (!m_protocol.parse(f)) {
            return false;
        }

        if (m_logger.hadWarning()) {
            m_logger.log(commsdsl::ErrorLevel_Error, "Warning treated as error");
            return false;
        }
    }

    if (!m_protocol.validate()) {
        return false;
    }

    if (m_logger.hadWarning()) {
        m_logger.log(commsdsl::ErrorLevel_Error, "Warning treated as error");
        return false;
    }

    // TODO: generate

    return true;
}

} // namespace commsdsl2comms
