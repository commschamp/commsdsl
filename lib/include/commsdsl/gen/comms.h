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

#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenNamespace.h"

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseIntField.h"
#include "commsdsl/parse/ParseFloatField.h"
#include "commsdsl/parse/ParseUnits.h"

#include <string>

namespace commsdsl
{

namespace gen
{

namespace comms
{

[[nodiscard]] std::string genClassName(const std::string& name);
[[nodiscard]] std::string genAccessName(const std::string& name);

[[nodiscard]] std::string genFullNameFor(const GenElem& elem);

[[nodiscard]] std::string genScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genCommonScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForOptions(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForInput(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForFactory(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForDispatch(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForMsgId(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForNamespaceMember(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForRoot(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForChecksum(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genScopeForCustomLayer(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

[[nodiscard]] std::string genRelHeaderPathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
[[nodiscard]] std::string genRelSourcePathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
[[nodiscard]] std::string genRelCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genRelHeaderPathForField(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genRelHeaderForOptions(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);
[[nodiscard]] std::string genRelHeaderForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genRelHeaderForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genRelHeaderForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genRelHeaderForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace = true);
[[nodiscard]] std::string genRelSourceForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace = true);
[[nodiscard]] std::string genRelHeaderForLayer(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genRelHeaderForChecksum(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genRelHeaderForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace = true);
[[nodiscard]] std::string genRelHeaderForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);
[[nodiscard]] std::string genRelSourceForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);

[[nodiscard]] std::string genHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genSourcePathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genHeaderPathForField(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genHeaderPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genHeaderPathForOptions(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genHeaderPathForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genHeaderPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genHeaderPathForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genHeaderPathForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genHeaderPathRoot(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genSourcePathRoot(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genPathForDoc(const std::string& name, const GenGenerator& generator);

[[nodiscard]] std::string genInputCodeRelPathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeAbsPathFor(const GenElem& elem, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeRelPathForRoot(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeAbsPathForRoot(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeRelPathForDoc(const std::string& name);
[[nodiscard]] std::string genInputCodeAbsPathForDoc(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeRelPathForOptions(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeAbsPathForOptions(const std::string& name, const GenGenerator& generator);
[[nodiscard]] std::string genInputCodeRelPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genInputCodeAbsPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genInputCodeRelPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
[[nodiscard]] std::string genInputCodeAbsPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);

[[nodiscard]] std::string genNamespaceBeginFor(
    const GenElem& elem,
    const GenGenerator& generator);

[[nodiscard]] std::string genNamespaceEndFor(
    const GenElem& elem,
    const GenGenerator& generator);

void genPrepareIncludeStatement(std::vector<std::string>& includes);

[[nodiscard]] const std::string& genCppIntTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len);
[[nodiscard]] std::string genCppIntChangedSignTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len);
[[nodiscard]] const std::string& genCppFloatTypeFor(commsdsl::parse::ParseFloatField::ParseType value);

[[nodiscard]] bool genIsGlobalField(const GenElem& elem);
[[nodiscard]] bool genIsInterfaceDeepMemberField(const GenElem& elem);
[[nodiscard]] bool genIsInterfaceShallowMemberField(const GenElem& elem);
[[nodiscard]] bool genIsMessageShallowMemberField(const GenElem& elem);
[[nodiscard]] bool genIsVersionOptionalField(const GenElem& elem, const GenGenerator& generator);

[[nodiscard]] unsigned genSinceVersionOf(const GenElem& elem);

[[nodiscard]] const std::string& genParseEndianToOpt(commsdsl::parse::ParseEndian value);

[[nodiscard]] const std::string& genParseUnitsToOpt(commsdsl::parse::ParseUnits value);

[[nodiscard]] std::size_t genMaxPossibleLength();

[[nodiscard]] std::size_t genAddLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
