#pragma once

namespace commsdsl2comms
{

class Generator;
class FieldBase
{
public:
    static bool write(Generator& generator);

private:
    explicit FieldBase(Generator& generator) : m_generator(generator) {}

    bool writeDefinition() const;

private:
    Generator& m_generator;
};

} // namespace commsdsl2comms
