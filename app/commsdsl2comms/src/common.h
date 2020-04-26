//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
const std::string& serverInputMessagesStr();
const std::string& clientInputMessagesStr();
const std::string& serverInputStr();
const std::string& clientInputStr();
const std::string& checksumStr();
const std::string& layerStr();
const std::string& pluginNsStr();
const std::string& serHiddenStr();
const std::string& cmakeListsFileStr();
const std::string& transportMessageSuffixStr();
const std::string& pluginStr();
const std::string& origSuffixStr();
const std::string& forceEmptyDisplayNameStr();
const std::string& docStr();
const std::string& versionStr();
const std::string& optionsStr();
const std::string& bareMetalStr();
const std::string& dataViewStr();
const std::string& seqDefaultSizeStr();
const std::string& emptyOptionString();
const std::string& inputTestStr();
const std::string& testStr();
const std::string& inputStr();
const std::string& dispatchStr();
const std::string& commonSuffixStr();
const std::string& valSuffixStr();
const std::string& valueTypeStr();

void nameToClass(std::string& str);
std::string nameToClassCopy(const std::string& str);
void nameToAccess(std::string& str);
std::string nameToAccessCopy(const std::string& str);
void updateName(std::string& str);
std::string updateNameCopy(const std::string& str);
std::string adjustName(const std::string& str);
std::string numToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string numToString(std::intmax_t value);

inline
std::string numToString(unsigned value, unsigned hexWidth = 0U)
{
    return numToString(static_cast<std::uintmax_t>(value), hexWidth);
}

const std::string& boolToString(bool value);

std::string makeMultilineCopy(const std::string& value, unsigned len = 60);
std::string makeDoxygenMultilineCopy(const std::string& value, unsigned len = 60);

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
const std::string& displayName(const std::string& dslDisplayName, const std::string& dslName);

void toLower(std::string& str);
std::string toLowerCopy(const std::string& str);
void toUpper(std::string& str);
std::string toUpperCopy(const std::string& str);

} // namespace common

} // namespace commsdsl2comms
