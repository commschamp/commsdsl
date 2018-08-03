#pragma once

namespace commsdsl
{

namespace util
{

template <typename T>
constexpr unsigned toUnsigned(T val)
{
    return static_cast<unsigned>(val);
}

} // namespace util

} // namespace commsdsl
