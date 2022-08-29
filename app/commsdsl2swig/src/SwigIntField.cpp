//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigIntField.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigIntField::SwigIntField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

const std::string& SwigIntField::swigConvertIntType(commsdsl::parse::IntField::Type value, std::size_t len)
{
    static const std::string Map[] = {
        /* Int8 */ "signed char",
        /* Uint8 */ "unsigned char",
        /* Int16 */ "short",
        /* Uint16 */ "unsigned short",
        /* Int32 */ "int",
        /* Uint32 */ "unsigned",
        /* Int64 */ "long long",
        /* Uint64 */ "unsigned long long",
        /* Intvar */ strings::emptyString(),
        /* Uintvar */ strings::emptyString(),
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::IntField::Type::NumOfValues), "Invalid map");

    auto idx = static_cast<unsigned>(value);
    if (MapSize <= idx) {
        assert(false); // should not happen
        return strings::emptyString();
    }

    auto& str = Map[idx];
    if (!str.empty()) {
        return str;
    }

    using Type = commsdsl::parse::IntField::Type;

    if (value == Type::Intvar) {
        if (len <= 1) {
            return swigConvertIntType(Type::Int8, 0U);
        }

        if (len <= 2) {
            return swigConvertIntType(Type::Int16, 0U);
        }

        if (len <= 4) {
            return swigConvertIntType(Type::Int32, 0U);
        }        

        return swigConvertIntType(Type::Int64, 0U);
    }

    assert(value == Type::Uintvar);
    if (len <= 1) {
        return swigConvertIntType(Type::Uint8, 0U);
    }

    if (len <= 2) {
        return swigConvertIntType(Type::Uint16, 0U);
    }

    if (len <= 4) {
        return swigConvertIntType(Type::Uint32, 0U);
    }        

    return swigConvertIntType(Type::Uint64, 0U);
}

bool SwigIntField::writeImpl() const
{
    return swigWrite();
}

std::string SwigIntField::swigValueTypeImpl() const
{
    static const std::string Templ = 
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = intDslObj();
    util::ReplacementMap repl = {
        {"TYPE", swigConvertIntType(obj.type(), obj.maxLength())}
    };

    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2swig
