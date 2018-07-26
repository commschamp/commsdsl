#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/Units.h"

namespace commsdsl2comms
{

namespace common
{

const std::string& emptyString();
const std::string& headerSuffix();
const std::string& srcSuffix();
const std::string& includeStr();
const std::string& messageStr();
const std::string& messageClassStr();
const std::string& frameStr();
const std::string& fieldStr();
const std::string& fieldBaseStr();
const std::string& commsStr();
const std::string& indentStr();
const std::string& doxygenPrefixStr();
const std::string& fieldsSuffixStr();
const std::string& layersSuffixStr();
const std::string& membersSuffixStr();
const std::string& optFieldSuffixStr();
const std::string& prefixFieldSuffixStr();
const std::string& defaultOptionsStr();
const std::string& msgIdEnumNameStr();
const std::string& msgIdPrefixStr();
const std::string& allMessagesStr();
const std::string& checksumStr();
const std::string& layerStr();
const std::string& pluginNsStr();
const std::string& serHiddenStr();
const std::string& cmakeListsFileStr();
const std::string& transportMessageSuffixStr();

void nameToClass(std::string& str);
std::string nameToClassCopy(const std::string& str);
void nameToAccess(std::string& str);
std::string nameToAccessCopy(const std::string& str);
void updateName(std::string& str);
std::string adjustName(const std::string& str);
std::string numToString(std::uintmax_t value, bool hexOut = false);
std::string numToString(std::intmax_t value);

inline
std::string numToString(unsigned value, bool hexOut = false)
{
    return numToString(static_cast<std::uintmax_t>(value), hexOut);
}

std::string makeMultilineCopy(const std::string& value, unsigned len = 60);
void insertIndent(std::string& str);
std::string insertIndentCopy(const std::string& str);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl);

using StringsList = std::vector<std::string>;
void mergeIncludes(const StringsList& from, StringsList& to);
void mergeInclude(const std::string& inc, StringsList& to);
std::string includesToStatements(const StringsList& list);
std::string listToString(
    const StringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");
void addToList(const std::string& what, StringsList& to);

const std::string& dslEndianToOpt(commsdsl::Endian value);
const std::string& dslUnitsToOpt(commsdsl::Units value);

} // namespace common

} // namespace commsdsl2comms
