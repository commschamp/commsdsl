//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

[[nodiscard]] const std::string& parseEmptyString();
[[nodiscard]] const std::string& parseNameStr();
[[nodiscard]] const std::string& parseDisplayNameStr();
[[nodiscard]] const std::string& parseIdStr();
[[nodiscard]] const std::string& parseVersionStr();
[[nodiscard]] const std::string& parseDslVersionStr();
[[nodiscard]] const std::string& parseDescriptionStr();
[[nodiscard]] const std::string& parseEndianStr();
[[nodiscard]] const std::string& parseBigStr();
[[nodiscard]] const std::string& parseLittleStr();
[[nodiscard]] const std::string& parseFieldsStr();
[[nodiscard]] const std::string& parseMessagesStr();
[[nodiscard]] const std::string& parseMessageStr();
[[nodiscard]] const std::string& parseFrameStr();
[[nodiscard]] const std::string& parseFramesStr();
[[nodiscard]] const std::string& parseIntStr();
[[nodiscard]] const std::string& parseFloatStr();
[[nodiscard]] const std::string& parseTypeStr();
[[nodiscard]] const std::string& parseDefaultValueStr();
[[nodiscard]] const std::string& parseUnitsStr();
[[nodiscard]] const std::string& parseScalingStr();
[[nodiscard]] const std::string& parseLengthStr();
[[nodiscard]] const std::string& parseBitLengthStr();
[[nodiscard]] const std::string& parseSerOffparseSetStr();
[[nodiscard]] const std::string& parseValidRangeStr();
[[nodiscard]] const std::string& parseValidFullRangeStr();
[[nodiscard]] const std::string& parseValidValueStr();
[[nodiscard]] const std::string& parseSpecialStr();
[[nodiscard]] const std::string& parseValStr();
[[nodiscard]] const std::string& parseMetaStr();
[[nodiscard]] const std::string& parseValidMinStr();
[[nodiscard]] const std::string& parseValidMaxStr();
[[nodiscard]] const std::string& parseNanStr();
[[nodiscard]] const std::string& parseInfStr();
[[nodiscard]] const std::string& parseNegInfStr();
[[nodiscard]] const std::string& parseBitparseFieldStr();
[[nodiscard]] const std::string& parseBundleStr();
[[nodiscard]] const std::string& parseVariantStr();
[[nodiscard]] const std::string& parseMembersStr();
[[nodiscard]] const std::string& parseSinceVersionStr();
[[nodiscard]] const std::string& parseDeprecatedStr();
[[nodiscard]] const std::string& parseRemovedStr();
[[nodiscard]] const std::string& parseRefStr();
[[nodiscard]] const std::string& parseFieldStr();
[[nodiscard]] const std::string& parseNsStr();
[[nodiscard]] const std::string& parseEnumStr();
[[nodiscard]] const std::string& parseNonUniqueAllowedStr();
[[nodiscard]] const std::string& parseNonUniqueSpecialsAllowedStr();
[[nodiscard]] const std::string& parseNonUniqueMsgIdAllowedStr();
[[nodiscard]] const std::string& parseReservedValueStr();
[[nodiscard]] const std::string& parseReservedStr();
[[nodiscard]] const std::string& parseBitStr();
[[nodiscard]] const std::string& parseBitsStr();
[[nodiscard]] const std::string& parseIdxStr();
[[nodiscard]] const std::string& parseSetStr();
[[nodiscard]] const std::string& parseMaskStr();
[[nodiscard]] const std::string& parseReuseStr();
[[nodiscard]] const std::string& parseValidCheckVersionStr();
[[nodiscard]] const std::string& parseLengthPrefixStr();
[[nodiscard]] const std::string& parseCountPrefixStr();
[[nodiscard]] const std::string& parseEncodingStr();
[[nodiscard]] const std::string& parseZeroTermSuffixStr();
[[nodiscard]] const std::string& parseStringStr();
[[nodiscard]] const std::string& parseDataStr();
[[nodiscard]] const std::string& parseCountStr();
[[nodiscard]] const std::string& parseElemFixedLengthStr();
[[nodiscard]] const std::string& parseElementStr();
[[nodiscard]] const std::string& parseElemLengthPrefixStr();
[[nodiscard]] const std::string& parseListStr();
[[nodiscard]] const std::string& parseOptionalStr();
[[nodiscard]] const std::string& parseDefaultModeStr();
[[nodiscard]] const std::string& parseCondStr();
[[nodiscard]] const std::string& parseAndStr();
[[nodiscard]] const std::string& parseOrStr();
[[nodiscard]] const std::string& parseCopyFieldsFromStr();
[[nodiscard]] const std::string& parseCopyFieldsAliasesStr();
[[nodiscard]] const std::string& parseOrderStr();
[[nodiscard]] const std::string& parsePlatformsStr();
[[nodiscard]] const std::string& parsePlatformStr();
[[nodiscard]] const std::string& parseInterfacesStr();
[[nodiscard]] const std::string& parseInterfaceStr();
[[nodiscard]] const std::string& parseLayersStr();
[[nodiscard]] const std::string& parsePayloadStr();
[[nodiscard]] const std::string& parseSizeStr();
[[nodiscard]] const std::string& parseSyncStr();
[[nodiscard]] const std::string& parseChecksumStr();
[[nodiscard]] const std::string& parseAlgStr();
[[nodiscard]] const std::string& parseAlgNameStr();
[[nodiscard]] const std::string& parseFromStr();
[[nodiscard]] const std::string& parseUntilStr();
[[nodiscard]] const std::string& parseVerifyBeforeReadStr();
[[nodiscard]] const std::string& parseInterfaceFieldNameStr();
[[nodiscard]] const std::string& parseValueStr();
[[nodiscard]] const std::string& pseudoStr();
[[nodiscard]] const std::string& parseFixedValueStr();
[[nodiscard]] const std::string& parseCustomStr();
[[nodiscard]] const std::string& parseSemanticTypeStr();
[[nodiscard]] const std::string& parseNoneStr();
[[nodiscard]] const std::string& parseMessageIdStr();
[[nodiscard]] const std::string& parseIdReplacementStr();
[[nodiscard]] const std::string& parseDisplayDesimalsStr();
[[nodiscard]] const std::string& parseDisplayOffsetStr();
[[nodiscard]] const std::string& parseDisplayExtModeCtrlStr();
[[nodiscard]] const std::string& parseHexAssignStr();
[[nodiscard]] const std::string& parseSignExtStr();
[[nodiscard]] const std::string& parseDisplayReadOnlyStr();
[[nodiscard]] const std::string& parseDisplayHiddenStr();
[[nodiscard]] const std::string& parseCustomizableStr();
[[nodiscard]] const std::string& parseFailOnInvalidStr();
[[nodiscard]] const std::string& parseSenderStr();
[[nodiscard]] const std::string& parseClientStr();
[[nodiscard]] const std::string& parseServerStr();
[[nodiscard]] const std::string& parseBothStr();
[[nodiscard]] const std::string& parseDefaultMemberStr();
[[nodiscard]] const std::string& parseDisplayIdxReadOnlyHiddenStr();
[[nodiscard]] const std::string& parseDisplaySpecialsStr();
[[nodiscard]] const std::string& parseAliasStr();
[[nodiscard]] const std::string& parseReuseAliasesStr();
[[nodiscard]] const std::string& parseForceGenStr();
[[nodiscard]] const std::string& parseValidateMinLengthStr();
[[nodiscard]] const std::string& parseDefaultValidValueStr();
[[nodiscard]] const std::string& parseAvailableLengthLimitStr();
[[nodiscard]] const std::string& parseValueOverrideStr();
[[nodiscard]] const std::string& parseReadOverrideStr();
[[nodiscard]] const std::string& parseWriteOverrideStr();
[[nodiscard]] const std::string& parseRefreshOverrideStr();
[[nodiscard]] const std::string& parseLengthOverrideStr();
[[nodiscard]] const std::string& parseValidOverrideStr();
[[nodiscard]] const std::string& parseNameOverrideStr();
[[nodiscard]] const std::string& parseReplaceStr();
[[nodiscard]] const std::string& parseCopyCodeFromStr();
[[nodiscard]] const std::string& parseSemanticLayerTypeStr();
[[nodiscard]] const std::string& parseChecksumFromStr();
[[nodiscard]] const std::string& parseChecksumUntilStr();
[[nodiscard]] const std::string& parseTermSuffixStr();
[[nodiscard]] const std::string& parseMissingOnReadFailStr();
[[nodiscard]] const std::string& parseMissingOnInvalparseIdStr();
[[nodiscard]] const std::string& parseReuseCodeStr();
[[nodiscard]] const std::string& parseConstructStr();
[[nodiscard]] const std::string& parseReadCondStr();
[[nodiscard]] const std::string& parseValidCondStr();
[[nodiscard]] const std::string& parseConstructAsReadCondStr();
[[nodiscard]] const std::string& parseConstructAsValidCondStr();
[[nodiscard]] const std::string& parseCopyConstructFromStr();
[[nodiscard]] const std::string& parseCopyReadCondFromStr();
[[nodiscard]] const std::string& parseCopyValidCondFromStr();
[[nodiscard]] const std::string& parseSeekFieldStr();
[[nodiscard]] const std::string& parseEscFieldStr();
[[nodiscard]] const std::string& parseSystemStr();
[[nodiscard]] const std::string& parseReadAlignStr();
[[nodiscard]] const std::string& parseWritePadAlignStr();
[[nodiscard]] const std::string& parseWritePadInLengthStr();

[[nodiscard]] char parseSiblingRefPrefix();
[[nodiscard]] char parseStringRefPrefix();
[[nodiscard]] char parseSchemaRefPrefix();
[[nodiscard]] char parseInterfaceRefPrefix();

[[nodiscard]] unsigned parseStrToUnsigned(const std::string& str, bool* ok = nullptr, int base = 0);
[[nodiscard]] std::intmax_t parseStrToIntMax(const std::string& str, bool* ok = nullptr, int base = 0);
[[nodiscard]] std::uintmax_t parseStrToUintMax(const std::string& str, bool* ok = nullptr, int base = 0);
[[nodiscard]] double parseStrToDouble(const std::string& str, bool* ok = nullptr, bool allowSpecials = true);
[[nodiscard]] bool parseStrToBool(const std::string& str, bool* ok = nullptr);
[[nodiscard]] bool parseIsFpSpecial(const std::string& str);
[[nodiscard]] ParseUnits parseStrToUnits(const std::string& str, bool* ok = nullptr);

[[nodiscard]] const std::string& parseGetStringProp(
    const ParsePropsMap& map,
    const std::string& prop,
    const std::string& defaultValue = parseEmptyString());

[[nodiscard]] commsdsl::parse::ParseEndian parseEndian(const std::string& value, commsdsl::parse::ParseEndian defaultEndian);

void parseToLower(std::string& str);
[[nodiscard]] std::string parseToLowerCopy(const std::string& str);
void parseRemoveHeadingTrailingWhitespaces(std::string& str);
void parseNormaliseString(std::string& str);
[[nodiscard]] std::pair<std::string, std::string> parseRange(const std::string& str, bool* ok = nullptr);

[[nodiscard]] bool parseIsValidName(const std::string& value);
[[nodiscard]] bool parseIsValidRefName(const char* buf, std::size_t len);
[[nodiscard]] bool parseIsValidRefName(const std::string& value);
[[nodiscard]] bool parseIsValidExternalRefName(const std::string& value);

[[nodiscard]] std::size_t parseMaxPossibleLength();
void parseAddToLength(std::size_t newLen, std::size_t& accLen);
[[nodiscard]] std::size_t parseMulLength(std::size_t len, std::size_t factor);

} // namespace common

} // namespace parse

} // namespace commsdsl
