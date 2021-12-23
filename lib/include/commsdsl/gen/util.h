#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>

namespace commsdsl  
{

namespace gen
{  

namespace util
{

std::string strReplace(const std::string& str, const std::string& what, const std::string& with);
std::string strToName(const std::string& value);
std::vector<std::string> strSplitByAnyCharCompressed(const std::string& str, const std::string& splitChars);
std::string numToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string numToString(std::intmax_t value);
std::string numToString(unsigned value, unsigned hexWidth = 0U);
std::string pathAddElem(const std::string& path, const std::string& elem);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

} // namespace util

} // namespace gen

} // namespace commsdsl
