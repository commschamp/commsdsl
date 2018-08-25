#pragma once

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

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
