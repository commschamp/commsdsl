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

#include "Wireshark.h"
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
        "#^#EXTRA#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offset_limit, funcs, next_idx, msg)\n"
        "    #^#REPLACE#$#\n"
        "    local result = #^#SUCCESS#$#\n"
        "    local next_offset = offset\n"
        "    #^#BODY#$#\n"
        "    return result, next_offset\n"
        "end\n"
        "#^#EXTEND#$#\n"
    ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genLayer.genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(m_genLayer);
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
        {"EXTRA", wiresharkExtraDissectCodeImpl()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyImpl();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkLayer::wiresharkIsInterfaceSupported(const WiresharkInterface& iFace) const
{
    return wiresharkIsInterfaceSupportedImpl(iFace);
}

std::string WiresharkLayer::wiresharkDissectBodyImpl() const
{
    return strings::genEmptyString();
}

bool WiresharkLayer::wiresharkIsInterfaceSupportedImpl([[maybe_unused]] const WiresharkInterface& iFace) const
{
    return true;
}

std::string WiresharkLayer::wiresharkExtraDissectCodeImpl() const
{
    return strings::genEmptyString();
}

std::string WiresharkLayer::wiresharkDissectFieldCode() const
{
    auto* field = wiresharkField();

    if (field == nullptr) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "result, next_offset = #^#NAME#$#(tvb, tree, offset, offset_limit)\n"
        "if result ~= #^#SUCCESS#$# then\n"
        "    return result, next_offset\n"
        "end\n"
    ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genLayer.genGenerator());
    util::GenReplacementMap repl = {
        {"NAME", field->wiresharkDissectName()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkLayer::wiresharkNextFuncCode() const
{
    static const std::string Templ =
        "local next_func = funcs[next_idx]\n"
        "if next_func == #^#NIL#$# then\n"
        "    return #^#ERROR#$#, offset\n"
        "end\n"
        "result, next_offset = next_func(tvb, tree, offset, offset_limit, funcs, next_idx + 1, msg)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(m_genLayer.genGenerator());
    util::GenReplacementMap repl = {
        {"NIL", strings::genNilStr()},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::CodegenError)},
    };

    return util::genProcessTemplate(Templ, repl);
}

const WiresharkField* WiresharkLayer::wiresharkField() const
{
    auto* field = m_genLayer.genMemberField();
    if (field == nullptr) {
        field = m_genLayer.genExternalField();
    }

    return WiresharkField::wiresharkCast(field);
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
