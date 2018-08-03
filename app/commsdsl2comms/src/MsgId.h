#pragma once

namespace commsdsl2comms
{

class Generator;
class MsgId
{
public:
    static bool write(Generator& generator);

private:
    explicit MsgId(Generator& generator) : m_generator(generator) {}

    bool writeDefinition() const;

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
