//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "common.h"

#include <algorithm>
#include <iterator>
#include <cctype>
#include <cmath>
#include <cassert>
#include <limits>

namespace commsdsl
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

Units strToUnits(const std::string& str, bool* ok)
{
    static const std::map<std::string, Units> Map = {
        std::make_pair(emptyString(), Units::Unknown),
        std::make_pair("ns", Units::Nanoseconds),
        std::make_pair("nanosec", Units::Nanoseconds),
        std::make_pair("nanosecs", Units::Nanoseconds),
        std::make_pair("nanosecond", Units::Nanoseconds),
        std::make_pair("nanoseconds", Units::Nanoseconds),
        std::make_pair("us", Units::Microseconds),
        std::make_pair("microsec", Units::Microseconds),
        std::make_pair("microsecs", Units::Microseconds),
        std::make_pair("microsecond", Units::Microseconds),
        std::make_pair("microseconds", Units::Microseconds),
        std::make_pair("ms", Units::Milliseconds),
        std::make_pair("millisec", Units::Milliseconds),
        std::make_pair("millisecs", Units::Milliseconds),
        std::make_pair("millisecond", Units::Milliseconds),
        std::make_pair("milliseconds", Units::Milliseconds),
        std::make_pair("s", Units::Seconds),
        std::make_pair("sec", Units::Seconds),
        std::make_pair("secs", Units::Seconds),
        std::make_pair("second", Units::Seconds),
        std::make_pair("seconds", Units::Seconds),
        std::make_pair("min", Units::Minutes),
        std::make_pair("mins", Units::Minutes),
        std::make_pair("minute", Units::Minutes),
        std::make_pair("minutes", Units::Minutes),
        std::make_pair("h", Units::Hours),
        std::make_pair("hour", Units::Hours),
        std::make_pair("hours", Units::Hours),
        std::make_pair("d", Units::Days),
        std::make_pair("day", Units::Days),
        std::make_pair("days", Units::Days),
        std::make_pair("w", Units::Weeks),
        std::make_pair("week", Units::Weeks),
        std::make_pair("weeks", Units::Weeks),
        std::make_pair("nm", Units::Nanometers),
        std::make_pair("nanometer", Units::Nanometers),
        std::make_pair("nanometre", Units::Nanometers),
        std::make_pair("nanometres", Units::Nanometers),
        std::make_pair("nanometers", Units::Nanometers),
        std::make_pair("um", Units::Micrometers),
        std::make_pair("micrometer", Units::Micrometers),
        std::make_pair("micrometre", Units::Micrometers),
        std::make_pair("micrometres", Units::Micrometers),
        std::make_pair("micrometers", Units::Micrometers),
        std::make_pair("mm", Units::Millimeters),
        std::make_pair("millimeter", Units::Millimeters),
        std::make_pair("millimetre", Units::Millimeters),
        std::make_pair("millimetres", Units::Millimeters),
        std::make_pair("millimeters", Units::Millimeters),
        std::make_pair("cm", Units::Centimeters),
        std::make_pair("centimeter", Units::Centimeters),
        std::make_pair("centimetre", Units::Centimeters),
        std::make_pair("centimetres", Units::Centimeters),
        std::make_pair("centimeters", Units::Centimeters),
        std::make_pair("m", Units::Meters),
        std::make_pair("meter", Units::Meters),
        std::make_pair("metre", Units::Meters),
        std::make_pair("metres", Units::Meters),
        std::make_pair("meters", Units::Meters),
        std::make_pair("km", Units::Kilometers),
        std::make_pair("kilometer", Units::Kilometers),
        std::make_pair("kilometre", Units::Kilometers),
        std::make_pair("kilometres", Units::Kilometers),
        std::make_pair("kilometers", Units::Kilometers),
        std::make_pair("nm/s", Units::NanometersPerSecond),
        std::make_pair("nmps", Units::NanometersPerSecond),
        std::make_pair("nanometer/second", Units::NanometersPerSecond),
        std::make_pair("nanometre/second", Units::NanometersPerSecond),
        std::make_pair("nanometers/second", Units::NanometersPerSecond),
        std::make_pair("nanometres/second", Units::NanometersPerSecond),
        std::make_pair("um/s", Units::MicrometersPerSecond),
        std::make_pair("umps", Units::MicrometersPerSecond),
        std::make_pair("micrometers/second", Units::MicrometersPerSecond),
        std::make_pair("micrometres/second", Units::MicrometersPerSecond),
        std::make_pair("micrometer/second", Units::MicrometersPerSecond),
        std::make_pair("micrometre/second", Units::MicrometersPerSecond),
        std::make_pair("mm/s", Units::MillimetersPerSecond),
        std::make_pair("mmps", Units::MillimetersPerSecond),
        std::make_pair("millimeter/second", Units::MillimetersPerSecond),
        std::make_pair("millimetre/second", Units::MillimetersPerSecond),
        std::make_pair("millimeters/second", Units::MillimetersPerSecond),
        std::make_pair("millimetres/second", Units::MillimetersPerSecond),
        std::make_pair("cm/s", Units::CentimetersPerSecond),
        std::make_pair("cmps", Units::CentimetersPerSecond),
        std::make_pair("centimeter/second", Units::CentimetersPerSecond),
        std::make_pair("centimetre/second", Units::CentimetersPerSecond),
        std::make_pair("centimeters/second", Units::CentimetersPerSecond),
        std::make_pair("centimetres/second", Units::CentimetersPerSecond),
        std::make_pair("m/s", Units::MetersPerSecond),
        std::make_pair("mps", Units::MetersPerSecond),
        std::make_pair("meter/second", Units::MetersPerSecond),
        std::make_pair("metre/second", Units::MetersPerSecond),
        std::make_pair("meters/second", Units::MetersPerSecond),
        std::make_pair("metres/second", Units::MetersPerSecond),
        std::make_pair("km/s", Units::KilometersPerSecond),
        std::make_pair("kmps", Units::KilometersPerSecond),
        std::make_pair("kps", Units::KilometersPerSecond),
        std::make_pair("kilometer/second", Units::KilometersPerSecond),
        std::make_pair("kilometre/second", Units::KilometersPerSecond),
        std::make_pair("kilometers/second", Units::KilometersPerSecond),
        std::make_pair("kilometres/second", Units::KilometersPerSecond),
        std::make_pair("km/h", Units::KilometersPerHour),
        std::make_pair("kmph", Units::KilometersPerHour),
        std::make_pair("kph", Units::KilometersPerHour),
        std::make_pair("kilometer/hour", Units::KilometersPerHour),
        std::make_pair("kilometre/hour", Units::KilometersPerHour),
        std::make_pair("kilometers/hour", Units::KilometersPerHour),
        std::make_pair("kilometres/hour", Units::KilometersPerHour),
        std::make_pair("hz", Units::Hertz),
        std::make_pair("hertz", Units::Hertz),
        std::make_pair("khz", Units::KiloHertz),
        std::make_pair("kilohertz", Units::KiloHertz),
        std::make_pair("mhz", Units::MegaHertz),
        std::make_pair("megahertz", Units::MegaHertz),
        std::make_pair("ghz", Units::GigaHertz),
        std::make_pair("gigahertz", Units::GigaHertz),
        std::make_pair("deg", Units::Degrees),
        std::make_pair("degree", Units::Degrees),
        std::make_pair("degrees", Units::Degrees),
        std::make_pair("rad", Units::Radians),
        std::make_pair("radian", Units::Radians),
        std::make_pair("radians", Units::Radians),
        std::make_pair("na", Units::Nanoamps),
        std::make_pair("nanoamp", Units::Nanoamps),
        std::make_pair("nanoamps", Units::Nanoamps),
        std::make_pair("nanoampere", Units::Nanoamps),
        std::make_pair("nanoamperes", Units::Nanoamps),
        std::make_pair("ua", Units::Microamps),
        std::make_pair("microamp", Units::Microamps),
        std::make_pair("microamps", Units::Microamps),
        std::make_pair("microampere", Units::Microamps),
        std::make_pair("microamperes", Units::Microamps),
        std::make_pair("ma", Units::Milliamps),
        std::make_pair("milliamp", Units::Milliamps),
        std::make_pair("milliamps", Units::Milliamps),
        std::make_pair("milliampere", Units::Milliamps),
        std::make_pair("milliamperes", Units::Milliamps),
        std::make_pair("a", Units::Amps),
        std::make_pair("amp", Units::Amps),
        std::make_pair("amps", Units::Amps),
        std::make_pair("ampere", Units::Amps),
        std::make_pair("amperes", Units::Amps),
        std::make_pair("ka", Units::Kiloamps),
        std::make_pair("kiloamp", Units::Kiloamps),
        std::make_pair("kiloamps", Units::Kiloamps),
        std::make_pair("kiloampere", Units::Kiloamps),
        std::make_pair("kiloamperes", Units::Kiloamps),
        std::make_pair("nv", Units::Nanovolts),
        std::make_pair("nanovolt", Units::Nanovolts),
        std::make_pair("nanovolts", Units::Nanovolts),
        std::make_pair("uv", Units::Microvolts),
        std::make_pair("microvolt", Units::Microvolts),
        std::make_pair("microvolts", Units::Microvolts),
        std::make_pair("mv", Units::Millivolts),
        std::make_pair("millivolt", Units::Millivolts),
        std::make_pair("millivolts", Units::Millivolts),
        std::make_pair("v", Units::Volts),
        std::make_pair("volt", Units::Volts),
        std::make_pair("volts", Units::Volts),
        std::make_pair("kv", Units::Kilovolts),
        std::make_pair("kilovolt", Units::Kilovolts),
        std::make_pair("kilovolts", Units::Kilovolts),
        std::make_pair("b", Units::Bytes),
        std::make_pair("byte", Units::Bytes),
        std::make_pair("bytes", Units::Bytes),
        std::make_pair("kb", Units::Kilobytes),
        std::make_pair("kilobyte", Units::Kilobytes),
        std::make_pair("kilobytes", Units::Kilobytes),
        std::make_pair("mb", Units::Megabytes),
        std::make_pair("megabyte", Units::Megabytes),
        std::make_pair("megabytes", Units::Megabytes),        
        std::make_pair("gb", Units::Gigabytes),
        std::make_pair("gigabyte", Units::Gigabytes),
        std::make_pair("gigabytes", Units::Gigabytes),
        std::make_pair("tb", Units::Terabytes),
        std::make_pair("terabyte", Units::Terabytes),
        std::make_pair("terabytes", Units::Terabytes),
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
        return Units::Unknown;
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

    auto valueCpy = toLowerCopy(value);
    auto mapIter = std::find(std::begin(Map), std::end(Map), valueCpy);
    if (mapIter == std::end(Map)) {
        return Endian_NumOfValues;
    }

    return static_cast<Endian>(std::distance(std::begin(Map), mapIter));
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

    if (value[0] != '^') {
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

} // namespace commsdsl
