#pragma once

namespace commsdsl2comms
{

class Generator;
class DefaultOptions
{
public:
    static bool write(Generator& generator);

private:
    explicit DefaultOptions(Generator& generator) : m_generator(generator) {}

    bool writeDefinition() const;

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
