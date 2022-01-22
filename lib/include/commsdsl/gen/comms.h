#pragma once

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Generator.h"

#include <string>

namespace commsdsl
{

namespace gen
{

namespace comms
{

std::string className(const std::string& name);
std::string namespaceName(const std::string& name);

std::string scopeFor(
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

std::string relHeaderPathFor(
    const Elem& elem, 
    const Generator& generator);

std::string namespaceBeginFor(
    const Elem& elem, 
    const Generator& generator);           

std::string namespaceEndFor(
    const Elem& elem, 
    const Generator& generator);     

void prepareIncludeStatement(std::vector<std::string>& includes);          

} // namespace comms

} // namespace gen

} // namespace commsdsl
