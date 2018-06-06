#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <vector>

namespace commsdsl2comms
{

namespace common
{

const std::string& emptyString();
const std::string& headerSuffix();
const std::string& includeStr();
const std::string& messageStr();
const std::string& fieldBaseStr();
const std::string& commsStr();

void nameToClass(std::string& str);
std::string nameToClassCopy(const std::string& str);
void updateName(std::string& str);
std::string adjustName(const std::string& str);
std::string numToString(std::uintmax_t value);
std::string makeMultiline(const std::string& value, unsigned len = 60);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

using IncludesList = std::vector<std::string>;
void mergeIncludes(const IncludesList& from, IncludesList& to);
std::string includesToStatements(const IncludesList& list);

} // namespace common

} // namespace commsdsl2comms
