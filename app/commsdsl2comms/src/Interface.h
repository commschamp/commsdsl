#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Interface.h"

#include "Field.h"

namespace commsdsl2comms
{

class Generator;
class Interface
{
public:

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Interface(Generator& gen, const commsdsl::Interface& msg)
      : m_generator(gen),
        m_dslObj(msg)
    {
    }

    bool prepare();

    bool write();

    const std::string& name() const;

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

    bool hasVersion() const;

private:

    bool writeProtocol();
    bool writePluginHeader();
    bool writePluginSrc();
    std::string getDescription() const;
    std::string getFieldsClassesList() const;
    std::string getFieldsAccessList() const;
    std::string getIncludes() const;
    std::string getFieldsAccessDoc() const;
    std::string getFieldsDef() const;
    std::string getFieldsOpts() const;
    unsigned getHexMsgIdWidth() const;

    Generator& m_generator;
    commsdsl::Interface m_dslObj;
    std::string m_externalRef;
    std::vector<FieldPtr> m_fields;
};

using InterfacePtr = std::unique_ptr<Interface>;

inline
InterfacePtr createInterface(Generator& gen, const commsdsl::Interface& msg)
{
    return InterfacePtr(new Interface(gen, msg));
}

} // namespace commsdsl2comms
