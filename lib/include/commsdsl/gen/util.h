//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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
std::vector<std::string> strSplitByAnyChar(const std::string& str, const std::string& splitChars, bool compressed = true);
std::string strInsertIndent(const std::string& str);
unsigned strToUnsigned(const std::string& str);
bool strStartsWith(const std::string& str, const std::string& prefix);
bool strEndsWith(const std::string& str, const std::string& suffix);
std::string strToUpper(const std::string& str);
std::string strToLower(const std::string& str);
std::string strToMacroName(const std::string& str);
std::string numToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string numToString(std::intmax_t value);
std::string numToString(unsigned value, unsigned hexWidth = 0U);
const std::string& boolToString(bool value);
std::string pathAddElem(const std::string& path, const std::string& elem);
std::string pathUp(const std::string& path);
std::string nameToNs(const std::string& value);

using ReplacementMap = std::map<std::string, std::string>;
std::string processTemplate(const std::string& templ, const ReplacementMap& repl, bool tidyCode = false);

using StringsList = std::vector<std::string>;
std::string strListToString(
    const StringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");

void addToStrList(std::string&& value, StringsList& list);
void addToStrList(const std::string& value, StringsList& list);

std::string strMakeMultiline(const std::string& value, unsigned len = 60, bool dropReplacedWhiteChar = true);

std::string readFileContents(const std::string& filePath);
bool isFileReadable(const std::string& filePath);

const std::string& displayName(const std::string& dslDisplayName, const std::string& dslName);

} // namespace util

} // namespace gen

} // namespace commsdsl
