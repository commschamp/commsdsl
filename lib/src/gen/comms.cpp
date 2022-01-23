#include "commsdsl/gen/comms.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Field.h"

#include <algorithm>
#include <cassert>
#include <cctype>
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
        result = generator.mainNamespace();
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

std::string scopeForInternal(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement,
    const std::string& sep)
{
    std::string result;
    auto* parent = elem.getParent();
    if (parent != nullptr) {
        result = scopeForInternal(*parent, generator, addMainNamespace, true, sep);
    }
    else if (addMainNamespace) {
        result = generator.mainNamespace();
    }

    do {
        if (!addElement) {
            break;
        }

        auto& elemName = elem.name();
        if (elemName.empty()) {
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }

        auto elemType = elem.elemType();
        assert((elemType == Elem::Type_Namespace) || (parent != nullptr)); // Only namespace allowed not to have parent

        if (elemType == Elem::Type_Namespace) {
            result.append(namespaceName(elemName));
            break;
        }

        if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
            // Global fields reside in appropriate namespace
            result.append(strings::fieldNamespaceStr() + sep);
        }

        result.append(className(elem.name()));

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

std::string namespaceName(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::tolower(static_cast<int>(result[0])));
    }

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

std::string scopeForInterface(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string scopeForOptions(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsStr()
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
        strings::inputStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string relHeaderPathFor(const Elem& elem, const Generator& generator)
{
    return scopeForInternal(elem, generator, true, true, PathSep) + strings::cppHeaderSuffixStr();    
}

std::string relHeaderPathForField(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::fieldNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForOptions(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
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
    if (parent != nullptr) {
        result += namespaceBeginFor(*parent, generator);
    }
    else {
        appendToResultFunc(namespaceName(generator.mainNamespace()));
    }

    auto elemType = elem.elemType();
    if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
        appendToResultFunc(strings::fieldNamespaceStr());
    }    

    // TODO: other sub-namespaces

    if (elem.elemType() != Elem::Type_Namespace) {
        return result;
    }

    auto& elemName = elem.name();
    if (elemName.empty()) {
        return result;
    }

    appendToResultFunc(namespaceName(elemName));
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

    auto elemType = elem.elemType();
    if (elemType != Elem::Type_Namespace) {
        assert(parent != nullptr);
        if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
            appendToResultFunc(strings::fieldNamespaceStr());
        }

        result += namespaceEndFor(*parent, generator);

        // TODO: other sub namespaces

        return result;
    }

    auto& elemName = elem.name();
    
    if (!elemName.empty()) {
        appendToResultFunc(namespaceName(elemName));
    }
    
    if (parent != nullptr) {
        result += namespaceEndFor(*parent, generator);
    }
    else {
        appendToResultFunc(namespaceName(generator.mainNamespace()));
    }

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

bool isGlobalField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    return parent->elemType() == Elem::Type_Namespace;
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

} // namespace comms

} // namespace gen

} // namespace commsdsl
