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

std::string genClassName(const std::string& name);
std::string genAccessName(const std::string& name);

std::string genFullNameFor(const GenElem& elem);

std::string genScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genCommonScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForOptions(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForInput(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForFactory(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForDispatch(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForMsgId(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForNamespaceMember(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForRoot(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForChecksum(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genScopeForCustomLayer(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace = true,
    bool addElement = true);

std::string genRelHeaderPathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
std::string genRelSourcePathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
std::string genRelCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
std::string genRelHeaderPathForField(const std::string& name, const GenGenerator& generator);
std::string genRelHeaderForOptions(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);
std::string genRelHeaderForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genRelHeaderForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genRelHeaderForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genRelHeaderForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genRelSourceForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genRelHeaderForLayer(const std::string& name, const GenGenerator& generator);
std::string genRelHeaderForChecksum(const std::string& name, const GenGenerator& generator);
std::string genRelHeaderForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace = true);
std::string genRelHeaderForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);
std::string genRelSourceForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true);

std::string genHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
std::string genSourcePathFor(const GenElem& elem, const GenGenerator& generator);
std::string genHeaderPathForField(const std::string& name, const GenGenerator& generator);
std::string genHeaderPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genHeaderPathForOptions(const std::string& name, const GenGenerator& generator);
std::string genHeaderPathForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genHeaderPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genHeaderPathForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
std::string genHeaderPathRoot(const std::string& name, const GenGenerator& generator);
std::string genSourcePathRoot(const std::string& name, const GenGenerator& generator);
std::string genPathForDoc(const std::string& name, const GenGenerator& generator);

std::string genInputCodePathFor(const GenElem& elem, const GenGenerator& generator);
std::string genInputCodePathForRoot(const std::string& name, const GenGenerator& generator);
std::string genInputCodePathForDoc(const std::string& name, const GenGenerator& generator);
std::string genInputCodePathForOptions(const std::string& name, const GenGenerator& generator);
std::string genInputCodePathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string genInputCodePathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);

std::string genNamespaceBeginFor(
    const GenElem& elem,
    const GenGenerator& generator);

std::string genNamespaceEndFor(
    const GenElem& elem,
    const GenGenerator& generator);

void genPrepareIncludeStatement(std::vector<std::string>& includes);

const std::string& genCppIntTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len);
std::string genCppIntChangedSignTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len);
const std::string& genCppFloatTypeFor(commsdsl::parse::ParseFloatField::ParseType value);

bool genIsGlobalField(const GenElem& elem);
bool genIsInterfaceDeepMemberField(const GenElem& elem);
bool genIsInterfaceShallowMemberField(const GenElem& elem);
bool genIsMessageShallowMemberField(const GenElem& elem);
bool genIsVersionOptionalField(const GenElem& elem, const GenGenerator& generator);

unsigned genSinceVersionOf(const GenElem& elem);

const std::string& genParseEndianToOpt(commsdsl::parse::ParseEndian value);

const std::string& genParseUnitsToOpt(commsdsl::parse::ParseUnits value);

std::size_t genMaxPossibleLength();

std::size_t genAddLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
