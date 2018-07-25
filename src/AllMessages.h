#pragma once

namespace commsdsl2comms
{

class Generator;
class AllMessages
{
public:
    static bool write(Generator& generator);

private:
    explicit AllMessages(Generator& generator) : m_generator(generator) {}

    bool writeProtocolDefinition() const;
    bool writePluginDefinition() const;

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
