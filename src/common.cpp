#include "common.h"

#include <algorithm>
#include <iterator>
#include <cctype>
#include <cmath>
#include <cassert>

namespace bbmp
{

namespace common
{

const std::string& emptyString()
{
    static const std::string Str;
    return Str;
}

const std::string& nameStr()
{
    static const std::string Str("name");
    return Str;
}

const std::string& displayNameStr()
{
    static const std::string Str("displayName");
    return Str;
}

const std::string& idStr()
{
    static const std::string Str("id");
    return Str;
}

const std::string& versionStr()
{
    static const std::string Str("version");
    return Str;
}

const std::string& descriptionStr()
{
    static const std::string Str("description");
    return Str;
}

const std::string& endianStr()
{
    static const std::string Str("endian");
    return Str;
}

const std::string& bigStr()
{
    static const std::string Str("big");
    return Str;
}

const std::string& littleStr()
{
    static const std::string Str("little");
    return Str;
}

const std::string& fieldsStr()
{
    static const std::string Str("fields");
    return Str;
}

const std::string& messagesStr()
{
    static const std::string Str("messages");
    return Str;
}

const std::string& messageStr()
{
    static const std::string Str("message");
    return Str;
}

const std::string& frameStr()
{
    static const std::string Str("frame");
    return Str;
}

const std::string& framesStr()
{
    static const std::string Str("frames");
    return Str;
}

const std::string& intStr()
{
    static const std::string Str("int");
    return Str;
}

const std::string& floatStr()
{
    static const std::string Str("float");
    return Str;
}

const std::string& typeStr()
{
    static const std::string Str("type");
    return Str;
}

const std::string& defaultValueStr()
{
    static const std::string Str("defaultValue");
    return Str;
}

const std::string& unitsStr()
{
    static const std::string Str("units");
    return Str;
}

const std::string& scalingStr()
{
    static const std::string Str("scaling");
    return Str;
}

const std::string& lengthStr()
{
    static const std::string Str("length");
    return Str;
}

const std::string& bitLengthStr()
{
    static const std::string Str("bitLength");
    return Str;
}

const std::string& serOffsetStr()
{
    static const std::string Str("serOffset");
    return Str;
}

const std::string& validRangeStr()
{
    static const std::string Str("validRange");
    return Str;
}

const std::string& validFullRangeStr()
{
    static const std::string Str("validFullRange");
    return Str;
}

const std::string& validValueStr()
{
    static const std::string Str("validValue");
    return Str;
}

const std::string& specialStr()
{
    static const std::string Str("special");
    return Str;
}

const std::string& valStr()
{
    static const std::string Str("val");
    return Str;
}

const std::string& metaStr()
{
    static const std::string Str("meta");
    return Str;
}

const std::string& validMinStr()
{
    static const std::string Str("validMin");
    return Str;
}

const std::string& validMaxStr()
{
    static const std::string Str("validMax");
    return Str;
}

const std::string& nanStr()
{
    static const std::string Str("nan");
    return Str;
}

const std::string& infStr()
{
    static const std::string Str("inf");
    return Str;
}

const std::string& negInfStr()
{
    static const std::string Str("-inf");
    return Str;
}

const std::string& bitfieldStr()
{
    static const std::string Str("bitfield");
    return Str;
}

const std::string& bundleStr()
{
    static const std::string Str("bundle");
    return Str;
}

const std::string& membersStr()
{
    static const std::string Str("members");
    return Str;
}

const std::string& sinceVersionStr()
{
    static const std::string Str("sinceVersion");
    return Str;
}

const std::string& deprecatedStr()
{
    static const std::string Str("deprecated");
    return Str;
}

const std::string& refStr()
{
    static const std::string Str("ref");
    return Str;
}

const std::string& origStr()
{
    static const std::string Str("orig");
    return Str;
}

const std::string& nsStr()
{
    static const std::string Str("ns");
    return Str;
}

const std::string& enumStr()
{
    static const std::string Str("enum");
    return Str;
}

const std::string& nonUniqueAllowedStr()
{
    static const std::string Str("nonUniqueAllowed");
    return Str;
}

const std::string& reservedValueStr()
{
    static const std::string Str("reservedValue");
    return Str;
}

const std::string& reservedStr()
{
    static const std::string Str("reserved");
    return Str;
}

const std::string& bitStr()
{
    static const std::string Str("bit");
    return Str;
}

const std::string& idxStr()
{
    static const std::string Str("idx");
    return Str;
}

const std::string& setStr()
{
    static const std::string Str("set");
    return Str;
}

const std::string& reuseStr()
{
    static const std::string Str("reuse");
    return Str;
}

unsigned strToUnsigned(const std::string& str, bool* ok, int base)
{
    unsigned result = 0U;
    try {
        result = std::stoul(str, 0, base);
        if (ok != nullptr) {
            *ok = true;
        }

    } catch (...) {
        if (ok != nullptr) {
            *ok = false;
        }
    }
    return result;
}

std::intmax_t strToIntMax(const std::string& str, bool* ok, int base)
{
    std::intmax_t result = 0;
    try {
        result = std::stoll(str, 0, base);
        if (ok != nullptr) {
            *ok = true;
        }

    } catch (...) {
        if (ok != nullptr) {
            *ok = false;
        }
    }
    return result;
}

std::uintmax_t strToUintMax(const std::string& str, bool* ok, int base)
{
    std::uintmax_t result = 0U;
    try {
        result = std::stoull(str, 0, base);
        if (ok != nullptr) {
            *ok = true;
        }

    } catch (...) {
        if (ok != nullptr) {
            *ok = false;
        }
    }
    return result;
}

double strToDouble(const std::string& str, bool* ok, bool allowSpecials)
{
    auto updateOk =
        [ok](bool val)
        {
            if (ok != nullptr) {
                *ok = val;
            }
        };

    if (allowSpecials) {
        static const std::map<std::string, double> Map = {
            std::make_pair(nanStr(), std::numeric_limits<double>::quiet_NaN()),
            std::make_pair(infStr(), std::numeric_limits<double>::infinity()),
            std::make_pair(negInfStr(), -(std::numeric_limits<double>::infinity()))
        };

        auto iter = Map.find(str);
        if (iter != Map.end()) {
            updateOk(true);
            return iter->second;
        }
    }

    double result = 0.0;
    try {
        result = std::stod(str, 0);
        updateOk(true);

    } catch (...) {
        updateOk(false);
    }
    return result;
}

bool strToBool(const std::string& str, bool* ok)
{
    auto updateOkFunc =
        [&ok](bool val)
        {
            if (ok != nullptr) {
                *ok = val;
            }
        };

    static const std::string TrueMap[] = {
        "true",
        "1"
    };

    auto strCopy = toLowerCopy(str);
    auto trueIter = std::find(std::begin(TrueMap), std::end(TrueMap), strCopy);
    if (trueIter != std::end(TrueMap)) {
        updateOkFunc(true);
        return true;
    }

    static const std::string FalseMap[] = {
        "false",
        "0"
    };

    auto falseIter = std::find(std::begin(FalseMap), std::end(FalseMap), strCopy);
    bool okValue = false;
    if (falseIter != std::end(FalseMap)) {
        okValue = true;
    }
    updateOkFunc(okValue);
    return false;
}

bool isFpSpecial(const std::string& str)
{
    static const std::string Map[] = {
        nanStr(),
        infStr(),
        negInfStr()
    };

    auto iter = std::find(std::begin(Map), std::end(Map), str);
    return iter != std::end(Map);
}

const std::string& getStringProp(
    const PropsMap& map,
    const std::string prop,
    const std::string& defaultValue)
{
    auto iter = map.lower_bound(prop);
    if ((iter == map.end()) || (iter->first != prop)) {
        return defaultValue;
    }

    return iter->second;
}

Endian parseEndian(const std::string& value, Endian defaultEndian)
{
    if (value.empty()) {
        return defaultEndian;
    }

    static const std::string Map[] = {
        /* Endian_Little */ common::littleStr(),
        /* Endian_Big */ common::bigStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == Endian_NumOfValues, "Invalid map");

    auto valueCpy = toLowerCopy(value);
    auto mapIter = std::find(std::begin(Map), std::end(Map), valueCpy);
    if (mapIter == std::end(Map)) {
        return Endian_NumOfValues;
    }

    return static_cast<Endian>(std::distance(std::begin(Map), mapIter));
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

std::string toLowerCopy(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    std::transform(
        str.begin(), str.end(), std::back_inserter(result),
        [](char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
    return result;
}

std::pair<std::string, std::string> parseRange(const std::string& str, bool* ok)
{
    bool status = false;
    std::pair<std::string, std::string> result;
    do {
        static const char Beg = '[';
        static const char End = ']';
        static const char Sep = ',';

        if (str.size() <= 3U) {
            break;
        }

        auto begPos = str.find(Beg, 0);
        if (begPos != 0) {
            break;
        }

        auto sepPos = str.find(Sep, begPos + 1);
        if (sepPos == std::string::npos) {
            break;
        }

        if (str.find(Sep, sepPos + 1) != std::string::npos) {
            break;
        }

        auto endPos = str.find(End, sepPos + 1);
        if ((endPos == std::string::npos) ||
            (endPos != (str.size() - 1))) {
            break;
        }

        static const std::string WhiteChars(" \t");
        auto beforeSepPos = str.find_last_not_of(WhiteChars, sepPos - 1);
        assert(beforeSepPos != std::string::npos);
        auto afterSepPos = str.find_first_not_of(WhiteChars, sepPos + 1);
        assert(afterSepPos != std::string::npos);

        result.first.assign(str.begin() + begPos + 1, str.begin() + beforeSepPos + 1);
        result.second.assign(str.begin() + afterSepPos, str.begin() + endPos);
        status = true;
    } while (false);

    if (ok != nullptr) {
        *ok = status;
    }

    return result;
}

bool isValidName(const std::string& value)
{
    if (value.empty()) {
        return false;
    }

    if ((std::isalpha(value[0]) == 0) && (value[0] != '_')) {
        return false;
    }

    return std::all_of(
                value.begin(), value.end(),
                [](char ch)
                {
                    return (std::isalnum(ch) != 0) || (ch == '_');
                });
}

bool isValidRefName(const std::string& value)
{
    if (value.empty()) {
        return false;
    }

    if ((std::isalpha(value[0]) == 0) && (value[0] != '_')) {
        return false;
    }

    bool validChars =
        std::all_of(
            value.begin(), value.end(),
            [](char ch)
            {
                return (std::isalnum(ch) != 0) || (ch == '_') || (ch == '.');
            });
    if (!validChars) {
        return false;
    }

    if (value[value.size() - 1] == '.') {
        return false;
    }

    auto dotPos = value.find_first_of('.');
    while (dotPos != std::string::npos) {
        auto nextPos = value.find_first_of('.', dotPos + 1);
        if (nextPos <= (dotPos + 1)) {
            return false; // sequential dots without name in the middle
        }
        dotPos = nextPos;
    }

    return true;
}

} // namespace common

} // namespace bbmp
