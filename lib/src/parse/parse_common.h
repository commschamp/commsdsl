//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseUnits.h"

#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace commsdsl
{

namespace parse
{

namespace common
{

using ParsePropsMap = std::multimap<std::string, std::string>;

const std::string& parseEmptyString();
const std::string& parseNameStr();
const std::string& parseDisplayNameStr();
const std::string& parseIdStr();
const std::string& parseVersionStr();
const std::string& parseDslVersionStr();
const std::string& parseDescriptionStr();
const std::string& parseEndianStr();
const std::string& parseBigStr();
const std::string& parseLittleStr();
const std::string& parseFieldsStr();
const std::string& parseMessagesStr();
const std::string& parseMessageStr();
const std::string& parseFrameStr();
const std::string& parseFramesStr();
const std::string& parseIntStr();
const std::string& parseFloatStr();
const std::string& parseTypeStr();
const std::string& parseDefaultValueStr();
const std::string& parseUnitsStr();
const std::string& parseScalingStr();
const std::string& parseLengthStr();
const std::string& parseBitLengthStr();
const std::string& parseSerOffparseSetStr();
const std::string& parseValidRangeStr();
const std::string& parseValidFullRangeStr();
const std::string& parseValidValueStr();
const std::string& parseSpecialStr();
const std::string& parseValStr();
const std::string& parseMetaStr();
const std::string& parseValidMinStr();
const std::string& parseValidMaxStr();
const std::string& parseNanStr();
const std::string& parseInfStr();
const std::string& parseNegInfStr();
const std::string& parseBitparseFieldStr();
const std::string& parseBundleStr();
const std::string& parseVariantStr();
const std::string& parseMembersStr();
const std::string& parseSinceVersionStr();
const std::string& parseDeprecatedStr();
const std::string& parseRemovedStr();
const std::string& parseRefStr();
const std::string& parseFieldStr();
const std::string& parseNsStr();
const std::string& parseEnumStr();
const std::string& parseNonUniqueAllowedStr();
const std::string& parseNonUniqueSpecialsAllowedStr();
const std::string& parseNonUniqueMsgIdAllowedStr();
const std::string& parseReservedValueStr();
const std::string& parseReservedStr();
const std::string& parseBitStr();
const std::string& parseIdxStr();
const std::string& parseSetStr();
const std::string& parseReuseStr();
const std::string& parseValidCheckVersionStr();
const std::string& parseLengthPrefixStr();
const std::string& parseCountPrefixStr();
const std::string& parseEncodingStr();
const std::string& parseZeroTermSuffixStr();
const std::string& parseStringStr();
const std::string& parseDataStr();
const std::string& parseCountStr();
const std::string& parseElemFixedLengthStr();
const std::string& parseElementStr();
const std::string& parseElemLengthPrefixStr();
const std::string& parseListStr();
const std::string& parseOptionalStr();
const std::string& parseDefaultModeStr();
const std::string& parseCondStr();
const std::string& parseAndStr();
const std::string& parseOrStr();
const std::string& parseCopyFieldsFromStr();
const std::string& parseCopyFieldsAliasesStr();
const std::string& parseOrderStr();
const std::string& parsePlatformsStr();
const std::string& parsePlatformStr();
const std::string& parseInterfacesStr();
const std::string& parseInterfaceStr();
const std::string& parseLayersStr();
const std::string& parsePayloadStr();
const std::string& parseSizeStr();
const std::string& parseSyncStr();
const std::string& parseChecksumStr();
const std::string& parseAlgStr();
const std::string& parseAlgNameStr();
const std::string& parseFromStr();
const std::string& parseUntilStr();
const std::string& parseVerifyBeforeReadStr();
const std::string& parseInterfaceFieldNameStr();
const std::string& parseValueStr();
const std::string& pseudoStr();
const std::string& parseFixedValueStr();
const std::string& parseCustomStr();
const std::string& parseSemanticTypeStr();
const std::string& parseNoneStr();
const std::string& parseMessageIdStr();
const std::string& parseIdReplacementStr();
const std::string& parseDisplayDesimalsStr();
const std::string& parseDisplayOffsetStr();
const std::string& parseDisplayExtModeCtrlStr();
const std::string& parseHexAssignStr();
const std::string& parseSignExtStr();
const std::string& parseDisplayReadOnlyStr();
const std::string& parseDisplayHiddenStr();
const std::string& parseCustomizableStr();
const std::string& parseFailOnInvalidStr();
const std::string& parseSenderStr();
const std::string& parseClientStr();
const std::string& parseServerStr();
const std::string& parseBothStr();
const std::string& parseDefaultMemberStr();
const std::string& parseDisplayIdxReadOnlyHiddenStr();
const std::string& parseDisplaySpecialsStr();
const std::string& parseAliasStr();
const std::string& parseReuseAliasesStr();
const std::string& parseForceGenStr();
const std::string& parseValidateMinLengthStr();
const std::string& parseDefaultValidValueStr();
const std::string& parseAvailableLengthLimitStr();
const std::string& parseValueOverrideStr();
const std::string& parseReadOverrideStr();
const std::string& parseWriteOverrideStr();
const std::string& parseRefreshOverrideStr();
const std::string& parseLengthOverrideStr();
const std::string& parseValidOverrideStr();
const std::string& parseNameOverrideStr();
const std::string& parseReplaceStr();
const std::string& parseCopyCodeFromStr();
const std::string& parseSemanticLayerTypeStr();
const std::string& parseChecksumFromStr();
const std::string& parseChecksumUntilStr();
const std::string& parseTermSuffixStr();
const std::string& parseMissingOnReadFailStr();
const std::string& parseMissingOnInvalparseIdStr();
const std::string& parseReuseCodeStr();
const std::string& parseConstructStr();
const std::string& parseReadCondStr();
const std::string& parseValidCondStr();
const std::string& parseConstructAsReadCondStr();
const std::string& parseConstructAsValidCondStr();
const std::string& parseCopyConstructFromStr();
const std::string& parseCopyReadCondFromStr();
const std::string& parseCopyValidCondFromStr();

char parseSiblingRefPrefix();
char parseStringRefPrefix();
char parseSchemaRefPrefix();
char parseInterfaceRefPrefix();

unsigned parseStrToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);
std::intmax_t parseStrToIntMax(const std::string& str, bool* ok = nullptr, int base = 0);
std::uintmax_t parseStrToUintMax(const std::string& str, bool* ok = nullptr, int base = 0);
double parseStrToDouble(const std::string& str, bool* ok = nullptr, bool allowSpecials = true);
bool parseStrToBool(const std::string& str, bool* ok = nullptr);
bool parseIsFpSpecial(const std::string& str);
ParseUnits parseStrToUnits(const std::string& str, bool* ok = nullptr);

const std::string& parseGetStringProp(
    const ParsePropsMap& map,
    const std::string& prop,
    const std::string& defaultValue = parseEmptyString());

commsdsl::parse::ParseEndian parseEndian(const std::string& value, commsdsl::parse::ParseEndian defaultEndian);

void parseToLower(std::string& str);
std::string parseToLowerCopy(const std::string& str);
void parseRemoveHeadingTrailingWhitespaces(std::string& str);
void parseNormaliseString(std::string& str);
std::pair<std::string, std::string> parseRange(const std::string& str, bool* ok = nullptr);

bool parseIsValidName(const std::string& value);
bool parseIsValidRefName(const char* buf, std::size_t len);
bool parseIsValidRefName(const std::string& value);
bool parseIsValidExternalRefName(const std::string& value);

std::size_t parseMaxPossibleLength();
void parseAddToLength(std::size_t newLen, std::size_t& accLen);
std::size_t parseMulLength(std::size_t len, std::size_t factor);

} // namespace common

} // namespace parse

} // namespace commsdsl
