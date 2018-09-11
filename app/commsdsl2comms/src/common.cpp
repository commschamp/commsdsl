#include "common.h"

#include <cctype>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>
#include <type_traits>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace common
{

const std::string& emptyString()
{
    static const std::string Str;
    return Str;
}

const std::string& headerSuffix()
{
    static const std::string Str(".h");
    return Str;
}

const std::string& srcSuffix()
{
    static const std::string Str(".cpp");
    return Str;
}

const std::string& includeStr()
{
    static const std::string Str("include");
    return Str;
}

const std::string& messageStr()
{
    static const std::string Str("message");
    return Str;
}

const std::string& messageClassStr()
{
    static const std::string Str("Message");
    return Str;
}

const std::string& frameStr()
{
    static const std::string Str("frame");
    return Str;
}

const std::string& fieldStr()
{
    static const std::string Str("field");
    return Str;
}

const std::string& fieldBaseStr()
{
    static const std::string Str("FieldBase");
    return Str;
}

const std::string& commsStr()
{
    static const std::string Str("comms");
    return Str;
}

const std::string& indentStr()
{
    static const std::string Str("    ");
    return Str;
}

const std::string& doxygenPrefixStr()
{
    static const std::string Str("/// ");
    return Str;
}

const std::string& fieldsSuffixStr()
{
    static const std::string Str("Fields");
    return Str;
}

const std::string& layersSuffixStr()
{
    static const std::string Str("Layers");
    return Str;
}

const std::string& membersSuffixStr()
{
    static const std::string Str("Members");
    return Str;
}

const std::string& optFieldSuffixStr()
{
    static const std::string Str("Field");
    return Str;
}

const std::string& prefixFieldSuffixStr()
{
    static const std::string Str("PrefixField");
    return Str;
}

const std::string& defaultOptionsStr()
{
    static const std::string Str("DefaultOptions");
    return Str;
}

const std::string& msgIdEnumNameStr()
{
    static const std::string Str("MsgId");
    return Str;
}

const std::string& msgIdPrefixStr()
{
    static const std::string Str(msgIdEnumNameStr() + "_");
    return Str;
}

const std::string& allMessagesStr()
{
    static const std::string Str("AllMessages");
    return Str;
}

const std::string& checksumStr()
{
    static const std::string Str("checksum");
    return Str;
}

const std::string& layerStr()
{
    static const std::string Str("layer");
    return Str;
}

const std::string& pluginNsStr()
{
    static const std::string Str("cc_plugin");
    return Str;
}

const std::string& serHiddenStr()
{
    static const std::string Str("serHidden");
    return Str;
}

const std::string& cmakeListsFileStr()
{
    static const std::string Str("CMakeLists.txt");
    return Str;
}

const std::string& transportMessageSuffixStr()
{
    static const std::string Str("TransportMessage");
    return Str;
}

const std::string& pluginStr()
{
    static const std::string Str("plugin");
    return Str;
}

const std::string& origSuffixStr()
{
    static const std::string Str("Orig");
    return Str;
}

const std::string& forceEmptyDisplayNameStr()
{
    static const std::string Str("_");
    return Str;
}

const std::string& docStr()
{
    static const std::string Str("doc");
    return Str;
}

void nameToClass(std::string& str)
{
    if (str.empty()) {
        return;
    }

    str[0] = static_cast<char>(std::toupper(static_cast<int>(str[0])));
}

std::string nameToClassCopy(const std::string& str)
{
    std::string result = str;
    nameToClass(result);
    return result;
}

void nameToAccess(std::string& str)
{
    if (str.empty()) {
        return;
    }

    str[0] = static_cast<char>(std::tolower(static_cast<int>(str[0])));
}

std::string nameToAccessCopy(const std::string& str)
{
    std::string result = str;
    nameToAccess(result);
    return result;
}

void updateName(std::string& str)
{
    ba::replace_all(str, " ", "_");
    ba::replace_all(str, ".", "_");
}

std::string updateNameCopy(const std::string& str)
{
    std::string cpy(str);
    updateName(cpy);
    return cpy;
}

std::string adjustName(const std::string& str)
{
    std::string result = str;
    updateName(result);
    return result;
}

std::string numToString(std::uintmax_t value, unsigned hexWidth)
{
    if (hexWidth == 0U) {
        if (value <= std::numeric_limits<std::uint16_t>::max()) {
            return std::to_string(value) + "U";
        }

        if (value <= std::numeric_limits<std::uint32_t>::max()) {
            return std::to_string(value) + "UL";
        }
    }

    std::stringstream stream;
    stream << std::hex << "0x" << std::uppercase <<
              std::setfill('0') << std::setw(hexWidth) << value;
    if ((0U < hexWidth) && (value <= std::numeric_limits<std::uint16_t>::max())) {
        stream << "U";
    }
    else if ((0U < hexWidth) && (value <= std::numeric_limits<std::uint32_t>::max())) {
        stream << "UL";
    }
    else {
        stream << "ULL";
    }
    return stream.str();
}

std::string numToString(std::intmax_t value)
{
    if ((std::numeric_limits<std::int16_t>::min() <= value) &&
        (value <= std::numeric_limits<std::int16_t>::max())) {
        return std::to_string(value);
    }

    if ((std::numeric_limits<std::int32_t>::min() <= value) &&
        (value <= std::numeric_limits<std::int32_t>::max())) {
        return std::to_string(value) + "L";
    }

    if (0 < value) {
        std::stringstream stream;
        stream << std::hex << "0x" << value << "LL";
        return stream.str();
    }

    std::stringstream stream;
    stream << std::hex << "-0x" << -value << "LL";
    return stream.str();
}

std::string makeMultilineCopy(const std::string& value, unsigned len)
{
    if (value.size() <= len) {
        return value;
    }

    assert(0U < len);
    std::string result;
    result.reserve((value.size() * 3) / 2);
    std::size_t pos = 0;
    while (pos < value.size()) {
        auto nextPos = pos + len;
        if (value.size() <= nextPos) {
            break;
        }

        static const std::string WhiteSpace(" \t\r");
        auto prePos = value.find_last_of(WhiteSpace, nextPos);
        if ((prePos == std::string::npos) || (prePos < pos)) {
            prePos = pos;
        }

        auto postPos = value.find_first_of(WhiteSpace, nextPos + 1);
        if (postPos == std::string::npos) {
            postPos = value.size();
        }

        if ((prePos <= pos) && (value.size() <= postPos)) {
            break;
        }

        auto insertFunc =
            [&result, &pos, &value](std::size_t newPos)
            {
                assert(pos <= newPos);
                assert(newPos <= value.size());
                result.insert(result.end(), value.begin() + pos, value.begin() + newPos);
                result.push_back('\n');
                pos = newPos + 1;
            };

        if (prePos <= pos) {
            insertFunc(postPos);
            continue;
        }

        if (value.size() <= postPos) {
            insertFunc(prePos);
            continue;
        }

        auto preDiff = nextPos - prePos;
        auto postDiff = postPos - nextPos;

        if (preDiff <= postDiff) {
            insertFunc(prePos);
            continue;
        }

        insertFunc(postPos);
    }

    if (pos < value.size()) {
        result.insert(result.end(), value.begin() + pos, value.end());
    }

    return result;
}

void insertIndent(std::string& str)
{
    auto& indent = indentStr();
    str.insert(str.begin(), indent.begin(), indent.end());
    ba::replace_all(str, "\n", "\n" + indent);
}

std::string insertIndentCopy(const std::string& str)
{
    std::string result(str);
    insertIndent(result);
    return result;
}

std::string processTemplate(const std::string& templ, const ReplacementMap& repl)
{
    std::string result;
    result.reserve(templ.size() * 2U);
    std::size_t templPos = 0U;
    while (templPos < templ.size()) {
        static const std::string Prefix("#^#");
        auto prefixPos = templ.find(Prefix, templPos);
        if (prefixPos == std::string::npos) {
            break;
        }

        static const std::string Suffix("#$#");
        auto suffixPos = templ.find(Suffix, prefixPos + Prefix.size());
        if (suffixPos == std::string::npos) {
            assert(!"Incorrect template");
            templPos = templ.size();
            break;
        }
        auto afterSuffixPos = suffixPos + Suffix.size();

        std::string key(templ.begin() + prefixPos + Prefix.size(), templ.begin() + suffixPos);
        const std::string* valuePtr = &emptyString();
        auto iter = repl.find(key);
        if (iter != repl.end()) {
            valuePtr = &(iter->second);
        }
        auto& value = *valuePtr;

        auto lineStartPos = 0U;
        auto lastNewLinePos = templ.find_last_of('\n', prefixPos);
        if (lastNewLinePos != std::string::npos) {
            lineStartPos = lastNewLinePos + 1U;
        }

        assert(lineStartPos <= prefixPos);
        auto indent = prefixPos - lineStartPos;

        // Check empty row
        std::size_t posToCopyUntil = prefixPos;
        std::size_t nextTemplPos = afterSuffixPos;
        do {
            if (!value.empty()) {
                break;
            }

            static const std::string WhiteSpaces(" \t\r");
            std::string preStr(templ.begin() + lineStartPos, templ.begin() + prefixPos);
            if ((!preStr.empty()) &&
                (preStr.find_first_not_of(WhiteSpaces) != std::string::npos)) {
                break;
            }

            auto nextNewLinePos = templ.find_first_of('\n', suffixPos + Suffix.size());
            if (nextNewLinePos == std::string::npos) {
                assert(!"Incorrect template");
                break;
            }

            std::string postStr(templ.begin() + afterSuffixPos, templ.begin() + nextNewLinePos);
            if ((!postStr.empty()) &&
                (postStr.find_first_not_of(WhiteSpaces) != std::string::npos)) {
                break;
            }

            posToCopyUntil = lineStartPos;
            nextTemplPos = nextNewLinePos + 1;
        } while (false);

        result.insert(result.end(), templ.begin() + templPos, templ.begin() + posToCopyUntil);
        templPos = nextTemplPos;

        if (value.empty()) {
            continue;
        }

        if (indent == 0U) {
            result += value;
            continue;
        }

        std::string repSep("\n");
        repSep.reserve(repSep.size() + indent);
        std::fill_n(std::back_inserter(repSep), indent, ' ');
        auto updatedValue = ba::replace_all_copy(value, "\n", repSep);
        result += updatedValue;
    }

    if (templPos < templ.size()) {
        result.insert(result.end(), templ.begin() + templPos, templ.end());
    }
    return result;
}

void mergeIncludes(const StringsList& from, StringsList& to)
{
    to.reserve(to.size() + from.size());
    for (auto& inc : from) {
        auto iter = std::lower_bound(to.begin(), to.end(), inc);
        if ((iter != to.end()) && (*iter == inc)) {
            continue;
        }

        to.insert(iter, inc);
    }
}

void mergeInclude(const std::string& inc, StringsList& to)
{
    StringsList list;
    list.push_back(inc);
    mergeIncludes(list, to);
}

std::string includesToStatements(const StringsList& list)
{
    std::string result;
    for (auto& inc : list) {
        result += "#include ";
        if (inc[0] != '<') {
            result += '\"';
            result += inc;
            result += '\"';
        }
        else {
            result += inc;
        }
        result += '\n';
    }
    return result;
}

std::string listToString(
    const StringsList& list,
    const std::string& join,
    const std::string& last)
{
    std::string result;
    for (auto& e : list) {
        result += e;
        if (&e != &list.back()) {
            result += join;
        }
        else {
            result += last;
        }
    }
    return result;
}

void addToList(const std::string& what, StringsList& to)
{
    auto iter = std::find(to.begin(), to.end(), what);
    if (iter == to.end()) {
        to.push_back(what);
    }
}

const std::string& dslEndianToOpt(commsdsl::Endian value)
{
    static const std::string Map[] = {
        "comms::option::LittleEndian",
        "comms::option::BigEndian"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<decltype(MapSize)>(commsdsl::Endian_NumOfValues),
        "Invalid map");

    if (commsdsl::Endian_NumOfValues <= value) {
        assert(!"Should not happen");
        value = commsdsl::Endian_Little;
    }

    return Map[value];
}

const std::string& dslUnitsToOpt(commsdsl::Units value)
{
    if (commsdsl::Units::NumOfValues <= value) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    static const std::string UnitsMap[] = {
        /* Unknown */ common::emptyString(),
        /* Nanoseconds */ "comms::option::UnitsNanoseconds",
        /* Microseconds */ "comms::option::UnitsMicroseconds",
        /* Milliseconds */ "comms::option::UnitsMilliseconds",
        /* Seconds */ "comms::option::UnitsSeconds",
        /* Minutes */ "comms::option::UnitsMinutes",
        /* Hours */ "comms::option::UnitsHours",
        /* Days */ "comms::option::UnitsDays",
        /* Weeks */ "comms::option::UnitsWeeks",
        /* Nanometers */ "comms::option::UnitsNanometers",
        /* Micrometers */ "comms::option::UnitsMicrometers",
        /* Millimeters */ "comms::option::UnitsMillimeters",
        /* Centimeters */ "comms::option::UnitsCentimeters",
        /* Meters */ "comms::option::UnitsMeters",
        /* Kilometers */ "comms::option::UnitsKilometers",
        /* NanometersPerSecond */ "comms::option::UnitsNanometersPerSecond",
        /* MicrometersPerSecond */ "comms::option::UnitsMicrometersPerSecond",
        /* MillimetersPerSecond */ "comms::option::UnitsMillimetersPerSecond",
        /* CentimetersPerSecond */ "comms::option::UnitsCentimetersPerSecond",
        /* MetersPerSecond */ "comms::option::UnitsMetersPerSecond",
        /* KilometersPerSecond */ "comms::option::UnitsKilometersPerSecond",
        /* KilometersPerHour */ "comms::option::UnitsKilometersPerHour",
        /* Hertz */ "comms::option::UnitsHertz",
        /* KiloHertz */ "comms::option::UnitsKilohertz",
        /* MegaHertz */ "comms::option::UnitsMegahertz",
        /* GigaHertz */ "comms::option::UnitsGigahertz",
        /* Degrees */ "comms::option::UnitsDegrees",
        /* Radians */ "comms::option::UnitsRadians",
        /* Nanoamps */ "comms::option::UnitsNanoamps",
        /* Microamps */ "comms::option::UnitsMicroamps",
        /* Milliamps */ "comms::option::UnitsMilliamps",
        /* Amps */ "comms::option::UnitsAmps",
        /* Kiloamps */ "comms::option::UnitsKiloamps",
        /* Nanovolts */ "comms::option::UnitsNanovolts",
        /* Microvolts */ "comms::option::UnitsMicrovolts",
        /* Millivolts */ "comms::option::UnitsMillivolts",
        /* Volts */ "comms::option::UnitsVolts",
        /* Kilovolts */ "comms::option::UnitsKilovolts",
    };

    static const std::size_t UnitsMapSize =
        std::extent<decltype(UnitsMap)>::value;
    static_assert(static_cast<decltype(UnitsMapSize)>(commsdsl::Units::NumOfValues) == UnitsMapSize,
        "Invalid Map");

    auto idx = static_cast<unsigned>(value);
    return UnitsMap[idx];
}

const std::string& displayName(const std::string& dslDisplayName, const std::string& dslName)
{
    if (dslDisplayName.empty()) {
        return dslName;
    }

    if (dslDisplayName == forceEmptyDisplayNameStr()) {
        return emptyString();
    }

    return dslDisplayName;

}

void toLower(std::string& str)
{
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
}

} // namespace common

} // namespace commsdsl2comms
