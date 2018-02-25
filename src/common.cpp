#include "common.h"

#include <algorithm>
#include <iterator>

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

    auto mapIter = std::find(std::begin(Map), std::end(Map), value);
    if (mapIter == std::end(Map)) {
        return Endian_NumOfValues;
    }

    return static_cast<Endian>(std::distance(std::begin(Map), mapIter));
}

} // namespace common

} // namespace bbmp
