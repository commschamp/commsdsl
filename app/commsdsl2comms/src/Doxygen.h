#pragma once

#include <string>

namespace commsdsl2comms
{

class Generator;
class Doxygen
{
public:
    static bool write(Generator& generator);

private:
    explicit Doxygen(Generator& generator) : m_generator(generator) {}

    bool writeConf() const;
    bool writeLayout() const;
    bool writeNamespaces() const;
    bool writeMainpage() const;

    std::string getMessagesDoc() const;
    std::string getFieldsDoc() const;
    std::string getInterfacesDoc() const;
    std::string getFramesDoc() const;
    std::string getPlatformsDoc() const;
    std::string getCustomizeDoc() const;
    std::string getVersionDoc() const;

    Generator& m_generator;
};

} // namespace commsdsl2comms
