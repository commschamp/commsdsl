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

std::string genStrReplace(const std::string& str, const std::string& what, const std::string& with);
std::string genStrToName(const std::string& value);
std::vector<std::string> genStrSplitByAnyChar(const std::string& str, const std::string& splitChars, bool compressed = true);
std::string genStrInsertIndent(const std::string& str);
unsigned genStrToUnsigned(const std::string& str);
bool genStrStartsWith(const std::string& str, const std::string& prefix);
bool genStrEndsWith(const std::string& str, const std::string& suffix);
std::string genStrToUpper(const std::string& str);
std::string genStrToLower(const std::string& str);
std::string genStrToMacroName(const std::string& str);
std::string genNumToString(std::uintmax_t value, unsigned hexWidth = 0U);
std::string genNumToString(std::intmax_t value);
std::string genNumToStringWithHexComment(std::intmax_t value);
std::string genNumToString(unsigned value, unsigned hexWidth = 0U);
const std::string& genBoolToString(bool value);
std::string genPathAddElem(const std::string& path, const std::string& elem);
std::string genPathUp(const std::string& path);
std::string genNameToNs(const std::string& value);

using ReplacementMap = std::map<std::string, std::string>;
std::string genProcessTemplate(const std::string& templ, const ReplacementMap& repl, bool tidyCode = false);

using StringsList = std::vector<std::string>;
std::string genStrListToString(
    const StringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");

void genAddToStrList(std::string&& value, StringsList& list);
void genAddToStrList(const std::string& value, StringsList& list);

std::string genStrMakeMultiline(const std::string& value, unsigned len = 60, bool dropReplacedWhiteChar = true);

std::string genReadFileContents(const std::string& filePath);
bool genIsFileReadable(const std::string& filePath);

const std::string& genDisplayName(const std::string& dslDisplayName, const std::string& dslName);

} // namespace util

} // namespace gen

} // namespace commsdsl
