#pragma once

#include <string>
#include <map>
#include <cstdint>

namespace commsdsl2comms
{

namespace common
{

const std::string& emptyString();
const std::string& headerSuffix();
const std::string& includeStr();
const std::string& messageStr();

void nameToClass(std::string& str);
void updateName(std::string& str);
std::string adjustName(const std::string& str);
std::string numToString(std::uintmax_t value);
std::string makeMultiline(const std::string& value, unsigned len = 60);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

} // namespace common

} // namespace commsdsl2comms
