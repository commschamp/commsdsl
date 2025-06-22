//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsChecksumLayer.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "CommsGenerator.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsChecksumLayer::CommsChecksumLayer(CommsGenerator& generator, commsdsl::parse::ParseLayer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsChecksumLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::commsPrepare();
}

CommsChecksumLayer::IncludesList CommsChecksumLayer::commsDefIncludesImpl() const
{
    IncludesList result;
    auto obj = checksumDslObj();
    if (!obj.fromLayer().empty()) {
        assert(obj.untilLayer().empty());
        result.push_back("comms/frame/ChecksumLayer.h");
    }
    else {
        assert(!obj.untilLayer().empty());
        result.push_back("comms/frame/ChecksumPrefixLayer.h");
    }

    const std::string ChecksumMap[] = {
        /* Custom */ strings::emptyString(),
        /* Sum */ "BasicSum",
        /* Crc_CCITT */ "Crc",
        /* Crc_16 */ "Crc",
        /* Crc_32 */ "Crc",
        /* Xor */ "BasicXor",
    };

    const std::size_t ChecksumMapSize = std::extent<decltype(ChecksumMap)>::value;
    static_assert(ChecksumMapSize == static_cast<std::size_t>(commsdsl::parse::ParseChecksumLayer::Alg::NumOfValues),
            "Invalid map");

    auto idx = static_cast<std::size_t>(obj.alg());
    if (ChecksumMapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        idx = 0U;
    }

    if (!ChecksumMap[idx].empty()) {
        result.push_back("comms/frame/checksum/" + ChecksumMap[idx] + strings::cppHeaderSuffixStr());
    }
    else {
        assert(!obj.customAlgName().empty());
        result.push_back(comms::relHeaderForChecksum(comms::className(obj.customAlgName()), generator()));
    }
    return result;
}

std::string CommsChecksumLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ = 
        "comms::frame::Checksum#^#PREFIX_VAR#$#Layer<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    #^#ALG#$#,\n"
        "    #^#PREV_LAYER#$##^#COMMA#$#\n"
        "    #^#EXTRA_OPT#$#\n"
        ">";    

    util::ReplacementMap repl = {
        {"FIELD_TYPE", commsDefFieldType()},
        {"ALG", commsDefAlgInternal()},
        {"PREV_LAYER", prevName},
        {"EXTRA_OPT", commsDefExtraOptInternal()}
    };

    if (!repl["EXTRA_OPT"].empty()) {
        repl["COMMA"] = std::string(",");
    }

    if (!checksumDslObj().untilLayer().empty()) {
        repl["PREFIX_VAR"] = "Prefix";
    }    

    return util::processTemplate(Templ, repl);
}

std::string CommsChecksumLayer::commsDefAlgInternal() const
{
    const std::string ClassMap[] = {
        /* Custom */ strings::emptyString(),
        /* Sum */ "BasicSum",
        /* Crc_CCITT */ "Crc_CCITT",
        /* Crc_16 */ "Crc_16",
        /* Crc_32 */ "Crc_32",
        /* Xor */ "BasicXor",
    };

    const std::size_t ClassMapSize = std::extent<decltype(ClassMap)>::value;
    static_assert(ClassMapSize == static_cast<std::size_t>(commsdsl::parse::ParseChecksumLayer::Alg::NumOfValues),
            "Invalid map");


    auto obj = checksumDslObj();
    auto alg = obj.alg();
    auto idx = static_cast<std::size_t>(alg);

    if (ClassMapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        idx = 0U;
    }

    if (ClassMap[idx].empty()) {
        assert(!obj.customAlgName().empty());
        return comms::scopeForChecksum(obj.customAlgName(), generator());
    }

    auto str = "comms::frame::checksum::" + ClassMap[idx];
    if ((alg != commsdsl::parse::ParseChecksumLayer::Alg::Sum) &&
        (alg != commsdsl::parse::ParseChecksumLayer::Alg::Xor)) {
        return str;
    }

    static const std::string Templ = 
        "#^#ALG#$#<\n"
        "    #^#FIELD#$#::ValueType\n"
        ">";

    util::ReplacementMap repl = {
        {"ALG", std::move(str)},
        {"FIELD", commsDefFieldType()},
    };

    if (!util::strStartsWith(repl["FIELD"], "typename")) {
        repl["FIELD"] = "typename " + repl["FIELD"];
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsChecksumLayer::commsDefExtraOptInternal() const
{
    std::string result;
    if (checksumDslObj().verifyBeforeRead()) {
        result = "comms::option::def::ChecksumLayerVerifyBeforeRead";
    }
    return result;
}

} // namespace commsdsl2comms
