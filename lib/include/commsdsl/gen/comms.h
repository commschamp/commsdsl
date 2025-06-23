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

std::string className(const std::string& name);
std::string accessName(const std::string& name);

std::string fullNameFor(const GenElem& elem);

std::string scopeFor(
    const GenElem& elem, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);

std::string commonScopeFor(
    const GenElem& elem, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForOptions(
    const std::string& name, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);  

std::string scopeForInput(
    const std::string& name, 
    const GenGenerator& generator, 
    const GenNamespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);  

std::string scopeForFactory(
    const std::string& name, 
    const GenGenerator& generator, 
    const GenNamespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);      

std::string scopeForDispatch(
    const std::string& name, 
    const GenGenerator& generator, 
    const GenNamespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForMsgId(
    const std::string& name, 
    const GenGenerator& generator, 
    const GenNamespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);     

std::string scopeForNamespaceMember(
    const std::string& name, 
    const GenGenerator& generator, 
    const GenNamespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);      

std::string scopeForRoot(
    const std::string& name, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);

std::string scopeForChecksum(
    const std::string& name, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForCustomLayer(
    const GenElem& elem, 
    const GenGenerator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);      

std::string relHeaderPathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
std::string relSourcePathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace = true);
std::string relCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
std::string relHeaderPathForField(const std::string& name, const GenGenerator& generator); 
std::string relHeaderForOptions(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true); 
std::string relHeaderForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns); 
std::string relHeaderForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string relHeaderForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string relHeaderForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string relSourceForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string relHeaderForLayer(const std::string& name, const GenGenerator& generator); 
std::string relHeaderForChecksum(const std::string& name, const GenGenerator& generator); 
std::string relHeaderForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace = true); 
std::string relHeaderForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true); 
std::string relSourceForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace = true); 

std::string headerPathFor(const GenElem& elem, const GenGenerator& generator);
std::string sourcePathFor(const GenElem& elem, const GenGenerator& generator);
std::string headerPathForField(const std::string& name, const GenGenerator& generator); 
std::string headerPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns); 
std::string headerPathForOptions(const std::string& name, const GenGenerator& generator); 
std::string headerPathForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns); 
std::string headerPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns); 
std::string headerPathForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns); 
std::string commonHeaderPathFor(const GenElem& elem, const GenGenerator& generator);
std::string headerPathRoot(const std::string& name, const GenGenerator& generator);
std::string sourcePathRoot(const std::string& name, const GenGenerator& generator);
std::string pathForDoc(const std::string& name, const GenGenerator& generator); 

std::string inputCodePathFor(const GenElem& elem, const GenGenerator& generator);
std::string inputCodePathForRoot(const std::string& name, const GenGenerator& generator);
std::string inputCodePathForDoc(const std::string& name, const GenGenerator& generator);
std::string inputCodePathForOptions(const std::string& name, const GenGenerator& generator);
std::string inputCodePathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);
std::string inputCodePathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns);

std::string namespaceBeginFor(
    const GenElem& elem, 
    const GenGenerator& generator);           

std::string namespaceEndFor(
    const GenElem& elem, 
    const GenGenerator& generator);     

void prepareIncludeStatement(std::vector<std::string>& includes); 

const std::string& cppIntTypeFor(commsdsl::parse::ParseIntField::Type value, std::size_t len);
std::string cppIntChangedSignTypeFor(commsdsl::parse::ParseIntField::Type value, std::size_t len);
const std::string& cppFloatTypeFor(commsdsl::parse::ParseFloatField::Type value);

bool isGlobalField(const GenElem& elem);
bool isInterfaceDeepMemberField(const GenElem& elem);
bool isInterfaceShallowMemberField(const GenElem& elem);
bool isMessageShallowMemberField(const GenElem& elem);
bool isVersionOptionalField(const GenElem& elem, const GenGenerator& generator);

unsigned sinceVersionOf(const GenElem& elem);

const std::string& dslEndianToOpt(commsdsl::parse::ParseEndian value);

const std::string& dslUnitsToOpt(commsdsl::parse::ParseUnits value);

std::size_t maxPossibleLength();

std::size_t addLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
