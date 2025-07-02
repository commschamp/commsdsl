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

#include "parse_common.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iterator>
#include <limits>

namespace commsdsl
{

namespace parse
{

namespace common
{

namespace
{

const std::size_t MaxPossibleLength = std::numeric_limits<std::size_t>::max();

}

const std::string& parseEmptyString()
{
    static const std::string Str;
    return Str;
}

const std::string& parseNameStr()
{
    static const std::string Str("name");
    return Str;
}

const std::string& parseDisplayNameStr()
{
    static const std::string Str("displayName");
    return Str;
}

const std::string& parseIdStr()
{
    static const std::string Str("id");
    return Str;
}

const std::string& parseVersionStr()
{
    static const std::string Str("version");
    return Str;
}

const std::string& parseDslVersionStr()
{
    static const std::string Str("dslVersion");
    return Str;
}

const std::string& parseDescriptionStr()
{
    static const std::string Str("description");
    return Str;
}

const std::string& parseEndianStr()
{
    static const std::string Str("endian");
    return Str;
}

const std::string& parseBigStr()
{
    static const std::string Str("big");
    return Str;
}

const std::string& parseLittleStr()
{
    static const std::string Str("little");
    return Str;
}

const std::string& parseFieldsStr()
{
    static const std::string Str("fields");
    return Str;
}

const std::string& parseMessagesStr()
{
    static const std::string Str("messages");
    return Str;
}

const std::string& parseMessageStr()
{
    static const std::string Str("message");
    return Str;
}

const std::string& parseFrameStr()
{
    static const std::string Str("frame");
    return Str;
}

const std::string& parseFramesStr()
{
    static const std::string Str("frames");
    return Str;
}

const std::string& parseIntStr()
{
    static const std::string Str("int");
    return Str;
}

const std::string& parseFloatStr()
{
    static const std::string Str("float");
    return Str;
}

const std::string& parseTypeStr()
{
    static const std::string Str("type");
    return Str;
}

const std::string& parseDefaultValueStr()
{
    static const std::string Str("defaultValue");
    return Str;
}

const std::string& parseUnitsStr()
{
    static const std::string Str("units");
    return Str;
}

const std::string& parseScalingStr()
{
    static const std::string Str("scaling");
    return Str;
}

const std::string& parseLengthStr()
{
    static const std::string Str("length");
    return Str;
}

const std::string& parseBitLengthStr()
{
    static const std::string Str("bitLength");
    return Str;
}

const std::string& parseSerOffparseSetStr()
{
    static const std::string Str("serOffset");
    return Str;
}

const std::string& parseValidRangeStr()
{
    static const std::string Str("validRange");
    return Str;
}

const std::string& parseValidFullRangeStr()
{
    static const std::string Str("validFullRange");
    return Str;
}

const std::string& parseValidValueStr()
{
    static const std::string Str("validValue");
    return Str;
}

const std::string& parseSpecialStr()
{
    static const std::string Str("special");
    return Str;
}

const std::string& parseValStr()
{
    static const std::string Str("val");
    return Str;
}

const std::string& parseMetaStr()
{
    static const std::string Str("meta");
    return Str;
}

const std::string& parseValidMinStr()
{
    static const std::string Str("validMin");
    return Str;
}

const std::string& parseValidMaxStr()
{
    static const std::string Str("validMax");
    return Str;
}

const std::string& parseNanStr()
{
    static const std::string Str("nan");
    return Str;
}

const std::string& parseInfStr()
{
    static const std::string Str("inf");
    return Str;
}

const std::string& parseNegInfStr()
{
    static const std::string Str("-inf");
    return Str;
}

const std::string& parseBitparseFieldStr()
{
    static const std::string Str("bitfield");
    return Str;
}

const std::string& parseBundleStr()
{
    static const std::string Str("bundle");
    return Str;
}

const std::string& parseVariantStr()
{
    static const std::string Str("variant");
    return Str;
}

const std::string& parseMembersStr()
{
    static const std::string Str("members");
    return Str;
}

const std::string& parseSinceVersionStr()
{
    static const std::string Str("sinceVersion");
    return Str;
}

const std::string& parseDeprecatedStr()
{
    static const std::string Str("deprecated");
    return Str;
}

const std::string& parseRemovedStr()
{
    static const std::string Str("removed");
    return Str;
}

const std::string& parseRefStr()
{
    static const std::string Str("ref");
    return Str;
}

const std::string& parseFieldStr()
{
    static const std::string Str("field");
    return Str;
}

const std::string& parseNsStr()
{
    static const std::string Str("ns");
    return Str;
}

const std::string& parseEnumStr()
{
    static const std::string Str("enum");
    return Str;
}

const std::string& parseNonUniqueAllowedStr()
{
    static const std::string Str("nonUniqueAllowed");
    return Str;
}

const std::string& parseNonUniqueSpecialsAllowedStr()
{
    static const std::string Str("nonUniqueSpecialsAllowed");
    return Str;
}

const std::string& parseNonUniqueMsgIdAllowedStr()
{
    static const std::string Str("nonUniqueMsgIdAllowed");
    return Str;
}

const std::string& parseReservedValueStr()
{
    static const std::string Str("reservedValue");
    return Str;
}

const std::string& parseReservedStr()
{
    static const std::string Str("reserved");
    return Str;
}

const std::string& parseBitStr()
{
    static const std::string Str("bit");
    return Str;
}

const std::string& parseIdxStr()
{
    static const std::string Str("idx");
    return Str;
}

const std::string& parseSetStr()
{
    static const std::string Str("set");
    return Str;
}

const std::string& parseReuseStr()
{
    static const std::string Str("reuse");
    return Str;
}

const std::string& parseValidCheckVersionStr()
{
    static const std::string Str("validCheckVersion");
    return Str;
}

const std::string& parseLengthPrefixStr()
{
    static const std::string Str("lengthPrefix");
    return Str;
}

const std::string& parseCountPrefixStr()
{
    static const std::string Str("countPrefix");
    return Str;
}

const std::string& parseEncodingStr()
{
    static const std::string Str("encoding");
    return Str;
}

const std::string& parseZeroTermSuffixStr()
{
    static const std::string Str("zeroTermSuffix");
    return Str;
}

const std::string& parseStringStr()
{
    static const std::string Str("string");
    return Str;
}

const std::string& parseDataStr()
{
    static const std::string Str("data");
    return Str;
}

const std::string& parseCountStr()
{
    static const std::string Str("count");
    return Str;
}

const std::string& parseElemFixedLengthStr()
{
    static const std::string Str("elemFixedLength");
    return Str;
}

const std::string& parseElementStr()
{
    static const std::string Str("element");
    return Str;
}

const std::string& parseElemLengthPrefixStr()
{
    static const std::string Str("elemLengthPrefix");
    return Str;
}

const std::string& parseListStr()
{
    static const std::string Str("list");
    return Str;
}

const std::string& parseOptionalStr()
{
    static const std::string Str("optional");
    return Str;
}

const std::string& parseDefaultModeStr()
{
    static const std::string Str("defaultMode");
    return Str;
}

const std::string& parseCondStr()
{
    static const std::string Str("cond");
    return Str;
}

const std::string& parseAndStr()
{
    static const std::string Str("and");
    return Str;
}

const std::string& parseOrStr()
{
    static const std::string Str("or");
    return Str;
}

const std::string& parseCopyFieldsFromStr()
{
    static const std::string Str("copyFieldsFrom");
    return Str;
}

const std::string& parseCopyFieldsAliasesStr()
{
    static const std::string Str("copyFieldsAliases");
    return Str;
}

const std::string& parseOrderStr()
{
    static const std::string Str("order");
    return Str;
}

const std::string& parsePlatformsStr()
{
    static const std::string Str("platforms");
    return Str;
}

const std::string& parsePlatformStr()
{
    static const std::string Str("platform");
    return Str;
}

const std::string& parseInterfacesStr()
{
    static const std::string Str("interfaces");
    return Str;
}

const std::string& parseInterfaceStr()
{
    static const std::string Str("interface");
    return Str;
}

const std::string& parseLayersStr()
{
    static const std::string Str("layers");
    return Str;
}

const std::string& parsePayloadStr()
{
    static const std::string Str("payload");
    return Str;
}

const std::string& parseSizeStr()
{
    static const std::string Str("size");
    return Str;
}

const std::string& parseSyncStr()
{
    static const std::string Str("sync");
    return Str;
}

const std::string& parseChecksumStr()
{
    static const std::string Str("checksum");
    return Str;
}

const std::string& parseAlgStr()
{
    static const std::string Str("alg");
    return Str;
}

const std::string& parseAlgNameStr()
{
    static const std::string Str("algName");
    return Str;
}

const std::string& parseFromStr()
{
    static const std::string Str("from");
    return Str;
}

const std::string& parseUntilStr()
{
    static const std::string Str("until");
    return Str;
}

const std::string& parseVerifyBeforeReadStr()
{
    static const std::string Str("verifyBeforeRead");
    return Str;
}

const std::string& parseInterfaceFieldNameStr()
{
    static const std::string Str("interfaceFieldName");
    return Str;
}

const std::string& parseValueStr()
{
    static const std::string Str("value");
    return Str;
}

const std::string& pseudoStr()
{
    static const std::string Str("pseudo");
    return Str;
}

const std::string& parseFixedValueStr()
{
    static const std::string Str("fixedValue");
    return Str;
}

const std::string& parseCustomStr()
{
    static const std::string Str("custom");
    return Str;
}

const std::string& parseSemanticTypeStr()
{
    static const std::string Str("semanticType");
    return Str;
}

const std::string& parseNoneStr()
{
    static const std::string Str("none");
    return Str;
}

const std::string& parseMessageIdStr()
{
    static const std::string Str("messageId");
    return Str;
}

const std::string& parseIdReplacementStr()
{
    static const std::string Str("idReplacement");
    return Str;
}

const std::string& parseDisplayDesimalsStr()
{
    static const std::string Str("displayDecimals");
    return Str;
}

const std::string& parseDisplayOffsetStr()
{
    static const std::string Str("displayOffset");
    return Str;
}

const std::string& parseDisplayExtModeCtrlStr()
{
    static const std::string Str("displayExtModeCtrl");
    return Str;
}

const std::string& parseHexAssignStr()
{
    static const std::string Str("hexAssign");
    return Str;
}

const std::string& parseSignExtStr()
{
    static const std::string Str("signExt");
    return Str;
}

const std::string& parseDisplayReadOnlyStr()
{
    static const std::string Str("displayReadOnly");
    return Str;
}

const std::string& parseDisplayHiddenStr()
{
    static const std::string Str("displayHidden");
    return Str;
}

const std::string& parseCustomizableStr()
{
    static const std::string Str("customizable");
    return Str;
}

const std::string& parseFailOnInvalidStr()
{
    static const std::string Str("failOnInvalid");
    return Str;
}

const std::string& parseSenderStr()
{
    static const std::string Str("sender");
    return Str;
}

const std::string& parseClientStr()
{
    static const std::string Str("client");
    return Str;
}

const std::string& parseServerStr()
{
    static const std::string Str("server");
    return Str;
}

const std::string& parseBothStr()
{
    static const std::string Str("both");
    return Str;
}

const std::string& parseDefaultMemberStr()
{
    static const std::string Str("defaultMember");
    return Str;
}

const std::string& parseDisplayIdxReadOnlyHiddenStr()
{
    static const std::string Str("displayIdxReadOnlyHidden");
    return Str;
}

const std::string& parseDisplaySpecialsStr()
{
    static const std::string Str("displaySpecials");
    return Str;
}

const std::string& parseAliasStr()
{
    static const std::string Str("alias");
    return Str;
}

const std::string& parseReuseAliasesStr()
{
    static const std::string Str("reuseAliases");
    return Str;
}

const std::string& parseForceGenStr()
{
    static const std::string Str("forceGen");
    return Str;    
}

const std::string& parseValidateMinLengthStr()
{
    static const std::string Str("validateMinLength");
    return Str;    
}

const std::string& parseDefaultValidValueStr()
{
    static const std::string Str("defaultValidValue");
    return Str;      
}

const std::string& parseAvailableLengthLimitStr()
{
    static const std::string Str("availableLengthLimit");
    return Str; 
}

const std::string& parseValueOverrideStr()
{
    static const std::string Str("valueOverride");
    return Str; 
}

const std::string& parseReadOverrideStr()
{
    static const std::string Str("readOverride");
    return Str; 
}

const std::string& parseWriteOverrideStr()
{
    static const std::string Str("writeOverride");
    return Str; 
}

const std::string& parseRefreshOverrideStr()
{
    static const std::string Str("refreshOverride");
    return Str; 
}

const std::string& parseLengthOverrideStr()
{
    static const std::string Str("lengthOverride");
    return Str; 
}

const std::string& parseValidOverrideStr()
{
    static const std::string Str("validOverride");
    return Str; 
}

const std::string& parseNameOverrideStr()
{
    static const std::string Str("nameOverride");
    return Str; 
}

const std::string& parseReplaceStr()
{
    static const std::string Str("replace");
    return Str; 
}

const std::string& parseCopyCodeFromStr()
{
    static const std::string Str("copyCodeFrom");
    return Str; 
}

const std::string& parseSemanticLayerTypeStr()
{
    static const std::string Str("semanticLayerType");
    return Str; 
}

const std::string& parseChecksumFromStr()
{
    static const std::string Str("checksumFrom");
    return Str; 
}

const std::string& parseChecksumUntilStr()
{
    static const std::string Str("checksumUntil");
    return Str; 
}

const std::string& parseTermSuffixStr()
{
    static const std::string Str("termSuffix");
    return Str;     
}

const std::string& parseMissingOnReadFailStr()
{
    static const std::string Str("missingOnReadFail");
    return Str; 
}

const std::string& parseMissingOnInvalparseIdStr()
{
    static const std::string Str("missingOnInvalid");
    return Str; 
}

const std::string& parseReuseCodeStr()
{
    static const std::string Str("reuseCode");
    return Str;    
}

const std::string& parseConstructStr()
{
    static const std::string Str("construct");
    return Str;    
}

const std::string& parseReadCondStr()
{
    static const std::string Str("readCond");
    return Str;    
}

const std::string& parseValidCondStr()
{
    static const std::string Str("validCond");
    return Str;    
}

const std::string& parseConstructAsReadCondStr()
{
    static const std::string Str("constructAsReadCond");
    return Str;       
}

const std::string& parseConstructAsValidCondStr()
{
    static const std::string Str("constructAsValidCond");
    return Str;       
}

const std::string& parseCopyConstructFromStr()
{
    static const std::string Str("copyConstructFrom");
    return Str;      
}

const std::string& parseCopyReadCondFromStr()
{
    static const std::string Str("copyReadCondFrom");
    return Str;      
}

const std::string& parseCopyValidCondFromStr()
{
    static const std::string Str("copyValidCondFrom");
    return Str;      
}

char parseSiblingRefPrefix()
{
    return '$';
}

char parseStringRefPrefix()
{
    return '^';
}

char parseSchemaRefPrefix()
{
    return '@';
}

char parseInterfaceRefPrefix()
{
    return '%';
}

unsigned parseStrToUnsigned(const std::string& str, bool* ok, int base)
{
    unsigned result = 0U;
    try {
        result = static_cast<decltype(result)>(std::stoul(str, 0, base));
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

std::intmax_t parseStrToIntMax(const std::string& str, bool* ok, int base)
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

std::uintmax_t parseStrToUintMax(const std::string& str, bool* ok, int base)
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

double parseStrToDouble(const std::string& str, bool* ok, bool allowSpecials)
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
            std::make_pair(parseNanStr(), std::numeric_limits<double>::quiet_NaN()),
            std::make_pair(parseInfStr(), std::numeric_limits<double>::infinity()),
            std::make_pair(parseNegInfStr(), -(std::numeric_limits<double>::infinity()))
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

bool parseStrToBool(const std::string& str, bool* ok)
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

    auto strCopy = parseToLowerCopy(str);
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

bool parseIsFpSpecial(const std::string& str)
{
    static const std::string Map[] = {
        parseNanStr(),
        parseInfStr(),
        parseNegInfStr()
    };

    auto iter = std::find(std::begin(Map), std::end(Map), str);
    return iter != std::end(Map);
}

ParseUnits parseStrToUnits(const std::string& str, bool* ok)
{
    static const std::map<std::string, ParseUnits> Map = {
        std::make_pair(parseEmptyString(), ParseUnits::Unknown),
        std::make_pair("ns", ParseUnits::Nanoseconds),
        std::make_pair("nanosec", ParseUnits::Nanoseconds),
        std::make_pair("nanosecs", ParseUnits::Nanoseconds),
        std::make_pair("nanosecond", ParseUnits::Nanoseconds),
        std::make_pair("nanoseconds", ParseUnits::Nanoseconds),
        std::make_pair("us", ParseUnits::Microseconds),
        std::make_pair("microsec", ParseUnits::Microseconds),
        std::make_pair("microsecs", ParseUnits::Microseconds),
        std::make_pair("microsecond", ParseUnits::Microseconds),
        std::make_pair("microseconds", ParseUnits::Microseconds),
        std::make_pair("ms", ParseUnits::Milliseconds),
        std::make_pair("millisec", ParseUnits::Milliseconds),
        std::make_pair("millisecs", ParseUnits::Milliseconds),
        std::make_pair("millisecond", ParseUnits::Milliseconds),
        std::make_pair("milliseconds", ParseUnits::Milliseconds),
        std::make_pair("s", ParseUnits::Seconds),
        std::make_pair("sec", ParseUnits::Seconds),
        std::make_pair("secs", ParseUnits::Seconds),
        std::make_pair("second", ParseUnits::Seconds),
        std::make_pair("seconds", ParseUnits::Seconds),
        std::make_pair("min", ParseUnits::Minutes),
        std::make_pair("mins", ParseUnits::Minutes),
        std::make_pair("minute", ParseUnits::Minutes),
        std::make_pair("minutes", ParseUnits::Minutes),
        std::make_pair("h", ParseUnits::Hours),
        std::make_pair("hour", ParseUnits::Hours),
        std::make_pair("hours", ParseUnits::Hours),
        std::make_pair("d", ParseUnits::Days),
        std::make_pair("day", ParseUnits::Days),
        std::make_pair("days", ParseUnits::Days),
        std::make_pair("w", ParseUnits::Weeks),
        std::make_pair("week", ParseUnits::Weeks),
        std::make_pair("weeks", ParseUnits::Weeks),
        std::make_pair("nm", ParseUnits::Nanometers),
        std::make_pair("nanometer", ParseUnits::Nanometers),
        std::make_pair("nanometre", ParseUnits::Nanometers),
        std::make_pair("nanometres", ParseUnits::Nanometers),
        std::make_pair("nanometers", ParseUnits::Nanometers),
        std::make_pair("um", ParseUnits::Micrometers),
        std::make_pair("micrometer", ParseUnits::Micrometers),
        std::make_pair("micrometre", ParseUnits::Micrometers),
        std::make_pair("micrometres", ParseUnits::Micrometers),
        std::make_pair("micrometers", ParseUnits::Micrometers),
        std::make_pair("mm", ParseUnits::Millimeters),
        std::make_pair("millimeter", ParseUnits::Millimeters),
        std::make_pair("millimetre", ParseUnits::Millimeters),
        std::make_pair("millimetres", ParseUnits::Millimeters),
        std::make_pair("millimeters", ParseUnits::Millimeters),
        std::make_pair("cm", ParseUnits::Centimeters),
        std::make_pair("centimeter", ParseUnits::Centimeters),
        std::make_pair("centimetre", ParseUnits::Centimeters),
        std::make_pair("centimetres", ParseUnits::Centimeters),
        std::make_pair("centimeters", ParseUnits::Centimeters),
        std::make_pair("m", ParseUnits::Meters),
        std::make_pair("meter", ParseUnits::Meters),
        std::make_pair("metre", ParseUnits::Meters),
        std::make_pair("metres", ParseUnits::Meters),
        std::make_pair("meters", ParseUnits::Meters),
        std::make_pair("km", ParseUnits::Kilometers),
        std::make_pair("kilometer", ParseUnits::Kilometers),
        std::make_pair("kilometre", ParseUnits::Kilometers),
        std::make_pair("kilometres", ParseUnits::Kilometers),
        std::make_pair("kilometers", ParseUnits::Kilometers),
        std::make_pair("nm/s", ParseUnits::NanometersPerSecond),
        std::make_pair("nmps", ParseUnits::NanometersPerSecond),
        std::make_pair("nanometer/second", ParseUnits::NanometersPerSecond),
        std::make_pair("nanometre/second", ParseUnits::NanometersPerSecond),
        std::make_pair("nanometers/second", ParseUnits::NanometersPerSecond),
        std::make_pair("nanometres/second", ParseUnits::NanometersPerSecond),
        std::make_pair("um/s", ParseUnits::MicrometersPerSecond),
        std::make_pair("umps", ParseUnits::MicrometersPerSecond),
        std::make_pair("micrometers/second", ParseUnits::MicrometersPerSecond),
        std::make_pair("micrometres/second", ParseUnits::MicrometersPerSecond),
        std::make_pair("micrometer/second", ParseUnits::MicrometersPerSecond),
        std::make_pair("micrometre/second", ParseUnits::MicrometersPerSecond),
        std::make_pair("mm/s", ParseUnits::MillimetersPerSecond),
        std::make_pair("mmps", ParseUnits::MillimetersPerSecond),
        std::make_pair("millimeter/second", ParseUnits::MillimetersPerSecond),
        std::make_pair("millimetre/second", ParseUnits::MillimetersPerSecond),
        std::make_pair("millimeters/second", ParseUnits::MillimetersPerSecond),
        std::make_pair("millimetres/second", ParseUnits::MillimetersPerSecond),
        std::make_pair("cm/s", ParseUnits::CentimetersPerSecond),
        std::make_pair("cmps", ParseUnits::CentimetersPerSecond),
        std::make_pair("centimeter/second", ParseUnits::CentimetersPerSecond),
        std::make_pair("centimetre/second", ParseUnits::CentimetersPerSecond),
        std::make_pair("centimeters/second", ParseUnits::CentimetersPerSecond),
        std::make_pair("centimetres/second", ParseUnits::CentimetersPerSecond),
        std::make_pair("m/s", ParseUnits::MetersPerSecond),
        std::make_pair("mps", ParseUnits::MetersPerSecond),
        std::make_pair("meter/second", ParseUnits::MetersPerSecond),
        std::make_pair("metre/second", ParseUnits::MetersPerSecond),
        std::make_pair("meters/second", ParseUnits::MetersPerSecond),
        std::make_pair("metres/second", ParseUnits::MetersPerSecond),
        std::make_pair("km/s", ParseUnits::KilometersPerSecond),
        std::make_pair("kmps", ParseUnits::KilometersPerSecond),
        std::make_pair("kps", ParseUnits::KilometersPerSecond),
        std::make_pair("kilometer/second", ParseUnits::KilometersPerSecond),
        std::make_pair("kilometre/second", ParseUnits::KilometersPerSecond),
        std::make_pair("kilometers/second", ParseUnits::KilometersPerSecond),
        std::make_pair("kilometres/second", ParseUnits::KilometersPerSecond),
        std::make_pair("km/h", ParseUnits::KilometersPerHour),
        std::make_pair("kmph", ParseUnits::KilometersPerHour),
        std::make_pair("kph", ParseUnits::KilometersPerHour),
        std::make_pair("kilometer/hour", ParseUnits::KilometersPerHour),
        std::make_pair("kilometre/hour", ParseUnits::KilometersPerHour),
        std::make_pair("kilometers/hour", ParseUnits::KilometersPerHour),
        std::make_pair("kilometres/hour", ParseUnits::KilometersPerHour),
        std::make_pair("hz", ParseUnits::Hertz),
        std::make_pair("hertz", ParseUnits::Hertz),
        std::make_pair("khz", ParseUnits::KiloHertz),
        std::make_pair("kilohertz", ParseUnits::KiloHertz),
        std::make_pair("mhz", ParseUnits::MegaHertz),
        std::make_pair("megahertz", ParseUnits::MegaHertz),
        std::make_pair("ghz", ParseUnits::GigaHertz),
        std::make_pair("gigahertz", ParseUnits::GigaHertz),
        std::make_pair("deg", ParseUnits::Degrees),
        std::make_pair("degree", ParseUnits::Degrees),
        std::make_pair("degrees", ParseUnits::Degrees),
        std::make_pair("rad", ParseUnits::Radians),
        std::make_pair("radian", ParseUnits::Radians),
        std::make_pair("radians", ParseUnits::Radians),
        std::make_pair("na", ParseUnits::Nanoamps),
        std::make_pair("nanoamp", ParseUnits::Nanoamps),
        std::make_pair("nanoamps", ParseUnits::Nanoamps),
        std::make_pair("nanoampere", ParseUnits::Nanoamps),
        std::make_pair("nanoamperes", ParseUnits::Nanoamps),
        std::make_pair("ua", ParseUnits::Microamps),
        std::make_pair("microamp", ParseUnits::Microamps),
        std::make_pair("microamps", ParseUnits::Microamps),
        std::make_pair("microampere", ParseUnits::Microamps),
        std::make_pair("microamperes", ParseUnits::Microamps),
        std::make_pair("ma", ParseUnits::Milliamps),
        std::make_pair("milliamp", ParseUnits::Milliamps),
        std::make_pair("milliamps", ParseUnits::Milliamps),
        std::make_pair("milliampere", ParseUnits::Milliamps),
        std::make_pair("milliamperes", ParseUnits::Milliamps),
        std::make_pair("a", ParseUnits::Amps),
        std::make_pair("amp", ParseUnits::Amps),
        std::make_pair("amps", ParseUnits::Amps),
        std::make_pair("ampere", ParseUnits::Amps),
        std::make_pair("amperes", ParseUnits::Amps),
        std::make_pair("ka", ParseUnits::Kiloamps),
        std::make_pair("kiloamp", ParseUnits::Kiloamps),
        std::make_pair("kiloamps", ParseUnits::Kiloamps),
        std::make_pair("kiloampere", ParseUnits::Kiloamps),
        std::make_pair("kiloamperes", ParseUnits::Kiloamps),
        std::make_pair("nv", ParseUnits::Nanovolts),
        std::make_pair("nanovolt", ParseUnits::Nanovolts),
        std::make_pair("nanovolts", ParseUnits::Nanovolts),
        std::make_pair("uv", ParseUnits::Microvolts),
        std::make_pair("microvolt", ParseUnits::Microvolts),
        std::make_pair("microvolts", ParseUnits::Microvolts),
        std::make_pair("mv", ParseUnits::Millivolts),
        std::make_pair("millivolt", ParseUnits::Millivolts),
        std::make_pair("millivolts", ParseUnits::Millivolts),
        std::make_pair("v", ParseUnits::Volts),
        std::make_pair("volt", ParseUnits::Volts),
        std::make_pair("volts", ParseUnits::Volts),
        std::make_pair("kv", ParseUnits::Kilovolts),
        std::make_pair("kilovolt", ParseUnits::Kilovolts),
        std::make_pair("kilovolts", ParseUnits::Kilovolts),
        std::make_pair("b", ParseUnits::Bytes),
        std::make_pair("byte", ParseUnits::Bytes),
        std::make_pair("bytes", ParseUnits::Bytes),
        std::make_pair("kb", ParseUnits::Kilobytes),
        std::make_pair("kilobyte", ParseUnits::Kilobytes),
        std::make_pair("kilobytes", ParseUnits::Kilobytes),
        std::make_pair("mb", ParseUnits::Megabytes),
        std::make_pair("megabyte", ParseUnits::Megabytes),
        std::make_pair("megabytes", ParseUnits::Megabytes),        
        std::make_pair("gb", ParseUnits::Gigabytes),
        std::make_pair("gigabyte", ParseUnits::Gigabytes),
        std::make_pair("gigabytes", ParseUnits::Gigabytes),
        std::make_pair("tb", ParseUnits::Terabytes),
        std::make_pair("terabyte", ParseUnits::Terabytes),
        std::make_pair("terabytes", ParseUnits::Terabytes),
    };


    auto updateOkFunc =
        [ok](bool result)
        {
            if (ok != nullptr) {
                *ok = result;
            }
        };

    auto strToLook = parseToLowerCopy(str);
    strToLook.erase(
        std::remove_if(
            strToLook.begin(), strToLook.end(),
            [](char ch)
            {
                return (ch == ' ') || (ch == '\t');
            }),
        strToLook.end());

    auto iter = Map.find(strToLook);
    if (iter == Map.end()) {
        updateOkFunc(false);
        return ParseUnits::Unknown;
    }

    updateOkFunc(true);
    return iter->second;
}

const std::string& parseGetStringProp(
    const PropsMap& map,
    const std::string& prop,
    const std::string& defaultValue)
{
    auto iter = map.lower_bound(prop);
    if ((iter == map.end()) || (iter->first != prop)) {
        return defaultValue;
    }

    return iter->second;
}

commsdsl::parse::ParseEndian parseEndian(const std::string& value, commsdsl::parse::ParseEndian defaultEndian)
{
    if (value.empty()) {
        return defaultEndian;
    }

    static const std::string Map[] = {
        /* ParseEndian_Little */ common::parseLittleStr(),
        /* Endian_Big */ common::parseBigStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == ParseEndian_NumOfValues, "Invalid map");

    auto valueCpy = parseToLowerCopy(value);
    auto mapIter = std::find(std::begin(Map), std::end(Map), valueCpy);
    if (mapIter == std::end(Map)) {
        return ParseEndian_NumOfValues;
    }

    return static_cast<ParseEndian>(std::distance(std::begin(Map), mapIter));
}

void parseToLower(std::string& str)
{
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
}

std::string parseToLowerCopy(const std::string& str)
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

void parseRemoveHeadingTrailingWhitespaces(std::string& str)
{
    static const std::string WhiteSpaces(" \r\n\t");
    auto startPos = str.find_first_not_of(WhiteSpaces);
    if (startPos == std::string::npos) {
        str.clear();
        return;
    }

    if (startPos != 0U) {
        str.erase(str.begin(), str.begin() + startPos);
    }

    assert(!str.empty());
    auto endPos = str.find_last_not_of(WhiteSpaces);
    assert(endPos != std::string::npos);
    if (endPos == (str.size() - 1U)) {
        return;
    }

    str.erase(str.begin() + endPos + 1, str.end());
}

void parseNormaliseString(std::string& str)
{
    static const std::string ReplaceChars("\t\r\n");
    for (auto& ch : str) {
        auto iter = std::find(ReplaceChars.begin(), ReplaceChars.end(), ch);
        if (iter == ReplaceChars.end()) {
            continue;
        }

        ch = ' ';
    }

    bool removing = false;
    str.erase(
        std::remove_if(
            str.begin(), str.end(),
            [&removing](char ch)
            {
                if (ch != ' ') {
                    removing = false;
                    return false;
                }

                if (removing) {
                    return true;
                }

                removing = true;
                return false;
            }),
        str.end());
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

bool parseIsValidName(const std::string& value)
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

bool parseIsValidRefName(const char* buf, std::size_t len)
{
    if ((0U < len) && (buf[0] == parseSchemaRefPrefix())) {
        // Allow first character to be interschema ref
        ++buf;
        --len;
    } 

    if (len == 0U) {
        return false;
    }       

    if ((std::isalpha(buf[0]) == 0) && (buf[0] != '_')) {
        return false;
    }

    auto endBuf = buf + len;
    bool validChars =
        std::all_of(
            buf, endBuf,
            [](char ch)
            {
                return (std::isalnum(ch) != 0) || (ch == '_') || (ch == '.');
            });
    if (!validChars) {
        return false;
    }

    if (buf[len - 1] == '.') {
        return false;
    }

    auto iter = std::find(buf, endBuf, '.');
    while (iter != endBuf) {
        auto nextPosIter = iter + 1;
        assert(nextPosIter < endBuf);
        auto nextIter = std::find(nextPosIter, endBuf, '.');
        if (nextIter == nextPosIter) {
            return false; // sequential dots without name in the middle
        }
        iter = nextIter;
    }

    return true;
}

bool parseIsValidRefName(const std::string& value)
{
    return parseIsValidRefName(value.c_str(), value.size());
}

bool parseIsValidExternalRefName(const std::string& value)
{
    if (value.size() <= 1U) {
        return false;
    }

    if (value[0] != parseStringRefPrefix()) {
        return false;
    }

    return parseIsValidRefName(&value[1], value.size() - 1U);
}

std::size_t parseMaxPossibleLength()
{
    return MaxPossibleLength;
}

void parseAddToLength(std::size_t newLen, std::size_t& accLen)
{
    if ((MaxPossibleLength - accLen) <= newLen) {
        accLen = MaxPossibleLength;
        return;
    }

    accLen += newLen;
}

std::size_t parseMulLength(std::size_t len, std::size_t factor)
{
    if ((((MaxPossibleLength - 1) / factor) + 1) <= len) {
        return MaxPossibleLength;
    }

    return len * factor;
}

} // namespace common

} // namespace parse

} // namespace commsdsl
