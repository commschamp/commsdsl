#pragma once

#include <string>
#include <map>
#include <cstdint>

#include "bbmp/Endian.h"

namespace bbmp
{

namespace common
{

using PropsMap = std::map<std::string, std::string>;

const std::string& emptyString();
const std::string& nameStr();
const std::string& displayNameStr();
const std::string& idStr();
const std::string& versionStr();
const std::string& descriptionStr();
const std::string& endianStr();
const std::string& bigStr();
const std::string& littleStr();
const std::string& fieldsStr();
const std::string& messagesStr();
const std::string& messageStr();
const std::string& frameStr();
const std::string& framesStr();
const std::string& intStr();
const std::string& typeStr();
const std::string& defaultValueStr();
const std::string& unitsStr();
const std::string& scalingStr();
const std::string& lengthStr();

unsigned strToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);
std::intmax_t strToIntMax(const std::string& str, bool* ok = nullptr, int base = 0);
std::uintmax_t strToUintMax(const std::string& str, bool* ok = nullptr, int base = 0);

const std::string& getStringProp(
    const PropsMap& map,
    const std::string prop,
    const std::string& defaultValue = emptyString());

Endian parseEndian(const std::string& value, Endian defaultEndian);

} // namespace common

} // namespace bbmp
