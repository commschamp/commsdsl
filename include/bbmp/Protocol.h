#pragma once

#include <memory>
#include <string>

#include "BbmpApi.h"

namespace bbmp
{

class ProtocolImpl;
class BBMP_API Protocol
{
public:
    Protocol();
    ~Protocol();

    bool parse(const std::string& input);
private:
    std::unique_ptr<ProtocolImpl> m_pImpl;
};

} // namespace bbmp
