//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace commsdsl
{

namespace gen
{

namespace util
{

[[nodiscard]] std::string genStrReplace(const std::string& str, const std::string& what, const std::string& with);
[[nodiscard]] std::string genStrToName(const std::string& value);
[[nodiscard]] std::vector<std::string> genStrSplitByAnyChar(const std::string& str, const std::string& splitChars, bool compressed = true);
[[nodiscard]] std::vector<std::string> genStrSplit(const std::string& str, const std::string& sep, bool compressed = true);
[[nodiscard]] std::string genStrInsertIndent(const std::string& str);
[[nodiscard]] unsigned genStrToUnsigned(const std::string& str);
[[nodiscard]] bool genStrStartsWith(const std::string& str, const std::string& prefix);
[[nodiscard]] bool genStrEndsWith(const std::string& str, const std::string& suffix);
[[nodiscard]] std::string genStrToUpper(const std::string& str);
[[nodiscard]] std::string genStrToLower(const std::string& str);
[[nodiscard]] std::string genStrToMacroName(const std::string& str);
[[nodiscard]] std::string genNumToString(std::uintmax_t value, unsigned hexWidth = 0U);
[[nodiscard]] std::string genNumToString(std::intmax_t value);
[[nodiscard]] std::string genNumToStringWithHexComment(std::intmax_t value);
[[nodiscard]] std::string genNumToString(unsigned value, unsigned hexWidth = 0U);
[[nodiscard]] const std::string& genBoolToString(bool value);
[[nodiscard]] std::string genPathAddElem(const std::string& path, const std::string& elem);
[[nodiscard]] std::string genPathUp(const std::string& path);
[[nodiscard]] std::string genNameToNs(const std::string& value);
[[nodiscard]] std::string genScopeToRelPath(const std::string& value);
void genRemoveHeadingTrailingWhitespaces(std::string& str);

using GenReplacementMap = std::map<std::string, std::string>;
[[nodiscard]] std::string genProcessTemplate(const std::string& templ, const GenReplacementMap& repl, bool tidyCode = false);

using GenStringsList = std::vector<std::string>;
[[nodiscard]] std::string genStrListToString(
    const GenStringsList& list,
    const std::string& join = ",\n",
    const std::string& last = "\n");

void genAddToStrList(std::string&& value, GenStringsList& list);
void genAddToStrList(const std::string& value, GenStringsList& list);

[[nodiscard]] std::string genStrMakeMultiline(const std::string& value, unsigned len = 60, bool dropReplacedWhiteChar = true);

[[nodiscard]] std::string genReadFileContents(const std::string& filePath);
[[nodiscard]] bool genIsFileReadable(const std::string& filePath);

[[nodiscard]] const std::string& genDisplayName(const std::string& dslDisplayName, const std::string& dslName);

} // namespace util

} // namespace gen

} // namespace commsdsl
