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
    const std::string& sep,
    const Elem* leaf = nullptr)
{
    std::string result;
    if (leaf == nullptr) {
        leaf = &elem;
    }

    auto fieldTypeScope = (leaf->elemType() == Elem::Type_Field) && (sep == ScopeSep);

    auto* parent = elem.getParent();
    if (parent != nullptr) {
        result = scopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
    }
    else if (addMainNamespace) {
        result = generator.mainNamespace();
    }

    do {
        if (!addElement) {
            break;
        }

        auto elemType = elem.elemType();
        auto& elemName = elem.name();

        assert((elemType == Elem::Type_Namespace) || (parent != nullptr)); // Only namespace allowed not to have parent

        if (elemType == Elem::Type_Namespace) {
            if (elemName.empty()) {
                break;
            }

            if (!result.empty()) {
                result.append(sep);
            }               

            result.append(namespaceName(elemName));
            break;
        }

        if (!result.empty()) {
            result.append(sep);
        }            

        auto name = className(elemName);
        if ((name.empty()) && (elemType == Elem::Type_Interface)) {
            name = strings::messageClassStr();
        }        

        if ((elemType == Elem::Type_Field) && (parent->elemType() == Elem::Type_Namespace)) {
            // Global fields reside in appropriate namespace
            result.append(strings::fieldNamespaceStr() + sep);
        }

        if (elemType == Elem::Type_Message) {
            assert(parent->elemType() == Elem::Type_Namespace);
            result.append(strings::messageNamespaceStr() + sep);
        }     

        if (elemType == Elem::Type_Frame) {
            assert(parent->elemType() == Elem::Type_Namespace);
            result.append(strings::frameNamespaceStr() + sep);
        }            

        result.append(name);

        if ((elemType == Elem::Type_Message) && (fieldTypeScope)) {
            result.append("Fields");
            break;
        }   

        if ((elemType == Elem::Type_Interface) && (fieldTypeScope)) {
            result.append("Fields");
            break;
        }           

        if ((elemType == Elem::Type_Field) && (fieldTypeScope) && (&elem != leaf)) {
            result.append("Members");
            break;
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
    if (parent != nullptr) {
        result = commonScopeForInternal(*parent, generator, addMainNamespace, true, sep, leaf);
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

        if (elemType == Elem::Type_Message) {
            assert(parent->elemType() == Elem::Type_Namespace);
            result.append(strings::messageNamespaceStr() + sep);
        }            

        result.append(className(elem.name()));

        if ((elemType == Elem::Type_Message) && (fieldTypeScope)) {
            result.append("Fields");
        }

        if ((elemType == Elem::Type_Interface) && (fieldTypeScope)) {
            result.append("Fields");
        }        

        if ((elemType == Elem::Type_Field) && (fieldTypeScope) && (&elem != leaf)) {
            result.append("Members");
        }        

        if ((elemType == Elem::Type_Field) || (elemType == Elem::Type_Message) || (elemType == Elem::Type_Interface)) {
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

std::string namespaceName(const std::string& name)
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
    if (parent != nullptr) {
        result = fullNameFor(*parent);
    }

    if (!result.empty()) {
        result += '_';
    }

    auto elemType = elem.elemType();
    do {
        if (elemType == Elem::Type_Namespace) {
            result.append(namespaceName(static_cast<const gen::Namespace&>(elem).dslObj().name()));
            break;
        }

        if (elemType == Elem::Type_Field) {
            result.append(className(static_cast<const gen::Field&>(elem).dslObj().name()));
            break;
        }

        if (elemType == Elem::Type_Message) {
            result.append(className(static_cast<const gen::Message&>(elem).dslObj().name()));
            break;
        }        

        assert(false); // Not implemented
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

std::string scopeForRoot(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);    
}

std::string relHeaderPathFor(const Elem& elem, const Generator& generator)
{
    return scopeForInternal(elem, generator, true, true, PathSep) + strings::cppHeaderSuffixStr();    
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

std::string relHeaderForOptions(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsNamespaceStr()
    };

    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string relHeaderForRoot(const std::string& name, const Generator& generator)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, true, true, PathSep) + strings::cppHeaderSuffixStr();
}

std::string headerPathFor(const Elem& elem, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderPathFor(elem, generator);
}

std::string headerPathForField(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderPathForField(name, generator);
}

std::string commonHeaderPathFor(const Elem& elem, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relCommonHeaderPathFor(elem, generator);
}

std::string headerPathRoot(const std::string& name, const Generator& generator)
{
    return generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + relHeaderForRoot(name, generator);
}

std::string inputCodePathFor(const Elem& elem, const Generator& generator)
{
    return generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + comms::relHeaderPathFor(elem, generator);
}

std::string inputCodePathForRoot(const std::string& name, const Generator& generator)
{
    return generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + comms::relHeaderForRoot(name, generator);
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

std::string cppIntChangedSignTypeFor(commsdsl::parse::IntField::Type value, std::size_t len)
{
    auto str = cppIntTypeFor(value, len);
    assert(str.find("std::") == 0U);
    if (str.size() < 6) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
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
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
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

bool isInterfaceMemberField(const Elem& elem)
{
    if (elem.elemType() != Elem::Type_Field) {
        return false;
    }

    auto* parent = elem.getParent();
    assert(parent != nullptr);
    return parent->elemType() == Elem::Type_Interface;    
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
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        value = commsdsl::parse::Endian_Little;
    }

    return Map[value];
}

const std::string& dslUnitsToOpt(commsdsl::parse::Units value)
{
    if (commsdsl::parse::Units::NumOfValues <= value) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
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
    auto msgIdField = generator.getMessageIdField();
    if (msgIdField == nullptr) {
        return generator.mainNamespace() + "::" + strings::msgIdPrefixStr() + comms::fullNameFor(msg);
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
        return generator.mainNamespace() + "::" + strings::msgIdPrefixStr() + name;
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
