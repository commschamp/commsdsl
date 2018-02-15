#pragma once

#include <memory>
#include <string>
#include <functional>

#include "BbmpApi.h"
#include "ErrorLevel.h"
#include "Schema.h"

namespace bbmp
{

class ProtocolImpl;
class BBMP_API Protocol
{
public:
    using ErrorReportFunction = std::function<void (ErrorLevel, const std::string&)>;

    Protocol();
    ~Protocol();

    void setErrorReportCallback(ErrorReportFunction&& cb);

    bool parse(const std::string& input);
    bool validate();

    Schema schema() const;

private:
    std::unique_ptr<ProtocolImpl> m_pImpl;
};

} // namespace bbmp
