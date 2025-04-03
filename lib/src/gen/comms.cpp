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

#include "commsdsl/gen/comms.h"

#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <limits>
#include <vector>


namespace commsdsl
{

namespace gen
{

namespace comms
{

namespace 
{

const std::string ScopeSep("::");
const std::string PathSep("/");
const std::size_t MaxPossibleLength = std::numeric_limits<std::size_t>::max();

std::string scopeForElement(
    const std::string& name, 
    const Generator& generator, 
    const std::vector<std::string>& subElems,
    bool addMainNamespace, 
    bool addElement,
    const std::string& sep = ScopeSep)
{
    std::string result;
    if (addMainNamespace) {
        result = generator.currentSchema().mainNamespace();
    }

    for (auto& elem : subElems) {
        if (!result.empty()) {
            result.append(sep);
        }

        result.append(elem);
    }

    if (addElement) {
        if (!result.empty()) {
            result.append(sep);
        }        
        result.append(name);
    }

    return result;
}

void addElemNamespaceScopeInternal(
    Elem::Type elemType,
    const Elem* parent,
    const std::string& sep,
    std::string& str)
{
    if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
        // Global fields reside in appropriate namespace
        str.append(strings::fieldNamespaceStr() + sep);
    }

    if (elemType == Elem::Type_Message) {
        assert(parent->elemType() == Elem::Type_Namespace);
        str.append(strings::messageNamespaceStr() + sep);
    }     

    if (elemType == Elem::Type_Frame) {
        assert(parent->elemType() == Elem::Type_Namespace);
        str.append(strings::frameNamespaceStr() + sep);
    }  
}

void addFieldScopeSuffixInternal(
    Elem::Type elemType, 
    bool isLeaf, 
    std::string& str)
{
    if ((elemType == Elem::Type_Message) ||
        (elemType == Elem::Type_Interface)) {
        str.append(strings::fieldsSuffixStr());
        return;
    }      

    if (elemType == Elem::Type_Layer) {
        str.append(strings::membersSuffixStr());
        return;
    }  

    if (elemType == Elem::Type_Frame) {
        str.append(strings::layersSuffixStr());
        return;
    }   

    if ((elemType == Elem::Type_Field) && (!isLeaf)) {
        str.append(strings::membersSuffixStr());
        return;
    }  
}

void addNonFieldElemScopeSuffixInternal(
    Elem::Type elemType, 
    Elem::Type leafElemType,
    std::string& str)
{
    if ((elemType == Elem::Type_Frame) && (leafElemType != elemType)) {
        str.append(strings::layersSuffixStr());
        return;
    }  
}

void addNamespaceScopeInernal(
    const std::string& elemName,
    const std::string& sep,
    std::string& str)
{
    if (elemName.empty()) {
        return;
    }

    if (!str.empty()) {
        str.append(sep);
    }               

    str.append(elemName);
}

std::string scopeForInternal(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement,
    const std::string& sep,
    const Elem* leaf = nullptr)
{
    std::string result;
    if (leaf == nullptr) {
        leaf = &elem;
    }

    auto fieldTypeScope = (leaf->elemType() == Elem::Type_Field) && (sep == ScopeSep);

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    if (parent->elemType() != Elem::Type_Schema) {
        result = scopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
    }
    else if (addMainNamespace) {
        result = generator.schemaOf(elem).mainNamespace();
    }

    do {
        auto elemType = elem.elemType();
        if (!addElement) {
            std::string subNs;
            addElemNamespaceScopeInternal(elemType, parent, strings::emptyString(), subNs);
            if (subNs.empty()) {
                break;
            }

            if (!result.empty()) {
                result.append(sep);
            }    

            result.append(subNs);
            break;
        }
        
        auto& elemName = elem.name();

        if (elemType == Elem::Type_Namespace) {
            addNamespaceScopeInernal(elemName, sep, result);
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }            

        auto name = className(elemName);
        if ((name.empty()) && (elemType == Elem::Type_Interface)) {
            name = strings::messageClassStr();
        }    

        addElemNamespaceScopeInternal(elemType, parent, sep, result);    

        result.append(name);

        if (fieldTypeScope) {
            addFieldScopeSuffixInternal(elemType, &elem == leaf, result);
        } 
        else {
            addNonFieldElemScopeSuffixInternal(elemType, leaf->elemType(), result);
        }       

    } while (false);

    return result;
}

std::string commonScopeForInternal(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement,
    const std::string& sep,
    const Elem* leaf = nullptr)
{
    std::string result;

    if (leaf == nullptr) {
        leaf = &elem;
    }

    auto fieldTypeScope = (leaf->elemType() == Elem::Type_Field) && (sep == ScopeSep);

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    if (parent->elemType() != Elem::Type_Schema) {
        result = commonScopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
    }
    else if (addMainNamespace) {
        result = generator.schemaOf(elem).mainNamespace();
    }

    do {
        if (!addElement) {
            break;
        }

        auto elemType = elem.elemType();
        auto& elemName = elem.name();
        if (elemType == Elem::Type_Namespace) {
            addNamespaceScopeInernal(elemName, sep, result);
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }

        addElemNamespaceScopeInternal(elemType, parent, sep, result);    

        result.append(className(elem.name()));

        if (fieldTypeScope) {
            addFieldScopeSuffixInternal(elemType, &elem == leaf, result);
        }

        if ((elemType == Elem::Type_Field) || 
            (elemType == Elem::Type_Message) || 
            (elemType == Elem::Type_Interface) ||
            (elemType == Elem::Type_Layer) ||
            (elemType == Elem::Type_Frame)) {
            result.append(strings::commonSuffixStr());
        }

    } while (false);

    return result;
}

} // namespace 
    

std::string className(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::toupper(static_cast<int>(result[0])));
    }

    return result;
}

std::string accessName(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::tolower(static_cast<int>(result[0])));
    }

    return result;
}

std::string fullNameFor(const Elem& elem)
{
    std::string result;
    auto* parent = elem.getParent();
    assert(parent != nullptr);
    if (parent->elemType() != Elem::Type_Schema) {
        result = fullNameFor(*parent);
    }

    if (!result.empty()) {
        result += '_';
    }

    auto elemType = elem.elemType();
    do {
        if (elemType == Elem::Type_Namespace) {
            result.append(elem.name());
            break;
        }

        result.append(className(elem.name()));
    } while (false);
    return result;
}

std::string scopeFor(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    return scopeForInternal(elem, generator, addMainNamespace, addElement, ScopeSep);
}

std::string commonScopeFor(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    return commonScopeForInternal(elem, generator, addMainNamespace, addElement, ScopeSep);
}

std::string scopeForOptions(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string scopeForInput(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::inputNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string scopeForFactory(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace, 
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.name().empty()) {
        subElems.push_back(ns.name());
    }
    subElems.push_back(strings::factoryNamespaceStr());

    return scopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

std::string scopeForDispatch(
    const std::string& name, 
    const Generator& generator, 
    const Namespace& ns,
    bool addMainNamespace, 
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.name().empty()) {
        subElems.push_back(ns.name());
    }
    subElems.push_back(strings::dispatchNamespaceStr());
    
    return scopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

std::string scopeForRoot(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);    
}

std::string scopeForChecksum(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::frameNamespaceStr(),
        strings::checksumNamespaceStr()
    };
    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);    
}

std::string scopeForCustomLayer(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::frameNamespaceStr(),
        strings::layerNamespaceStr()
    };

    return scopeForElement(elem.name(), generator, SubElems, addMainNamespace, addElement, ScopeSep); 
}

std::string relHeaderPathFor(const Elem& elem, const Generator& generator, bool addMainNamespace)
{
    return scopeForInternal(elem, generator, addMainNamespace, true, PathSep) + strings::cppHeaderSuffixStr();    
}

std::string relSourcePathFor(const Elem& elem, const Generator& generator, bool addMainNamespace)
{
    return scopeForInternal(elem, generator, addMainNamespace, true, PathSep) + strings::cppSourceSuffixStr();    
}

std::string relCommonHeaderPathFor(const Elem& elem, const Generator& generator)
{
    return commonScopeForInternal(elem, generator, true, true, PathSep) + strings::cppHeaderSuffixStr();    
}

std::string relHeaderPathForField(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::fieldNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForOptions(const std::string& name, const Generator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForDispatch(const std::string& name, const Generator& generator, const Namespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.name().empty()) {
        subElems.push_back(ns.name());
    }

    subElems.push_back(strings::dispatchNamespaceStr());
    return scopeForElement(name, generator, subElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForFactory(const std::string& name, const Generator& generator, const Namespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.name().empty()) {
        subElems.push_back(ns.name());
    }

    subElems.push_back(strings::factoryNamespaceStr());
    return scopeForElement(name, generator, subElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForLayer(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::frameNamespaceStr(),
        strings::layerNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForChecksum(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::frameNamespaceStr(),
        strings::checksumNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForInput(const std::string& name, const Generator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems = {
        strings::inputNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForRoot(const std::string& name, const Generator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relSourceForRoot(const std::string& name, const Generator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, true, PathSep) + strings::cppSourceSuffixStr();
}

std::string headerPathFor(const Elem& elem, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderPathFor(elem, generator);
}

std::string sourcePathFor(const Elem& elem, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::srcDirStr() + '/' + relSourcePathFor(elem, generator);
}

std::string headerPathForField(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderPathForField(name, generator);
}

std::string headerPathForInput(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForInput(name, generator);
}

std::string headerPathForOptions(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForOptions(name, generator);
}

std::string headerPathForDispatch(const std::string& name, const Generator& generator, const Namespace& ns)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForDispatch(name, generator, ns);
}

std::string headerPathForFactory(const std::string& name, const Generator& generator, const Namespace& ns)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForFactory(name, generator, ns);
}

std::string commonHeaderPathFor(const Elem& elem, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relCommonHeaderPathFor(elem, generator);
}

std::string headerPathRoot(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForRoot(name, generator);
}

std::string sourcePathRoot(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::srcDirStr() + '/' + relSourceForRoot(name, generator);
}

std::string pathForDoc(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::docDirStr() + '/' + name;    
}

std::string inputCodePathFor(const Elem& elem, const Generator& generator)
{
    return 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        generator.schemaOf(elem).origNamespace() + '/' +
        comms::relHeaderPathFor(elem, generator, false);
}

std::string inputCodePathForRoot(const std::string& name, const Generator& generator)
{
    return 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        generator.currentSchema().origNamespace() + '/' +
        comms::relHeaderForRoot(name, generator, false);
}

std::string inputCodePathForDoc(const std::string& name, const Generator& generator)
{
    return generator.getCodeDir() + '/' + strings::docDirStr() + '/' + name;
}

std::string inputCodePathForOptions(const std::string& name, const Generator& generator)
{
    return 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        generator.currentSchema().origNamespace() + '/' +
        comms::relHeaderForOptions(name, generator, false);
}

std::string inputCodePathForInput(const std::string& name, const Generator& generator)
{
    return 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        generator.currentSchema().origNamespace() + '/' +
        comms::relHeaderForInput(name, generator, false);
}

std::string inputCodePathForFactory(const std::string& name, const Generator& generator, const Namespace& ns)
{
    return 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        generator.currentSchema().origNamespace() + '/' +
        comms::relHeaderForFactory(name, generator, ns);
}

std::string namespaceBeginFor(
    const Elem& elem, 
    const Generator& generator)
{
    std::string result;

    auto appendToResultFunc = 
        [&result](const std::string& str)
        {
            result += "namespace " + str + "\n{\n\n";
        };

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    do {
        if (parent->elemType() != Elem::Type_Schema) {
            result += namespaceBeginFor(*parent, generator);
            break;
        }

        auto& topNamespace = generator.getTopNamespace();
        if (!topNamespace.empty()) {
            appendToResultFunc(topNamespace);
        }

        appendToResultFunc(generator.schemaOf(elem).mainNamespace());
    } while (false);

    auto elemType = elem.elemType();
    if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
        appendToResultFunc(strings::fieldNamespaceStr());
    }   

    if (elemType == Elem::Type_Message) {
        assert(parent->elemType() == Elem::Type_Namespace);
        appendToResultFunc(strings::messageNamespaceStr());
    }

    if (elemType == Elem::Type_Frame) {
        assert(parent->elemType() == Elem::Type_Namespace);
        appendToResultFunc(strings::frameNamespaceStr());
    }

    if (elemType == Elem::Type_Layer) {
        assert(parent->elemType() == Elem::Type_Frame);
        appendToResultFunc(strings::layerNamespaceStr());
    }        

    if (elem.elemType() != Elem::Type_Namespace) {
        return result;
    }

    auto& elemName = elem.name();
    if (elemName.empty()) {
        return result;
    }

    appendToResultFunc(elemName);
    return result;
}       

std::string namespaceEndFor(
    const Elem& elem, 
    const Generator& generator)
{
    std::string result;

    auto appendToResultFunc = 
        [&result](const std::string& str)
        {
            result += "} // namespace " + str + "\n\n";
        };

    auto* parent = elem.getParent();
    assert(parent != nullptr);

    auto elemType = elem.elemType();
    if (elemType != Elem::Type_Namespace) {
        assert(parent != nullptr);
        if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
            appendToResultFunc(strings::fieldNamespaceStr());
        }

        if (elemType == Elem::Type_Message) {
            assert(parent->elemType() == Elem::Type_Namespace);
            appendToResultFunc(strings::messageNamespaceStr());
        }

        if (elemType == Elem::Type_Frame) {
            assert(parent->elemType() == Elem::Type_Namespace);
            appendToResultFunc(strings::frameNamespaceStr());
        }

        if (elemType == Elem::Type_Layer) {
            assert(parent->elemType() == Elem::Type_Frame);
            appendToResultFunc(strings::layerNamespaceStr());
        }        

        result += namespaceEndFor(*parent, generator);

        return result;
    }

    auto& elemName = elem.name();
    
    if (!elemName.empty()) {
        appendToResultFunc(elemName);
    }
    
    do {
        if (parent->elemType() != Elem::Type_Schema) {
            result += namespaceEndFor(*parent, generator);
            break;
        }

        appendToResultFunc(generator.schemaOf(elem).mainNamespace());

        auto& topNamespace = generator.getTopNamespace();
        if (!topNamespace.empty()) {
            appendToResultFunc(topNamespace);
        }
    } while (false);

    return result;
}

void prepareIncludeStatement(std::vector<std::string>& includes)
{
    std::sort(includes.begin(), includes.end());
    includes.erase(
        std::unique(includes.begin(), includes.end()),
        includes.end()
    );

    for (auto& i : includes) {
        if (i.empty()) {
            continue;
        }

        if (i.front() == '#') {
            continue;
        }

        static const std::string IncludePrefix("#include ");
        if (i.front() == '<') {
            i = IncludePrefix + i;
            continue;
        }

        i = IncludePrefix + '\"' + i + '\"';
    }
}

const std::string& cppIntTypeFor(commsdsl::parse::IntField::Type value, std::size_t len)
{
    static const std::string TypeMap[] = {
        /* Int8 */ "std::int8_t",
        /* Uint8 */ "std::uint8_t",
        /* Int16 */ "std::int16_t",
        /* Uint16 */ "std::uint16_t",
        /* Int32 */ "std::int32_t",
        /* Uint32 */ "std::uint32_t",
        /* Int64 */ "std::int64_t",
        /* Uint64 */ "std::uint64_t",
        /* Intvar */ strings::emptyString(),
        /* Uintvar */ strings::emptyString()
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        assert(false); // Should not happen
        return strings::emptyString();
    }

    auto& typeStr = TypeMap[idx];
    if (!typeStr.empty()) {
        return typeStr;
    }

    // Variable length
    auto offset = idx - static_cast<decltype(idx)>(commsdsl::parse::IntField::Type::Intvar);
    assert(offset < 2U);

    if (len <= 2U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::IntField::Type::Int16);
        return TypeMap[base + offset];
    }

    if (len <= 4U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::IntField::Type::Int32);
        return TypeMap[base + offset];
    }

    auto base = static_cast<decltype(idx)>(commsdsl::parse::IntField::Type::Int64);
    return TypeMap[base + offset];    
}

std::string cppIntChangedSignTypeFor(commsdsl::parse::IntField::Type value, std::size_t len)
{
    auto str = cppIntTypeFor(value, len);
    assert(str.find("std::") == 0U);
    if (str.size() < 6) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return str;
    }

    if (str[5] == 'u') {
        str.erase(str.begin() + 5);
    }
    else {
        str.insert(str.begin() + 5, 'u');
    }

    return str;    
}

const std::string& cppFloatTypeFor(commsdsl::parse::FloatField::Type value)
{
    static const std::string TypeMap[] = {
        /* Float */ "float",
        /* Double */ "double",
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::FloatField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::emptyString();
    }

    return TypeMap[idx];    
}

bool isGlobalField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    return parent->elemType() == Elem::Type_Namespace;
}

bool isInterfaceDeepMemberField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    while (parent != nullptr) {
        if (parent->elemType() == Elem::Type_Interface) {
            return true;
        }

        parent = parent->getParent();
    }
    return false;
}

bool isInterfaceShallowMemberField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    return parent->elemType() == Elem::Type_Interface;
}

bool isMessageShallowMemberField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    return parent->elemType() == Elem::Type_Message;
}

bool isVersionOptionalField(const Elem& elem, const Generator& generator)
{
    if (elem.elemType() != Elem::Type_Field) {
        assert(false); // Should not happen
        return false;
    }    

    if (!generator.schemaOf(elem).versionDependentCode()) {
        return false;
    }

    auto& field = static_cast<const gen::Field&>(elem);
    auto& dslObj = field.dslObj();
    if (!generator.isElementOptional(dslObj.sinceVersion(), dslObj.deprecatedSince(), dslObj.isDeprecatedRemoved())) {
        return false;
    }

    auto* parent = field.getParent();
    assert(parent != nullptr);
    if (comms::sinceVersionOf(*parent) < dslObj.sinceVersion()) {
        return true;
    }

    if ((dslObj.deprecatedSince() < commsdsl::parse::Protocol::notYetDeprecated()) &&
        (dslObj.isDeprecatedRemoved())) {
        return true;
    }

    return false;     
}

unsigned sinceVersionOf(const Elem& elem)
{
    auto elemType = elem.elemType();
    if (elemType == Elem::Type_Message) {
        return static_cast<const gen::Message&>(elem).dslObj().sinceVersion();
    }

    if (elemType == Elem::Type_Field) {
        auto* parent = elem.getParent();
        assert(parent != nullptr);
        auto fieldResult = static_cast<const gen::Field&>(elem).dslObj().sinceVersion();
        return std::max(sinceVersionOf(*parent), fieldResult);
    }

    return 0U;
}

const std::string& dslEndianToOpt(commsdsl::parse::Endian value)
{
    static const std::string Map[] = {
        "comms::option::def::LittleEndian",
        "comms::option::def::BigEndian"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::Endian_NumOfValues),
        "Invalid map");

    if (commsdsl::parse::Endian_NumOfValues <= value) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = commsdsl::parse::Endian_Little;
    }

    return Map[value];
}

const std::string& dslUnitsToOpt(commsdsl::parse::Units value)
{
    if (commsdsl::parse::Units::NumOfValues <= value) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::emptyString();
    }

    static const std::string UnitsMap[] = {
        /* Unknown */ strings::emptyString(),
        /* Nanoseconds */ "comms::option::def::UnitsNanoseconds",
        /* Microseconds */ "comms::option::def::UnitsMicroseconds",
        /* Milliseconds */ "comms::option::def::UnitsMilliseconds",
        /* Seconds */ "comms::option::def::UnitsSeconds",
        /* Minutes */ "comms::option::def::UnitsMinutes",
        /* Hours */ "comms::option::def::UnitsHours",
        /* Days */ "comms::option::def::UnitsDays",
        /* Weeks */ "comms::option::def::UnitsWeeks",
        /* Nanometers */ "comms::option::def::UnitsNanometers",
        /* Micrometers */ "comms::option::def::UnitsMicrometers",
        /* Millimeters */ "comms::option::def::UnitsMillimeters",
        /* Centimeters */ "comms::option::def::UnitsCentimeters",
        /* Meters */ "comms::option::def::UnitsMeters",
        /* Kilometers */ "comms::option::def::UnitsKilometers",
        /* NanometersPerSecond */ "comms::option::def::UnitsNanometersPerSecond",
        /* MicrometersPerSecond */ "comms::option::def::UnitsMicrometersPerSecond",
        /* MillimetersPerSecond */ "comms::option::def::UnitsMillimetersPerSecond",
        /* CentimetersPerSecond */ "comms::option::def::UnitsCentimetersPerSecond",
        /* MetersPerSecond */ "comms::option::def::UnitsMetersPerSecond",
        /* KilometersPerSecond */ "comms::option::def::UnitsKilometersPerSecond",
        /* KilometersPerHour */ "comms::option::def::UnitsKilometersPerHour",
        /* Hertz */ "comms::option::def::UnitsHertz",
        /* KiloHertz */ "comms::option::def::UnitsKilohertz",
        /* MegaHertz */ "comms::option::def::UnitsMegahertz",
        /* GigaHertz */ "comms::option::def::UnitsGigahertz",
        /* Degrees */ "comms::option::def::UnitsDegrees",
        /* Radians */ "comms::option::def::UnitsRadians",
        /* Nanoamps */ "comms::option::def::UnitsNanoamps",
        /* Microamps */ "comms::option::def::UnitsMicroamps",
        /* Milliamps */ "comms::option::def::UnitsMilliamps",
        /* Amps */ "comms::option::def::UnitsAmps",
        /* Kiloamps */ "comms::option::def::UnitsKiloamps",
        /* Nanovolts */ "comms::option::def::UnitsNanovolts",
        /* Microvolts */ "comms::option::def::UnitsMicrovolts",
        /* Millivolts */ "comms::option::def::UnitsMillivolts",
        /* Volts */ "comms::option::def::UnitsVolts",
        /* Kilovolts */ "comms::option::def::UnitsKilovolts",
        /* Bytes */ "comms::option::def::UnitsBytes",
        /* Kilobytes */ "comms::option::def::UnitsKilobytes",
        /* Megabytes */ "comms::option::def::UnitsMegabytes",
        /* Gigabytes */ "comms::option::def::UnitsGigabytes",
        /* Terabytes */ "comms::option::def::UnitsTerabytes",
    };

    static const std::size_t UnitsMapSize = std::extent<decltype(UnitsMap)>::value;
    static_assert(static_cast<std::size_t>(commsdsl::parse::Units::NumOfValues) == UnitsMapSize,
        "Invalid Map");

    auto idx = static_cast<unsigned>(value);
    return UnitsMap[idx];
}

std::string messageIdStrFor(const commsdsl::gen::Message& msg, const Generator& generator)
{
    auto msgIdField = generator.currentSchema().getMessageIdField();
    if (msgIdField == nullptr) {
        return generator.currentSchema().mainNamespace() + "::" + strings::msgIdPrefixStr() + comms::fullNameFor(msg);
    }

    assert(msgIdField->dslObj().kind() == commsdsl::parse::Field::Kind::Enum);
    auto* castedEnumField = static_cast<const commsdsl::gen::EnumField*>(msgIdField);

    std::string name;
    auto obj = castedEnumField->enumDslObj();

    auto& revValues = obj.revValues();
    auto id = static_cast<std::intmax_t>(msg.dslObj().id());
    auto iter = revValues.find(id);
    if (iter != revValues.end()) {
        name = iter->second;
    }

    if (!name.empty()) {
        return generator.currentSchema().mainNamespace() + "::" + strings::msgIdPrefixStr() + name;
    }

    return util::numToString(id);    
}

std::size_t maxPossibleLength()
{
    return MaxPossibleLength;
}

std::size_t addLength(std::size_t len1, std::size_t len2)
{
    if ((MaxPossibleLength - len1) <= len2) {
        return MaxPossibleLength;
    }

    return len1 + len2;
}


} // namespace comms

} // namespace gen

} // namespace commsdsl
