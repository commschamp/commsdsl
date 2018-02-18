#pragma once

#include <string>
#include <map>

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

unsigned strToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);

const std::string& getStringProp(
    const PropsMap& map,
    const std::string prop,
    const std::string& defaultValue = emptyString());

} // namespace common

} // namespace bbmp
