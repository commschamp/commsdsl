//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <utility>

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/Units.h"

namespace commsdsl
{

namespace parse
{

namespace common
{

using PropsMap = std::multimap<std::string, std::string>;

const std::string& emptyString();
const std::string& nameStr();
const std::string& displayNameStr();
const std::string& idStr();
const std::string& versionStr();
const std::string& dslVersionStr();
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
const std::string& variantStr();
const std::string& membersStr();
const std::string& sinceVersionStr();
const std::string& deprecatedStr();
const std::string& removedStr();
const std::string& refStr();
const std::string& fieldStr();
const std::string& nsStr();
const std::string& enumStr();
const std::string& nonUniqueAllowedStr();
const std::string& nonUniqueSpecialsAllowedStr();
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
const std::string& copyFieldsAliasesStr();
const std::string& orderStr();
const std::string& platformsStr();
const std::string& platformStr();
const std::string& interfacesStr();
const std::string& interfaceStr();
const std::string& layersStr();
const std::string& payloadStr();
const std::string& sizeStr();
const std::string& syncStr();
const std::string& checksumStr();
const std::string& algStr();
const std::string& algNameStr();
const std::string& fromStr();
const std::string& untilStr();
const std::string& verifyBeforeReadStr();
const std::string& interfaceFieldNameStr();
const std::string& valueStr();
const std::string& pseudoStr();
const std::string& customStr();
const std::string& semanticTypeStr();
const std::string& noneStr();
const std::string& messageIdStr();
const std::string& idReplacementStr();
const std::string& displayDesimalsStr();
const std::string& displayOffsetStr();
const std::string& displayExtModeCtrlStr();
const std::string& hexAssignStr();
const std::string& signExtStr();
const std::string& displayReadOnlyStr();
const std::string& displayHiddenStr();
const std::string& customizableStr();
const std::string& failOnInvalidStr();
const std::string& senderStr();
const std::string& clientStr();
const std::string& serverStr();
const std::string& bothStr();
const std::string& defaultMemberStr();
const std::string& displayIdxReadOnlyHiddenStr();
const std::string& displaySpecialsStr();
const std::string& aliasStr();
const std::string& reuseAliasesStr();
const std::string& forceGenStr();
const std::string& validateMinLengthStr();
const std::string& defaultValidValueStr();
const std::string& availableLengthLimitStr();
const std::string& valueOverrideStr();
const std::string& readOverrideStr();
const std::string& writeOverrideStr();
const std::string& refreshOverrideStr();
const std::string& lengthOverrideStr();
const std::string& validOverrideStr();
const std::string& nameOverrideStr();
const std::string& replaceStr();
const std::string& copyOverrideCodeFromStr();
const std::string& semanticLayerTypeStr();
const std::string& checksumFromStr();
const std::string& checksumUntilStr();

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
bool isValidRefName(const char* buf, std::size_t len);
bool isValidRefName(const std::string& value);
bool isValidExternalRefName(const std::string& value);

std::size_t maxPossibleLength();
void addToLength(std::size_t newLen, std::size_t& accLen);
std::size_t mulLength(std::size_t len, std::size_t factor);

} // namespace common

} // namespace parse

} // namespace commsdsl
