#pragma once

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Message.h"

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/IntField.h"
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
std::string namespaceName(const std::string& name);

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

std::string scopeForInterface(
    const std::string& name, 
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
    bool addMainNamespace = true, 
    bool addElement = true);  

std::string scopeForRoot(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace = true, 
    bool addElement = true);

std::string relHeaderPathFor(const Elem& elem, const Generator& generator);
std::string relCommonHeaderPathFor(const Elem& elem, const Generator& generator);
std::string relHeaderPathForField(const std::string& name, const Generator& generator); 
std::string relHeaderForOptions(const std::string& name, const Generator& generator); 
std::string relHeaderForRoot(const std::string& name, const Generator& generator); 

std::string headerPathFor(const Elem& elem, const Generator& generator);
std::string commonHeaderPathFor(const Elem& elem, const Generator& generator);

std::string inputCodePathFor(const Elem& elem, const Generator& generator);

std::string namespaceBeginFor(
    const Elem& elem, 
    const Generator& generator);           

std::string namespaceEndFor(
    const Elem& elem, 
    const Generator& generator);     

void prepareIncludeStatement(std::vector<std::string>& includes); 

const std::string& cppIntTypeFor(commsdsl::parse::IntField::Type value, std::size_t len);

bool isGlobalField(const Elem& elem);

unsigned sinceVersionOf(const Elem& elem);

const std::string& dslEndianToOpt(commsdsl::parse::Endian value);

const std::string& dslUnitsToOpt(commsdsl::parse::Units value);

std::string messageIdStrFor(const commsdsl::gen::Message& msg, const Generator& generator);

std::size_t maxPossibleLength();

std::size_t addLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
