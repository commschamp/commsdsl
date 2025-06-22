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

const std::string& dslVersionStr()
{
    static const std::string Str("dslVersion");
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

const std::string& variantStr()
{
    static const std::string Str("variant");
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

const std::string& removedStr()
{
    static const std::string Str("removed");
    return Str;
}

const std::string& refStr()
{
    static const std::string Str("ref");
    return Str;
}

const std::string& fieldStr()
{
    static const std::string Str("field");
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

const std::string& nonUniqueSpecialsAllowedStr()
{
    static const std::string Str("nonUniqueSpecialsAllowed");
    return Str;
}

const std::string& nonUniqueMsgIdAllowedStr()
{
    static const std::string Str("nonUniqueMsgIdAllowed");
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

const std::string& validCheckVersionStr()
{
    static const std::string Str("validCheckVersion");
    return Str;
}

const std::string& lengthPrefixStr()
{
    static const std::string Str("lengthPrefix");
    return Str;
}

const std::string& countPrefixStr()
{
    static const std::string Str("countPrefix");
    return Str;
}

const std::string& encodingStr()
{
    static const std::string Str("encoding");
    return Str;
}

const std::string& zeroTermSuffixStr()
{
    static const std::string Str("zeroTermSuffix");
    return Str;
}

const std::string& stringStr()
{
    static const std::string Str("string");
    return Str;
}

const std::string& dataStr()
{
    static const std::string Str("data");
    return Str;
}

const std::string& countStr()
{
    static const std::string Str("count");
    return Str;
}

const std::string& elemFixedLengthStr()
{
    static const std::string Str("elemFixedLength");
    return Str;
}

const std::string& elementStr()
{
    static const std::string Str("element");
    return Str;
}

const std::string& elemLengthPrefixStr()
{
    static const std::string Str("elemLengthPrefix");
    return Str;
}

const std::string& listStr()
{
    static const std::string Str("list");
    return Str;
}

const std::string& optionalStr()
{
    static const std::string Str("optional");
    return Str;
}

const std::string& defaultModeStr()
{
    static const std::string Str("defaultMode");
    return Str;
}

const std::string& condStr()
{
    static const std::string Str("cond");
    return Str;
}

const std::string& andStr()
{
    static const std::string Str("and");
    return Str;
}

const std::string& orStr()
{
    static const std::string Str("or");
    return Str;
}

const std::string& copyFieldsFromStr()
{
    static const std::string Str("copyFieldsFrom");
    return Str;
}

const std::string& copyFieldsAliasesStr()
{
    static const std::string Str("copyFieldsAliases");
    return Str;
}

const std::string& orderStr()
{
    static const std::string Str("order");
    return Str;
}

const std::string& platformsStr()
{
    static const std::string Str("platforms");
    return Str;
}

const std::string& platformStr()
{
    static const std::string Str("platform");
    return Str;
}

const std::string& interfacesStr()
{
    static const std::string Str("interfaces");
    return Str;
}

const std::string& interfaceStr()
{
    static const std::string Str("interface");
    return Str;
}

const std::string& layersStr()
{
    static const std::string Str("layers");
    return Str;
}

const std::string& payloadStr()
{
    static const std::string Str("payload");
    return Str;
}

const std::string& sizeStr()
{
    static const std::string Str("size");
    return Str;
}

const std::string& syncStr()
{
    static const std::string Str("sync");
    return Str;
}

const std::string& checksumStr()
{
    static const std::string Str("checksum");
    return Str;
}

const std::string& algStr()
{
    static const std::string Str("alg");
    return Str;
}

const std::string& algNameStr()
{
    static const std::string Str("algName");
    return Str;
}

const std::string& fromStr()
{
    static const std::string Str("from");
    return Str;
}

const std::string& untilStr()
{
    static const std::string Str("until");
    return Str;
}

const std::string& verifyBeforeReadStr()
{
    static const std::string Str("verifyBeforeRead");
    return Str;
}

const std::string& interfaceFieldNameStr()
{
    static const std::string Str("interfaceFieldName");
    return Str;
}

const std::string& valueStr()
{
    static const std::string Str("value");
    return Str;
}

const std::string& pseudoStr()
{
    static const std::string Str("pseudo");
    return Str;
}

const std::string& fixedValueStr()
{
    static const std::string Str("fixedValue");
    return Str;
}

const std::string& customStr()
{
    static const std::string Str("custom");
    return Str;
}

const std::string& semanticTypeStr()
{
    static const std::string Str("semanticType");
    return Str;
}

const std::string& noneStr()
{
    static const std::string Str("none");
    return Str;
}

const std::string& messageIdStr()
{
    static const std::string Str("messageId");
    return Str;
}

const std::string& idReplacementStr()
{
    static const std::string Str("idReplacement");
    return Str;
}

const std::string& displayDesimalsStr()
{
    static const std::string Str("displayDecimals");
    return Str;
}

const std::string& displayOffsetStr()
{
    static const std::string Str("displayOffset");
    return Str;
}

const std::string& displayExtModeCtrlStr()
{
    static const std::string Str("displayExtModeCtrl");
    return Str;
}

const std::string& hexAssignStr()
{
    static const std::string Str("hexAssign");
    return Str;
}

const std::string& signExtStr()
{
    static const std::string Str("signExt");
    return Str;
}

const std::string& displayReadOnlyStr()
{
    static const std::string Str("displayReadOnly");
    return Str;
}

const std::string& displayHiddenStr()
{
    static const std::string Str("displayHidden");
    return Str;
}

const std::string& customizableStr()
{
    static const std::string Str("customizable");
    return Str;
}

const std::string& failOnInvalidStr()
{
    static const std::string Str("failOnInvalid");
    return Str;
}

const std::string& senderStr()
{
    static const std::string Str("sender");
    return Str;
}

const std::string& clientStr()
{
    static const std::string Str("client");
    return Str;
}

const std::string& serverStr()
{
    static const std::string Str("server");
    return Str;
}

const std::string& bothStr()
{
    static const std::string Str("both");
    return Str;
}

const std::string& defaultMemberStr()
{
    static const std::string Str("defaultMember");
    return Str;
}

const std::string& displayIdxReadOnlyHiddenStr()
{
    static const std::string Str("displayIdxReadOnlyHidden");
    return Str;
}

const std::string& displaySpecialsStr()
{
    static const std::string Str("displaySpecials");
    return Str;
}

const std::string& aliasStr()
{
    static const std::string Str("alias");
    return Str;
}

const std::string& reuseAliasesStr()
{
    static const std::string Str("reuseAliases");
    return Str;
}

const std::string& forceGenStr()
{
    static const std::string Str("forceGen");
    return Str;    
}

const std::string& validateMinLengthStr()
{
    static const std::string Str("validateMinLength");
    return Str;    
}

const std::string& defaultValidValueStr()
{
    static const std::string Str("defaultValidValue");
    return Str;      
}

const std::string& availableLengthLimitStr()
{
    static const std::string Str("availableLengthLimit");
    return Str; 
}

const std::string& valueOverrideStr()
{
    static const std::string Str("valueOverride");
    return Str; 
}

const std::string& readOverrideStr()
{
    static const std::string Str("readOverride");
    return Str; 
}

const std::string& writeOverrideStr()
{
    static const std::string Str("writeOverride");
    return Str; 
}

const std::string& refreshOverrideStr()
{
    static const std::string Str("refreshOverride");
    return Str; 
}

const std::string& lengthOverrideStr()
{
    static const std::string Str("lengthOverride");
    return Str; 
}

const std::string& validOverrideStr()
{
    static const std::string Str("validOverride");
    return Str; 
}

const std::string& nameOverrideStr()
{
    static const std::string Str("nameOverride");
    return Str; 
}

const std::string& replaceStr()
{
    static const std::string Str("replace");
    return Str; 
}

const std::string& copyCodeFromStr()
{
    static const std::string Str("copyCodeFrom");
    return Str; 
}

const std::string& semanticLayerTypeStr()
{
    static const std::string Str("semanticLayerType");
    return Str; 
}

const std::string& checksumFromStr()
{
    static const std::string Str("checksumFrom");
    return Str; 
}

const std::string& checksumUntilStr()
{
    static const std::string Str("checksumUntil");
    return Str; 
}

const std::string& termSuffixStr()
{
    static const std::string Str("termSuffix");
    return Str;     
}

const std::string& missingOnReadFailStr()
{
    static const std::string Str("missingOnReadFail");
    return Str; 
}

const std::string& missingOnInvalidStr()
{
    static const std::string Str("missingOnInvalid");
    return Str; 
}

const std::string& reuseCodeStr()
{
    static const std::string Str("reuseCode");
    return Str;    
}

const std::string& constructStr()
{
    static const std::string Str("construct");
    return Str;    
}

const std::string& readCondStr()
{
    static const std::string Str("readCond");
    return Str;    
}

const std::string& validCondStr()
{
    static const std::string Str("validCond");
    return Str;    
}

const std::string& constructAsReadCondStr()
{
    static const std::string Str("constructAsReadCond");
    return Str;       
}

const std::string& constructAsValidCondStr()
{
    static const std::string Str("constructAsValidCond");
    return Str;       
}

const std::string& copyConstructFromStr()
{
    static const std::string Str("copyConstructFrom");
    return Str;      
}

const std::string& copyReadCondFromStr()
{
    static const std::string Str("copyReadCondFrom");
    return Str;      
}

const std::string& copyValidCondFromStr()
{
    static const std::string Str("copyValidCondFrom");
    return Str;      
}

char siblingRefPrefix()
{
    return '$';
}

char stringRefPrefix()
{
    return '^';
}

char schemaRefPrefix()
{
    return '@';
}

char interfaceRefPrefix()
{
    return '%';
}

unsigned strToUnsigned(const std::string& str, bool* ok, int base)
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

ParseUnits strToUnits(const std::string& str, bool* ok)
{
    static const std::map<std::string, ParseUnits> Map = {
        std::make_pair(emptyString(), ParseUnits::Unknown),
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

    auto strToLook = toLowerCopy(str);
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

const std::string& getStringProp(
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
        /* ParseEndian_Little */ common::littleStr(),
        /* Endian_Big */ common::bigStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == ParseEndian_NumOfValues, "Invalid map");

    auto valueCpy = toLowerCopy(value);
    auto mapIter = std::find(std::begin(Map), std::end(Map), valueCpy);
    if (mapIter == std::end(Map)) {
        return ParseEndian_NumOfValues;
    }

    return static_cast<ParseEndian>(std::distance(std::begin(Map), mapIter));
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

void removeHeadingTrailingWhitespaces(std::string& str)
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

void normaliseString(std::string& str)
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

bool isValidRefName(const char* buf, std::size_t len)
{
    if ((0U < len) && (buf[0] == schemaRefPrefix())) {
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

bool isValidRefName(const std::string& value)
{
    return isValidRefName(value.c_str(), value.size());
}

bool isValidExternalRefName(const std::string& value)
{
    if (value.size() <= 1U) {
        return false;
    }

    if (value[0] != stringRefPrefix()) {
        return false;
    }

    return isValidRefName(&value[1], value.size() - 1U);
}

std::size_t maxPossibleLength()
{
    return MaxPossibleLength;
}

void addToLength(std::size_t newLen, std::size_t& accLen)
{
    if ((MaxPossibleLength - accLen) <= newLen) {
        accLen = MaxPossibleLength;
        return;
    }

    accLen += newLen;
}

std::size_t mulLength(std::size_t len, std::size_t factor)
{
    if ((((MaxPossibleLength - 1) / factor) + 1) <= len) {
        return MaxPossibleLength;
    }

    return len * factor;
}

} // namespace common

} // namespace parse

} // namespace commsdsl
