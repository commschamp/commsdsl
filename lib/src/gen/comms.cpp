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

#include "commsdsl/gen/GenEnumField.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenMessage.h"
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

const std::string GenScopeSep("::");
const std::string GenPathSep("/");
const std::size_t GenMaxPossibleLength = std::numeric_limits<std::size_t>::max();

std::string genScopeForElement(
    const std::string& name,
    const GenGenerator& generator,
    const std::vector<std::string>& subElems,
    bool addMainNamespace,
    bool addElement,
    const std::string& sep = GenScopeSep)
{
    std::string result;
    if (addMainNamespace) {
        result = generator.genCurrentSchema().genMainNamespace();
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

void genAddElemNamespaceScopeInternal(
    GenElem::GenType elemType,
    const GenElem* parent,
    const std::string& sep,
    std::string& str)
{
    if ((elemType == GenElem::GenType_Field) && (parent->genElemType() == GenElem::GenType_Namespace)) {
        // Global fields reside in appropriate namespace
        str.append(strings::genFieldNamespaceStr() + sep);
    }

    if (elemType == GenElem::GenType_Message) {
        assert(parent->genElemType() == GenElem::GenType_Namespace);
        str.append(strings::genMessageNamespaceStr() + sep);
    }

    if (elemType == GenElem::GenType_Frame) {
        assert(parent->genElemType() == GenElem::GenType_Namespace);
        str.append(strings::genFrameNamespaceStr() + sep);
    }
}

void genAddFieldScopeSuffixInternal(
    GenElem::GenType elemType,
    bool isLeaf,
    std::string& str)
{
    if ((elemType == GenElem::GenType_Message) ||
        (elemType == GenElem::GenType_Interface)) {
        str.append(strings::genFieldsSuffixStr());
        return;
    }

    if (elemType == GenElem::GenType_Layer) {
        str.append(strings::genMembersSuffixStr());
        return;
    }

    if (elemType == GenElem::GenType_Frame) {
        str.append(strings::genLayersSuffixStr());
        return;
    }

    if ((elemType == GenElem::GenType_Field) && (!isLeaf)) {
        str.append(strings::genMembersSuffixStr());
        return;
    }
}

void genAddNonFieldElemScopeSuffixInternal(
    GenElem::GenType elemType,
    GenElem::GenType leafElemType,
    std::string& str)
{
    if ((elemType == GenElem::GenType_Frame) && (leafElemType != elemType)) {
        str.append(strings::genLayersSuffixStr());
        return;
    }
}

void genAddNamespaceScopeInernal(
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

std::string genScopeForInternal(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement,
    const std::string& sep,
    const GenElem* leaf = nullptr)
{
    std::string result;
    if (leaf == nullptr) {
        leaf = &elem;
    }

    auto fieldTypeScope = (leaf->genElemType() == GenElem::GenType_Field) && (sep == GenScopeSep);

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    if (parent->genElemType() != GenElem::GenType_Schema) {
        result = genScopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
    }
    else if (addMainNamespace) {
        result = generator.genSchemaOf(elem).genMainNamespace();
    }

    do {
        auto elemType = elem.genElemType();
        if (!addElement) {
            std::string subNs;
            genAddElemNamespaceScopeInternal(elemType, parent, strings::genEmptyString(), subNs);
            if (subNs.empty()) {
                break;
            }

            if (!result.empty()) {
                result.append(sep);
            }

            result.append(subNs);
            break;
        }

        auto& elemName = elem.genName();

        if (elemType == GenElem::GenType_Namespace) {
            genAddNamespaceScopeInernal(elemName, sep, result);
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }

        auto name = genClassName(elemName);
        if ((name.empty()) && (elemType == GenElem::GenType_Interface)) {
            name = strings::genMessageClassStr();
        }

        genAddElemNamespaceScopeInternal(elemType, parent, sep, result);

        result.append(name);

        if (fieldTypeScope) {
            genAddFieldScopeSuffixInternal(elemType, &elem == leaf, result);
        }
        else {
            genAddNonFieldElemScopeSuffixInternal(elemType, leaf->genElemType(), result);
        }

    } while (false);

    return result;
}

std::string genCommonScopeForInternal(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement,
    const std::string& sep,
    const GenElem* leaf = nullptr)
{
    std::string result;

    if (leaf == nullptr) {
        leaf = &elem;
    }

    auto fieldTypeScope = (leaf->genElemType() == GenElem::GenType_Field) && (sep == GenScopeSep);

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    if (parent->genElemType() != GenElem::GenType_Schema) {
        result = genCommonScopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
    }
    else if (addMainNamespace) {
        result = generator.genSchemaOf(elem).genMainNamespace();
    }

    do {
        if (!addElement) {
            break;
        }

        auto elemType = elem.genElemType();
        auto& elemName = elem.genName();
        if (elemType == GenElem::GenType_Namespace) {
            genAddNamespaceScopeInernal(elemName, sep, result);
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }

        genAddElemNamespaceScopeInternal(elemType, parent, sep, result);

        result.append(genClassName(elem.genName()));

        if (fieldTypeScope) {
            genAddFieldScopeSuffixInternal(elemType, &elem == leaf, result);
        }

        if ((elemType == GenElem::GenType_Field) ||
            (elemType == GenElem::GenType_Message) ||
            (elemType == GenElem::GenType_Interface) ||
            (elemType == GenElem::GenType_Layer) ||
            (elemType == GenElem::GenType_Frame)) {
            result.append(strings::genCommonSuffixStr());
        }

    } while (false);

    return result;
}

} // namespace

std::string genClassName(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::toupper(static_cast<int>(result[0])));
    }

    return result;
}

std::string genAccessName(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::tolower(static_cast<int>(result[0])));
    }

    return result;
}

std::string genFullNameFor(const GenElem& elem)
{
    std::string result;
    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    if (parent->genElemType() != GenElem::GenType_Schema) {
        result = genFullNameFor(*parent);
    }

    if (!result.empty()) {
        result += '_';
    }

    auto elemType = elem.genElemType();
    do {
        if (elemType == GenElem::GenType_Namespace) {
            result.append(elem.genName());
            break;
        }

        result.append(genClassName(elem.genName()));
    } while (false);
    return result;
}

std::string genScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    return genScopeForInternal(elem, generator, addMainNamespace, addElement, GenScopeSep);
}

std::string genCommonScopeFor(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    return genCommonScopeForInternal(elem, generator, addMainNamespace, addElement, GenScopeSep);
}

std::string genScopeForOptions(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::genOptionsNamespaceStr()
    };

    return genScopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string genScopeForInput(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace,
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }
    subElems.push_back(strings::genInputNamespaceStr());

    return genScopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

std::string genScopeForFactory(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace,
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }
    subElems.push_back(strings::genFactoryNamespaceStr());

    return genScopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

std::string genScopeForDispatch(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace,
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }
    subElems.push_back(strings::genCispatchNamespaceStr());

    return genScopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

// TODO: remove
std::string genScopeForMsgId(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace,
    bool addElement)
{
    return genScopeForNamespaceMember(name, generator, ns, addMainNamespace, addElement);
}

std::string genScopeForNamespaceMember(
    const std::string& name,
    const GenGenerator& generator,
    const GenNamespace& ns,
    bool addMainNamespace,
    bool addElement)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    return genScopeForElement(name, generator, subElems, addMainNamespace, addElement);
}

std::string genScopeForRoot(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    static const std::vector<std::string> SubElems;
    return genScopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string genScopeForChecksum(
    const std::string& name,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::genFrameNamespaceStr(),
        strings::genChecksumNamespaceStr()
    };
    return genScopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string genScopeForCustomLayer(
    const GenElem& elem,
    const GenGenerator& generator,
    bool addMainNamespace,
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::genFrameNamespaceStr(),
        strings::genLayerNamespaceStr()
    };

    return genScopeForElement(elem.genName(), generator, SubElems, addMainNamespace, addElement, GenScopeSep);
}

std::string genRelHeaderPathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace)
{
    return genScopeForInternal(elem, generator, addMainNamespace, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelSourcePathFor(const GenElem& elem, const GenGenerator& generator, bool addMainNamespace)
{
    return genScopeForInternal(elem, generator, addMainNamespace, true, GenPathSep) + strings::genCppSourceSuffixStr();
}

std::string genRelCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator)
{
    return genCommonScopeForInternal(elem, generator, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderPathForField(const std::string& name, const GenGenerator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::genFieldNamespaceStr()
    };

    return genScopeForElement(name, generator, SubElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForOptions(const std::string& name, const GenGenerator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems = {
        strings::genOptionsNamespaceStr()
    };

    return genScopeForElement(name, generator, SubElems, addMainNamespace, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    subElems.push_back(strings::genCispatchNamespaceStr());
    return genScopeForElement(name, generator, subElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    subElems.push_back(strings::genFactoryNamespaceStr());
    return genScopeForElement(name, generator, subElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

// TODO: remove
std::string genRelHeaderForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return genRelHeaderForNamespaceMember(name, generator, ns);
}

std::string genRelHeaderForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    return genScopeForElement(name, generator, subElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelSourceForNamespaceMember(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    return genScopeForElement(name, generator, subElems, true, true, GenPathSep) + strings::genCppSourceSuffixStr();
}

std::string genRelHeaderForLayer(const std::string& name, const GenGenerator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::genFrameNamespaceStr(),
        strings::genLayerNamespaceStr()
    };

    return genScopeForElement(name, generator, SubElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForChecksum(const std::string& name, const GenGenerator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::genFrameNamespaceStr(),
        strings::genChecksumNamespaceStr()
    };

    return genScopeForElement(name, generator, SubElems, true, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns, bool addMainNamespace)
{
    std::vector<std::string> subElems;
    if (!ns.genName().empty()) {
        subElems.push_back(ns.genName());
    }

    subElems.push_back(strings::genInputNamespaceStr());

    return genScopeForElement(name, generator, subElems, addMainNamespace, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelHeaderForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems;
    return genScopeForElement(name, generator, SubElems, addMainNamespace, true, GenPathSep) + strings::genCppHeaderSuffixStr();
}

std::string genRelSourceForRoot(const std::string& name, const GenGenerator& generator, bool addMainNamespace)
{
    static const std::vector<std::string> SubElems;
    return genScopeForElement(name, generator, SubElems, addMainNamespace, true, GenPathSep) + strings::genCppSourceSuffixStr();
}

std::string genHeaderPathFor(const GenElem& elem, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderPathFor(elem, generator);
}

std::string genSourcePathFor(const GenElem& elem, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genSrcDirStr() + '/' + genRelSourcePathFor(elem, generator);
}

std::string genHeaderPathForField(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderPathForField(name, generator);
}

std::string genHeaderPathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForInput(name, generator, ns);
}

std::string genHeaderPathForOptions(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForOptions(name, generator);
}

std::string genHeaderPathForDispatch(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForDispatch(name, generator, ns);
}

std::string genHeaderPathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForFactory(name, generator, ns);
}

std::string genHeaderPathForMsgId(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForMsgId(name, generator, ns);
}

std::string genCommonHeaderPathFor(const GenElem& elem, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelCommonHeaderPathFor(elem, generator);
}

std::string genHeaderPathRoot(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genIncludeDirStr() + '/' + genRelHeaderForRoot(name, generator);
}

std::string genSourcePathRoot(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genSrcDirStr() + '/' + genRelSourceForRoot(name, generator);
}

std::string genPathForDoc(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetOutputDir() + '/' + strings::genDocDirStr() + '/' + name;
}

std::string genInputCodePathFor(const GenElem& elem, const GenGenerator& generator)
{
    return
        generator.genGetCodeDir() + '/' + strings::genIncludeDirStr() + '/' +
        generator.genSchemaOf(elem).genOrigNamespace() + '/' +
        comms::genRelHeaderPathFor(elem, generator, false);
}

std::string genInputCodePathForRoot(const std::string& name, const GenGenerator& generator)
{
    return
        generator.genGetCodeDir() + '/' + strings::genIncludeDirStr() + '/' +
        generator.genCurrentSchema().genOrigNamespace() + '/' +
        comms::genRelHeaderForRoot(name, generator, false);
}

std::string genInputCodePathForDoc(const std::string& name, const GenGenerator& generator)
{
    return generator.genGetCodeDir() + '/' + strings::genDocDirStr() + '/' + name;
}

std::string genInputCodePathForOptions(const std::string& name, const GenGenerator& generator)
{
    return
        generator.genGetCodeDir() + '/' + strings::genIncludeDirStr() + '/' +
        generator.genCurrentSchema().genOrigNamespace() + '/' +
        comms::genRelHeaderForOptions(name, generator, false);
}

std::string genInputCodePathForInput(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return
        generator.genGetCodeDir() + '/' + strings::genIncludeDirStr() + '/' +
        generator.genCurrentSchema().genOrigNamespace() + '/' +
        comms::genRelHeaderForInput(name, generator, ns, false);
}

std::string genInputCodePathForFactory(const std::string& name, const GenGenerator& generator, const GenNamespace& ns)
{
    return
        generator.genGetCodeDir() + '/' + strings::genIncludeDirStr() + '/' +
        generator.genCurrentSchema().genOrigNamespace() + '/' +
        comms::genRelHeaderForFactory(name, generator, ns);
}

std::string genNamespaceBeginFor(
    const GenElem& elem,
    const GenGenerator& generator)
{
    std::string result;

    auto appendToResultFunc =
        [&result](const std::string& str)
        {
            result += "namespace " + str + "\n{\n\n";
        };

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    do {
        if (parent->genElemType() != GenElem::GenType_Schema) {
            result += genNamespaceBeginFor(*parent, generator);
            break;
        }

        auto& topNamespace = generator.genGetTopNamespace();
        if (!topNamespace.empty()) {
            appendToResultFunc(topNamespace);
        }

        appendToResultFunc(generator.genSchemaOf(elem).genMainNamespace());
    } while (false);

    auto elemType = elem.genElemType();
    if ((elemType == GenElem::GenType_Field) && (parent->genElemType() == GenElem::GenType_Namespace)) {
        appendToResultFunc(strings::genFieldNamespaceStr());
    }

    if (elemType == GenElem::GenType_Message) {
        assert(parent->genElemType() == GenElem::GenType_Namespace);
        appendToResultFunc(strings::genMessageNamespaceStr());
    }

    if (elemType == GenElem::GenType_Frame) {
        assert(parent->genElemType() == GenElem::GenType_Namespace);
        appendToResultFunc(strings::genFrameNamespaceStr());
    }

    if (elemType == GenElem::GenType_Layer) {
        assert(parent->genElemType() == GenElem::GenType_Frame);
        appendToResultFunc(strings::genLayerNamespaceStr());
    }

    if (elem.genElemType() != GenElem::GenType_Namespace) {
        return result;
    }

    auto& elemName = elem.genName();
    if (elemName.empty()) {
        return result;
    }

    appendToResultFunc(elemName);
    return result;
}

std::string genNamespaceEndFor(
    const GenElem& elem,
    const GenGenerator& generator)
{
    std::string result;

    auto appendToResultFunc =
        [&result](const std::string& str)
        {
            result += "} // namespace " + str + "\n\n";
        };

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);

    auto elemType = elem.genElemType();
    if (elemType != GenElem::GenType_Namespace) {
        assert(parent != nullptr);
        if ((elemType == GenElem::GenType_Field) && (parent->genElemType() == GenElem::GenType_Namespace)) {
            appendToResultFunc(strings::genFieldNamespaceStr());
        }

        if (elemType == GenElem::GenType_Message) {
            assert(parent->genElemType() == GenElem::GenType_Namespace);
            appendToResultFunc(strings::genMessageNamespaceStr());
        }

        if (elemType == GenElem::GenType_Frame) {
            assert(parent->genElemType() == GenElem::GenType_Namespace);
            appendToResultFunc(strings::genFrameNamespaceStr());
        }

        if (elemType == GenElem::GenType_Layer) {
            assert(parent->genElemType() == GenElem::GenType_Frame);
            appendToResultFunc(strings::genLayerNamespaceStr());
        }

        result += genNamespaceEndFor(*parent, generator);

        return result;
    }

    auto& elemName = elem.genName();

    if (!elemName.empty()) {
        appendToResultFunc(elemName);
    }

    do {
        if (parent->genElemType() != GenElem::GenType_Schema) {
            result += genNamespaceEndFor(*parent, generator);
            break;
        }

        appendToResultFunc(generator.genSchemaOf(elem).genMainNamespace());

        auto& topNamespace = generator.genGetTopNamespace();
        if (!topNamespace.empty()) {
            appendToResultFunc(topNamespace);
        }
    } while (false);

    return result;
}

void genPrepareIncludeStatement(std::vector<std::string>& includes)
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

const std::string& genCppIntTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len)
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
        /* Intvar */ strings::genEmptyString(),
        /* Uintvar */ strings::genEmptyString()
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::ParseIntField::ParseType::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        assert(false); // Should not happen
        return strings::genEmptyString();
    }

    auto& typeStr = TypeMap[idx];
    if (!typeStr.empty()) {
        return typeStr;
    }

    // Variable length
    auto offset = idx - static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Intvar);
    assert(offset < 2U);

    if (len <= 2U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int16);
        return TypeMap[base + offset];
    }

    if (len <= 4U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int32);
        return TypeMap[base + offset];
    }

    auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int64);
    return TypeMap[base + offset];
}

std::string genCppIntChangedSignTypeFor(commsdsl::parse::ParseIntField::ParseType value, std::size_t len)
{
    auto str = genCppIntTypeFor(value, len);
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

const std::string& genCppFloatTypeFor(commsdsl::parse::ParseFloatField::ParseType value)
{
    static const std::string TypeMap[] = {
        /* Float */ "float",
        /* Double */ "double",
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::ParseFloatField::ParseType::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    return TypeMap[idx];
}

bool genIsGlobalField(const GenElem& elem)
{
    if (elem.genElemType() != GenElem::GenType_Field) {
        return false;
    }

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    return parent->genElemType() == GenElem::GenType_Namespace;
}

bool genIsInterfaceDeepMemberField(const GenElem& elem)
{
    if (elem.genElemType() != GenElem::GenType_Field) {
        return false;
    }

    auto* parent = elem.genGetParent();
    while (parent != nullptr) {
        if (parent->genElemType() == GenElem::GenType_Interface) {
            return true;
        }

        parent = parent->genGetParent();
    }
    return false;
}

bool genIsInterfaceShallowMemberField(const GenElem& elem)
{
    if (elem.genElemType() != GenElem::GenType_Field) {
        return false;
    }

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    return parent->genElemType() == GenElem::GenType_Interface;
}

bool genIsMessageShallowMemberField(const GenElem& elem)
{
    if (elem.genElemType() != GenElem::GenType_Field) {
        return false;
    }

    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    return parent->genElemType() == GenElem::GenType_Message;
}

bool genIsVersionOptionalField(const GenElem& elem, const GenGenerator& generator)
{
    if (elem.genElemType() != GenElem::GenType_Field) {
        assert(false); // Should not happen
        return false;
    }

    if (!generator.genSchemaOf(elem).genVersionDependentCode()) {
        return false;
    }

    auto& field = static_cast<const gen::GenField&>(elem);
    auto& dslObj = field.genParseObj();
    if (!generator.genIsElementOptional(dslObj.parseSinceVersion(), dslObj.parseDeprecatedSince(), dslObj.parseIsDeprecatedRemoved())) {
        return false;
    }

    auto* parent = field.genGetParent();
    assert(parent != nullptr);
    if (comms::genSinceVersionOf(*parent) < dslObj.parseSinceVersion()) {
        return true;
    }

    if ((dslObj.parseDeprecatedSince() < commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) &&
        (dslObj.parseIsDeprecatedRemoved())) {
        return true;
    }

    return false;
}

unsigned genSinceVersionOf(const GenElem& elem)
{
    auto elemType = elem.genElemType();
    if (elemType == GenElem::GenType_Message) {
        return static_cast<const gen::GenMessage&>(elem).genParseObj().parseSinceVersion();
    }

    if (elemType == GenElem::GenType_Field) {
        auto* parent = elem.genGetParent();
        assert(parent != nullptr);
        auto fieldResult = static_cast<const gen::GenField&>(elem).genParseObj().parseSinceVersion();
        return std::max(genSinceVersionOf(*parent), fieldResult);
    }

    return 0U;
}

const std::string& genParseEndianToOpt(commsdsl::parse::ParseEndian value)
{
    static const std::string Map[] = {
        "comms::option::def::LittleEndian",
        "comms::option::def::BigEndian"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::ParseEndian_NumOfValues),
        "Invalid map");

    if (commsdsl::parse::ParseEndian_NumOfValues <= value) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = commsdsl::parse::ParseEndian_Little;
    }

    return Map[value];
}

const std::string& genParseUnitsToOpt(commsdsl::parse::ParseUnits value)
{
    if (commsdsl::parse::ParseUnits::NumOfValues <= value) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    static const std::string UnitsMap[] = {
        /* Unknown */ strings::genEmptyString(),
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
    static_assert(static_cast<std::size_t>(commsdsl::parse::ParseUnits::NumOfValues) == UnitsMapSize,
        "Invalid Map");

    auto idx = static_cast<unsigned>(value);
    return UnitsMap[idx];
}

std::size_t genMaxPossibleLength()
{
    return GenMaxPossibleLength;
}

std::size_t genAddLength(std::size_t len1, std::size_t len2)
{
    if ((GenMaxPossibleLength - len1) <= len2) {
        return GenMaxPossibleLength;
    }

    return len1 + len2;
}

} // namespace comms

} // namespace gen

} // namespace commsdsl
