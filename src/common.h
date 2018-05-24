#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <utility>

#include "commsdsl/Endian.h"
#include "commsdsl/Units.h"

namespace commsdsl
{

namespace common
{

using PropsMap = std::multimap<std::string, std::string>;

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
const std::string& floatStr();
const std::string& typeStr();
const std::string& defaultValueStr();
const std::string& unitsStr();
const std::string& scalingStr();
const std::string& lengthStr();
const std::string& bitLengthStr();
const std::string& serOffsetStr();
const std::string& validRangeStr();
const std::string& validFullRangeStr();
const std::string& validValueStr();
const std::string& specialStr();
const std::string& valStr();
const std::string& metaStr();
const std::string& validMinStr();
const std::string& validMaxStr();
const std::string& nanStr();
const std::string& infStr();
const std::string& negInfStr();
const std::string& bitfieldStr();
const std::string& bundleStr();
const std::string& membersStr();
const std::string& sinceVersionStr();
const std::string& deprecatedStr();
const std::string& removedStr();
const std::string& refStr();
const std::string& fieldStr();
const std::string& nsStr();
const std::string& enumStr();
const std::string& nonUniqueAllowedStr();
const std::string& nonUniqueMsgIdAllowedStr();
const std::string& reservedValueStr();
const std::string& reservedStr();
const std::string& bitStr();
const std::string& idxStr();
const std::string& setStr();
const std::string& reuseStr();
const std::string& validCheckVersionStr();
const std::string& lengthPrefixStr();
const std::string& countPrefixStr();
const std::string& encodingStr();
const std::string& zeroTermSuffixStr();
const std::string& stringStr();
const std::string& dataStr();
const std::string& countStr();
const std::string& elemFixedLengthStr();
const std::string& elementStr();
const std::string& elemLengthPrefixStr();
const std::string& listStr();
const std::string& optionalStr();
const std::string& defaultModeStr();
const std::string& condStr();
const std::string& andStr();
const std::string& orStr();
const std::string& copyFieldsFromStr();
const std::string& orderStr();
const std::string& platformsStr();
const std::string& platformStr();
const std::string& interfacesStr();
const std::string& interfaceStr();
const std::string& layersStr();
const std::string& payloadStr();

unsigned strToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);
std::intmax_t strToIntMax(const std::string& str, bool* ok = nullptr, int base = 0);
std::uintmax_t strToUintMax(const std::string& str, bool* ok = nullptr, int base = 0);
double strToDouble(const std::string& str, bool* ok = nullptr, bool allowSpecials = true);
bool strToBool(const std::string& str, bool* ok = nullptr);
bool isFpSpecial(const std::string& str);
Units strToUnits(const std::string& str, bool* ok = nullptr);

const std::string& getStringProp(
    const PropsMap& map,
    const std::string& prop,
    const std::string& defaultValue = emptyString());

Endian parseEndian(const std::string& value, Endian defaultEndian);

void toLower(std::string& str);
std::string toLowerCopy(const std::string& str);
void removeHeadingTrailingWhitespaces(std::string& str);
void normaliseString(std::string& str);
std::pair<std::string, std::string> parseRange(const std::string& str, bool* ok = nullptr);

bool isValidName(const std::string& value);
bool isValidRefName(const std::string& value);

} // namespace common

} // namespace commsdsl
