//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CDataField.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

CDataField::CDataField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CDataField::genWriteImpl() const
{
    return cWrite();
}

void CDataField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    includes.push_back("<algorithm>");
    includes.push_back("<cstdint>");
    includes.push_back("comms/util/assign.h");
}

std::string CDataField::cHeaderCodeImpl() const
{
    static const std::string Templ =
        "/// @brief Get the stored value of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "/// @details When value is copied to the output buffer.\n"
        "/// @param[in] field Field handle.\n"
        "/// @param[out] buf Buffer to which value is copied.\n"
        "/// @param[in] bufSize Available size of the buffer.\n"
        "/// @return Amount of bytes copied.\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_getValue(const #^#NAME#$##^#SUFFIX#$#* field, uint8_t* buf, size_t bufSize);\n"
        "\n"
        "/// @brief Get size of the currently stored field value.\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_valueSize(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n#^#SET_FUNC#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (!genParseObj().parseIsFixedValue()) {
        static const std::string SetTempl = 
            "/// @brief Set value of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
            "/// @param[in, out] field Field handle.\n"
            "/// @param[in] buf Buffer from which the value to be copied.\n"
            "/// @param[in] bufSize Size of the buffer to copy.\n"
            "void #^#NAME#$##^#SUFFIX#$#_setValue(#^#NAME#$##^#SUFFIX#$#* field, const uint8_t* buf, size_t bufSize);"
            ;

        repl["SET_FUNC"] = util::genProcessTemplate(SetTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CDataField::cSourceCodeImpl() const
{
    static const std::string Templ =
        "size_t #^#NAME#$##^#SUFFIX#$#_getValue(const #^#NAME#$##^#SUFFIX#$#* field, uint8_t* buf, size_t bufSize)\n"
        "{\n"
        "    auto& fieldValue = from#^#CONV_SUFFIX#$#(field)->getValue();\n"
        "    auto bytesToCopy = std::min(bufSize, fieldValue.size());\n"
        "    std::copy_n(fieldValue.begin(), bytesToCopy, buf);\n"
        "    return bytesToCopy;\n"
        "}\n"
        "\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_valueSize(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->getValue().size();\n"
        "}\n"        
        "\n"
        "#^#SET_FUNC#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"CONV_SUFFIX", cConversionSuffix()}
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (!genParseObj().parseIsFixedValue()) {
        static const std::string SetTempl = 
            "void #^#NAME#$##^#SUFFIX#$#_setValue(#^#NAME#$##^#SUFFIX#$#* field, const uint8_t* buf, size_t bufSize)\n"
            "{\n"
            "    comms::util::assign(from#^#CONV_SUFFIX#$#(field)->value(), buf, buf + bufSize);\n"
            "}"
            ;

        repl["SET_FUNC"] = util::genProcessTemplate(SetTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
