//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{

CommsChecksumLayer::CommsChecksumLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsChecksumLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::prepare();
}

CommsChecksumLayer::IncludesList CommsChecksumLayer::commsDefIncludesImpl() const
{
    IncludesList result;
    auto obj = checksumDslObj();
    if (!obj.fromLayer().empty()) {
        assert(obj.untilLayer().empty());
        result.push_back("comms/protocol/ChecksumLayer.h");
    }
    else {
        assert(!obj.untilLayer().empty());
        result.push_back("comms/protocol/ChecksumPrefixLayer.h");
    }

    const std::string ChecksumMap[] = {
        /* Custom */ strings::emptyString(),
        /* Sum */ "BasicSum",
        /* Crc_CCITT */ "Crc",
        /* Crc_16 */ "Crc",
        /* Crc_32 */ "Crc"
    };

    const std::size_t ChecksumMapSize = std::extent<decltype(ChecksumMap)>::value;
    static_assert(ChecksumMapSize == static_cast<std::size_t>(commsdsl::parse::ChecksumLayer::Alg::NumOfValues),
            "Invalid map");

    auto idx = static_cast<std::size_t>(obj.alg());
    if (ChecksumMapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        idx = 0U;
    }

    if (!ChecksumMap[idx].empty()) {
        result.push_back("comms/protocol/checksum/" + ChecksumMap[idx] + strings::cppHeaderSuffixStr());
    }
    else {
        assert(!obj.customAlgName().empty());
        result.push_back(comms::relHeaderForChecksum(comms::className(obj.customAlgName()), generator()));
    }
    return result;
}

} // namespace commsdsl2new
