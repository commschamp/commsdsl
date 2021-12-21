#pragma once

#include <string>
#include <cstdint>

namespace commsdsl  
{

namespace gen
{  

namespace util
{

std::string strReplace(const std::string& str, const std::string& what, const std::string& with);
std::string strToName(const std::string& value);
std::string numToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string numToString(std::intmax_t value);
std::string numToString(unsigned value, unsigned hexWidth = 0U);

} // namespace util

} // namespace gen

} // namespace commsdsl
