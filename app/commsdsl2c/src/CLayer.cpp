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

#include "CLayer.h"

#include "CFrame.h"
#include "CGenerator.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

CLayer::CLayer(GenLayer& layer) :
    m_genLayer(layer)
{
}

CLayer::~CLayer() = default;

const CLayer* CLayer::cCast(const GenLayer* layer)
{
    if (layer == nullptr) {
        return nullptr;
    }

    auto* cLayer = dynamic_cast<const CLayer*>(layer);
    assert(cLayer != nullptr);
    return cLayer;
}

bool CLayer::cPrepare()
{
    m_cExternalField = CField::cCast(m_genLayer.genExternalField());
    m_cMemberField = CField::cCast(m_genLayer.genMemberField());
    return true;
}

void CLayer::cAddHeaderIncludes(GenStringsList& includes) const
{
    if (m_cExternalField != nullptr) {
        includes.push_back(m_cExternalField->cRelHeader());
    }

    if (m_cMemberField != nullptr) {
        m_cMemberField->cAddHeaderIncludes(includes);
    }
}

void CLayer::cAddSourceIncludes(GenStringsList& includes) const
{
    if (m_cExternalField != nullptr) {
        includes.push_back(m_cExternalField->cRelCommsHeader());
    }

    if (m_cMemberField != nullptr) {
        m_cMemberField->cAddSourceIncludes(includes);
    }
}

void CLayer::cAddCommsHeaderIncludes(GenStringsList& includes) const
{
    if (m_cExternalField != nullptr) {
        includes.push_back(m_cExternalField->cRelCommsHeader());
    }

    if (m_cMemberField != nullptr) {
        m_cMemberField->cAddCommsHeaderIncludes(includes);
    }
}

std::string CLayer::cName() const
{
    auto& cGenerator = CGenerator::cCast(m_genLayer.genGenerator());
    return cGenerator.cNameFor(m_genLayer);
}

std::string CLayer::cCommsTypeName() const
{
    return cName() + strings::genCommsNameSuffixStr();
}

std::string CLayer::cCommsType() const
{
    auto* frame = cParentFrame();
    assert(frame != nullptr);
    auto frameType = frame->cCommsType(false) + strings::genLayersSuffixStr();

    auto& cGenerator = CGenerator::cCast(m_genLayer.genGenerator());
    auto scope = comms::genScopeFor(m_genLayer, cGenerator);

    assert(frameType.size() <= scope.size());
    return frameType + '<' + CProtocolOptions::cName(cGenerator) + '>' + scope.substr(frameType.size());
}

std::string CLayer::cHeaderCode() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "/// @brief Framing layer <b>#^#DISP_NAME#$#</b> of @ref #^#FRAME#$# frame.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        "#^#CODE#$#\n"
        ;

    auto* cFrame = cParentFrame();

    auto parseObj = m_genLayer.genParseObj();

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"CODE", cHeaderCodeImpl()},
        {"FRAME", cFrame->cName()},
    };

    if (m_cMemberField != nullptr) {
        repl["FIELD"] = m_cMemberField->cHeaderCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CLayer::cSourceCode() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "#^#CODE#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"CODE", cSourceCodeImpl()},
    };

    if (m_cMemberField != nullptr) {
        repl["FIELD"] = m_cMemberField->cSourceCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CLayer::cCommsHeaderCode(const CInterface& iFace) const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "using #^#COMMS_NAME#$# = ::#^#COMMS_TYPE#$#;\n"
        "struct alignas(alignof(#^#COMMS_NAME#$#)) #^#NAME#$#_ {};\n"
        "\n"
        "inline const #^#COMMS_NAME#$#* fromLayerHandle(const #^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#COMMS_NAME#$#* fromLayerHandle(#^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline const #^#NAME#$#* toLayerHandle(const #^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#NAME#$#* toLayerHandle(#^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#NAME#$#*>(from);\n"
        "}\n"
        ;

    auto& cGenerator = CGenerator::cCast(m_genLayer.genGenerator());
    auto* ns = CNamespace::cCast(cParentFrame()->genParentNamespace());
    auto* input = ns->cInputMessages();
    assert(input != nullptr);

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"COMMS_NAME", cCommsTypeName()},
        {"COMMS_TYPE", m_genLayer.genTemplateScopeOfComms(iFace.cCommsTypeName(), input->cName(), CProtocolOptions::cName(cGenerator))},
    };

    if (m_cMemberField != nullptr) {
        repl["FIELD"] = m_cMemberField->cCommsHeaderCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool CLayer::cIsInterfaceSupported(const CInterface& iFace) const
{
    return cIsInterfaceSupportedImpl(iFace);
}

std::string CLayer::cFrameValueDef() const
{
    return cFrameValueDefImpl();
}

std::string CLayer::cFrameValueAssign(
    const std::string& valuesPtrName,
    const std::string& commsBundleName,
    unsigned layerIdx) const
{
    return cFrameValueAssignImpl(valuesPtrName, commsBundleName, layerIdx);
}

std::string CLayer::cHeaderCodeImpl() const
{
    return strings::genEmptyString();
}

std::string CLayer::cSourceCodeImpl() const
{
    return strings::genEmptyString();
}

bool CLayer::cIsInterfaceSupportedImpl([[maybe_unused]] const CInterface& iFace) const
{
    return true;
}

bool CLayer::cHasInputMessagesImpl() const
{
    return false;
}

std::string CLayer::cFrameValueDefImpl() const
{
    auto* field = cField();
    if (field == nullptr) {
        return strings::genEmptyString();
    }

    auto str = field->cFrameValueDef(comms::genAccessName(m_genLayer.genName()));
    if (!str.empty()) {
        str += " ///< Access to the value processed by the @ref " + cName() + " layer";
    }

    return str;
}

std::string CLayer::cFrameValueAssignImpl(
    const std::string& valuesPtrName,
    const std::string& commsBundleName,
    unsigned layerIdx) const
{
    auto* field = cField();
    if (field == nullptr) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "std::get<#^#IDX#$#>(#^#BUNDLE_NAME#$#)"
        ;

    util::GenReplacementMap repl = {
        {"IDX", std::to_string(layerIdx)},
        {"BUNDLE_NAME", commsBundleName},
    };

    auto rightHandValue = util::genProcessTemplate(Templ, repl);
    return field->cFrameValueAssign(valuesPtrName + "->m_" + comms::genAccessName(m_genLayer.genName()), rightHandValue);
}

const CField* CLayer::cField() const
{
    if (m_cExternalField != nullptr) {
        return m_cExternalField;
    }

    return m_cMemberField;
}

const CFrame* CLayer::cParentFrame() const
{
    auto* parent = m_genLayer.genGetParent();
    assert(parent != nullptr);
    auto* cFrame = CFrame::cCast(static_cast<const commsdsl::gen::GenFrame*>(parent));
    assert(cFrame != nullptr);
    return cFrame;
}

} // namespace commsdsl2c
