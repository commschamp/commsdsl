#pragma once

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Message.h"

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

std::string scopeForDispatch(
    const std::string& name, 
    const Generator& generator, 
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
std::string relSrcPathFor(const Elem& elem, const Generator& generator, bool addMainNamespace = true);
std::string relCommonHeaderPathFor(const Elem& elem, const Generator& generator);
std::string relHeaderPathForField(const std::string& name, const Generator& generator); 
std::string relHeaderForOptions(const std::string& name, const Generator& generator, bool addMainNamespace = true); 
std::string relHeaderForDispatch(const std::string& name, const Generator& generator); 
std::string relHeaderForLayer(const std::string& name, const Generator& generator); 
std::string relHeaderForChecksum(const std::string& name, const Generator& generator); 
std::string relHeaderForInput(const std::string& name, const Generator& generator, bool addMainNamespace = true); 
std::string relHeaderForRoot(const std::string& name, const Generator& generator, bool addMainNamespace = true); 

std::string headerPathFor(const Elem& elem, const Generator& generator);
std::string headerPathForField(const std::string& name, const Generator& generator); 
std::string headerPathForInput(const std::string& name, const Generator& generator); 
std::string headerPathForOptions(const std::string& name, const Generator& generator); 
std::string headerPathForDispatch(const std::string& name, const Generator& generator); 
std::string commonHeaderPathFor(const Elem& elem, const Generator& generator);
std::string headerPathRoot(const std::string& name, const Generator& generator);
std::string pathForDoc(const std::string& name, const Generator& generator); 

std::string inputCodePathFor(const Elem& elem, const Generator& generator);
std::string inputCodePathForRoot(const std::string& name, const Generator& generator);
std::string inputCodePathForDoc(const std::string& name, const Generator& generator);
std::string inputCodePathForOptions(const std::string& name, const Generator& generator);
std::string inputCodePathForInput(const std::string& name, const Generator& generator);

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
bool isVersionOptionaField(const Elem& elem, const Generator& generator);

unsigned sinceVersionOf(const Elem& elem);

const std::string& dslEndianToOpt(commsdsl::parse::Endian value);

const std::string& dslUnitsToOpt(commsdsl::parse::Units value);

std::string messageIdStrFor(const commsdsl::gen::Message& msg, const Generator& generator);

std::size_t maxPossibleLength();

std::size_t addLength(std::size_t len1, std::size_t len2);

} // namespace comms

} // namespace gen

} // namespace commsdsl
