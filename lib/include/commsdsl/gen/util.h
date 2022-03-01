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
std::string strInsertIndent(const std::string& str);
unsigned strToUnsigned(const std::string& str);
bool strStartsWith(const std::string& str, const std::string& prefix);
std::string strToUpper(const std::string& str);
std::string numToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string numToString(std::intmax_t value);
std::string numToString(unsigned value, unsigned hexWidth = 0U);
const std::string& boolToString(bool value);
std::string pathAddElem(const std::string& path, const std::string& elem);
std::string pathUp(const std::string& path);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

using StringsList = std::vector<std::string>;
std::string strListToString(
    const StringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");

void addToStrList(std::string&& value, StringsList& list);
void addToStrList(const std::string& value, StringsList& list);

std::string strMakeMultiline(const std::string& value, unsigned len = 60);

std::string readFileContents(const std::string& filePath);
bool isFileReadable(const std::string& filePath);

const std::string& displayName(const std::string& dslDisplayName, const std::string& dslName);

} // namespace util

} // namespace gen

} // namespace commsdsl
