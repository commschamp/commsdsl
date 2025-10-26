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

#include "CPayloadLayer.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

CPayloadLayer::CPayloadLayer(CGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CPayloadLayer::genPrepareImpl()
{
    return GenBase::genPrepareImpl() && cPrepare();
}

std::string CPayloadLayer::cFrameValueDefImpl() const
{
    static const std::string Templ =
        "uint8_t* m_#^#ACC_NAME#$#; ///< Buffer containing bytes processed by the @ref #^#NAME#$# layer.\n"
        "size_t m_#^#ACC_NAME#$#Len; ///< Amount of bytes processed by the @ref #^#NAME#$# layer."
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"ACC_NAME", comms::genAccessName(genName())},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CPayloadLayer::cFrameValueAssignImpl(
    const std::string& valuesPtrName,
    const std::string& commsBundleName,
    unsigned layerIdx) const
{
    static const std::string Templ =
        "if (#^#VALUES#$#->m_#^#ACC_NAME#$# != nullptr) {\n"
        "    auto& payloadField = std::get<#^#IDX#$#>(#^#BUNDLE#$#);\n"
        "    auto actBufSize = std::min(#^#VALUES#$#->m_#^#ACC_NAME#$#Len, payloadField.value().size());\n"
        "    std::copy_n(payloadField.value().begin(), actBufSize, #^#VALUES#$#->m_#^#ACC_NAME#$#);\n"
        "    #^#VALUES#$#->m_#^#ACC_NAME#$#Len = actBufSize;\n"
        "}";

    util::GenReplacementMap repl = {
        {"VALUES", valuesPtrName},
        {"BUNDLE", commsBundleName},
        {"IDX", std::to_string(layerIdx)},
        {"ACC_NAME", comms::genAccessName(genName())},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
