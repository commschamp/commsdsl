#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <vector>

#include "commsdsl/Endian.h"

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
const std::string& indentStr();
const std::string& doxigenPrefixStr();

void nameToClass(std::string& str);
std::string nameToClassCopy(const std::string& str);
void nameToAcces(std::string& str);
std::string nameToAccessCopy(const std::string& str);
void updateName(std::string& str);
std::string adjustName(const std::string& str);
std::string numToString(std::uintmax_t value);
std::string makeMultiline(const std::string& value, unsigned len = 60);
void insertIndent(std::string& str);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

using StringsList = std::vector<std::string>;
void mergeIncludes(const StringsList& from, StringsList& to);
std::string includesToStatements(const StringsList& list);
std::string listToString(
    const StringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");

const std::string& dslEndianToOpt(commsdsl::Endian value);

} // namespace common

} // namespace commsdsl2comms
