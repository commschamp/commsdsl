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

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/IntField.h"
#include "commsdsl/parse/FloatField.h"
#include "commsdsl/parse/Units.h"

#include <string>

namespace commsdsl
{

namespace gen
{

namespace comms
{

std::string className(const std::string& name);
std::string accessName(const std::string& name);

std::string fullNameFor(const Elem& elem);

std::string scopeFor(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);

std::string commonScopeFor(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForOptions(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);  

std::string scopeForInput(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);  

std::string scopeForFactory(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);      

std::string scopeForDispatch(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForMsgId(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace = true, 
    bool addElement = true);     

std::string scopeForRoot(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);

std::string scopeForChecksum(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);    

std::string scopeForCustomLayer(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);      

std::string relHeaderPathFor(const Elem& elem, const Generator& generator, bool addMainNamespace = true);
std::string relSourcePathFor(const Elem& elem, const Generator& generator, bool addMainNamespace = true);
std::string relCommonHeaderPathFor(const Elem& elem, const Generator& generator);
std::string relHeaderPathForField(const std::string& name, const Generator& generator); 
std::string relHeaderForOptions(const std::string& name, const Generator& generator, bool addMainNamespace = true); 
std::string relHeaderForDispatch(const std::string& name, const Generator& generator, const Namespace& ns); 
std::string relHeaderForFactory(const std::string& name, const Generator& generator, const Namespace& ns);
std::string relHeaderForMsgId(const std::string& name, const Generator& generator, const Namespace& ns);
std::string relHeaderForLayer(const std::string& name, const Generator& generator); 
std::string relHeaderForChecksum(const std::string& name, const Generator& generator); 
std::string relHeaderForInput(const std::string& name, const Generator& generator, const Namespace& ns, bool addMainNamespace = true); 
std::string relHeaderForRoot(const std::string& name, const Generator& generator, bool addMainNamespace = true); 
std::string relSourceForRoot(const std::string& name, const Generator& generator, bool addMainNamespace = true); 

std::string headerPathFor(const Elem& elem, const Generator& generator);
std::string sourcePathFor(const Elem& elem, const Generator& generator);
std::string headerPathForField(const std::string& name, const Generator& generator); 
std::string headerPathForInput(const std::string& name, const Generator& generator, const Namespace& ns); 
std::string headerPathForOptions(const std::string& name, const Generator& generator); 
std::string headerPathForDispatch(const std::string& name, const Generator& generator, const Namespace& ns); 
std::string headerPathForFactory(const std::string& name, const Generator& generator, const Namespace& ns); 
std::string headerPathForMsgId(const std::string& name, const Generator& generator, const Namespace& ns); 
std::string commonHeaderPathFor(const Elem& elem, const Generator& generator);
std::string headerPathRoot(const std::string& name, const Generator& generator);
std::string sourcePathRoot(const std::string& name, const Generator& generator);
std::string pathForDoc(const std::string& name, const Generator& generator); 

std::string inputCodePathFor(const Elem& elem, const Generator& generator);
std::string inputCodePathForRoot(const std::string& name, const Generator& generator);
std::string inputCodePathForDoc(const std::string& name, const Generator& generator);
std::string inputCodePathForOptions(const std::string& name, const Generator& generator);
std::string inputCodePathForInput(const std::string& name, const Generator& generator, const Namespace& ns);
std::string inputCodePathForFactory(const std::string& name, const Generator& generator, const Namespace& ns);

std::string namespaceBeginFor(
    const Elem& elem, 
    const Generator& generator);           

std::string namespaceEndFor(
    const Elem& elem, 
    const Generator& generator);     

void prepareIncludeStatement(std::vector<std::string>& includes); 

const std::string& cppIntTypeFor(commsdsl::parse::IntField::Type value, std::size_t len);
std::string cppIntChangedSignTypeFor(commsdsl::parse::IntField::Type value, std::size_t len);
const std::string& cppFloatTypeFor(commsdsl::parse::FloatField::Type value);

bool isGlobalField(const Elem& elem);
bool isInterfaceDeepMemberField(const Elem& elem);
bool isInterfaceShallowMemberField(const Elem& elem);
bool isMessageShallowMemberField(const Elem& elem);
bool isVersionOptionalField(const Elem& elem, const Generator& generator);

unsigned sinceVersionOf(const Elem& elem);

const std::string& dslEndianToOpt(commsdsl::parse::Endian value);

const std::string& dslUnitsToOpt(commsdsl::parse::Units value);

std::size_t maxPossibleLength();

std::size_t addLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
