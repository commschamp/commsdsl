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

} // namespace comms

} // namespace gen

} // namespace commsdsl
