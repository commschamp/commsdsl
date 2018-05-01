#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <limits>

#include "BbmpApi.h"
#include "ErrorLevel.h"
#include "Schema.h"
#include "Namespace.h"
#include "Field.h"

namespace bbmp
{

class ProtocolImpl;
class BBMP_API Protocol
{
public:
    using ErrorReportFunction = std::function<void (ErrorLevel, const std::string&)>;
    using NamespacesList = std::vector<Namespace>;
    using MessagesList = Namespace::MessagesList;

    Protocol();
    ~Protocol();

    void setErrorReportCallback(ErrorReportFunction&& cb);

    bool parse(const std::string& input);
    bool validate();

    Schema schema() const;
    NamespacesList namespaces() const;

    static constexpr unsigned notYetDeprecated()
    {
        return std::numeric_limits<unsigned>::max();
    }

    Field findField(const std::string& externalRef) const;

    MessagesList allMessages() const;

private:
    std::unique_ptr<ProtocolImpl> m_pImpl;
};

} // namespace bbmp
