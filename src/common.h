#pragma once

#include <string>

namespace bbmp
{

namespace common
{

const std::string& emptyString();
const std::string& nameStr();
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

unsigned strToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);



} // namespace common

} // namespace bbmp
