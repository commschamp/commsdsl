//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkLayer.h"

#include "WiresharkField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkLayer::WiresharkLayer(GenLayer& layer) :
    m_genLayer(layer)
{
}

WiresharkLayer::~WiresharkLayer() = default;

const WiresharkLayer* WiresharkLayer::wiresharkCast(const GenLayer* layer)
{
    if (layer == nullptr) {
        return nullptr;
    }

    return dynamic_cast<const WiresharkLayer*>(layer);
}

WiresharkLayer* WiresharkLayer::wiresharkCast(GenLayer* layer)
{
    if (layer == nullptr) {
        return nullptr;
    }

    return dynamic_cast<WiresharkLayer*>(layer);
}

std::string WiresharkLayer::wiresharkDissectName() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genLayer.genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(m_genLayer);
}

std::string WiresharkLayer::wiresharkDissectCode() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offsetLimit)\n"
        "    #^#REPLACE#$#\n"
        "    local result = true\n"
        "    #^#BODY#$#\n"
        "    return offset, result\n"
        "end\n"
        "#^#EXTEND#$#\n"
    ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genLayer.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(m_genLayer);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"FIELD", wiresharkFieldDissectCodeInternal()},
        {"NAME", wiresharkDissectName()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyImpl();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkLayer::wiresharkDissectBodyImpl() const
{
    auto* field = WiresharkField::wiresharkCast(m_genLayer.genMemberField());
    if (field == nullptr) {
        field = WiresharkField::wiresharkCast(m_genLayer.genExternalField());
    }

    if (field == nullptr) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "offset, result = #^#NAME#$#(tvb, tree, offset, offsetLimit)"
    ;

    util::GenReplacementMap repl = {
        {"NAME", field->wiresharkDissectName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkLayer::wiresharkFieldDissectCodeInternal() const
{
    auto* field = WiresharkField::wiresharkCast(m_genLayer.genMemberField());
    if (field == nullptr) {
        return strings::genEmptyString();
    }

    return field->wiresharkDissectCode();
}

} // namespace commsdsl2wireshark
