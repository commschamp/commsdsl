#pragma once

namespace commsdsl2comms
{

class Generator;
class Cmake
{
public:
    static bool write(Generator& generator);

private:
    explicit Cmake(Generator& generator) : m_generator(generator) {}

    bool writeMain() const;
    bool writePlugin() const;

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
