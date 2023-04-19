//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "CommsLayer.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsLayer::CommsLayer(commsdsl::gen::Layer& layer) :
    m_layer(layer)
{
    static_cast<void>(m_layer);
}
    
CommsLayer::~CommsLayer() = default;

bool CommsLayer::commsPrepare()
{
    m_commsExternalField = dynamic_cast<CommsField*>(m_layer.externalField());
    m_commsMemberField = dynamic_cast<CommsField*>(m_layer.memberField());
    assert((m_commsExternalField != nullptr) || (m_layer.externalField() == nullptr));
    assert((m_commsMemberField != nullptr) || (m_layer.memberField() == nullptr));
    return true;
}

CommsLayer::IncludesList CommsLayer::commsCommonIncludes() const
{
    IncludesList result;
    if (m_commsMemberField != nullptr) {
        auto fieldIncs = m_commsMemberField->commsCommonIncludes();
        std::move(fieldIncs.begin(), fieldIncs.end(), std::back_inserter(result));
    }    

    // auto otherIncs = commsCommonIncludesImpl();
    // std::move(otherIncs.begin(), otherIncs.end(), std::back_inserter(result));
    return result;
}

std::string CommsLayer::commsCommonCode() const
{
    if (m_commsMemberField == nullptr) {
        return strings::emptyString();
    }

    auto code = m_commsMemberField->commsCommonCode();
    if (code.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for all the common definitions of the fields defined in\n"
        "///     @ref #^#SCOPE#$##^#MEMBERS_SUFFIX#$# struct.\n"
        "struct #^#CLASS_NAME#$##^#MEMBERS_SUFFIX#$##^#COMMON_SUFFIX#$#\n"
        "{\n"
        "    #^#CODE#$#\n"
        "};\n";    

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_layer, m_layer.generator())},
        {"MEMBERS_SUFFIX", strings::membersSuffixStr()},
        {"COMMON_SUFFIX", strings::commonSuffixStr()},
        {"CLASS_NAME", comms::className(m_layer.dslObj().name())},
        {"CODE", std::move(code)},
    };

    return util::processTemplate(Templ, repl);
}

CommsLayer::IncludesList CommsLayer::commsDefIncludes() const
{
    IncludesList result;
    if (m_commsExternalField != nullptr) {
        result.push_back(comms::relHeaderPathFor(m_commsExternalField->field(), m_layer.generator()));
    }

    if (m_commsMemberField != nullptr) {
        auto fieldIncs = m_commsMemberField->commsDefIncludes();
        std::move(fieldIncs.begin(), fieldIncs.end(), std::back_inserter(result));
    }    

    auto otherIncs = commsDefIncludesImpl();
    std::move(otherIncs.begin(), otherIncs.end(), std::back_inserter(result));
    return result;
}

std::string CommsLayer::commsDefType(const CommsLayer* prevLayer, bool& hasInputMessages) const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#DOC#$#\n"
        "#^#TEMPL_PARAMS#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    #^#BASE#$#;\n";    

    std::string prevName;
    if (prevLayer != nullptr) {
        prevName = comms::className(prevLayer->layer().dslObj().name());
    }

    if (hasInputMessages) {
        assert(prevLayer != nullptr);
        prevName.append("<TMessage, TAllMessages>");
    }    

    util::ReplacementMap repl = {
        {"MEMBERS", commsDefMembersCodeInternal()},
        {"DOC", commsDefDocInternal()},
        {"CLASS_NAME", comms::className(m_layer.dslObj().name())},
        {"BASE", commsDefBaseTypeImpl(prevName)},
    };

    hasInputMessages = hasInputMessages || commsDefHasInputMessagesImpl();
    if (hasInputMessages) {
        repl["TEMPL_PARAMS"] = "template <typename TMessage, typename TAllMessages>";
    }

    return util::processTemplate(Templ, repl);
}

bool CommsLayer::commsIsCustomizable() const
{
    auto& gen = static_cast<CommsGenerator&>(m_layer.generator());
    auto level = gen.commsGetCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }
        
    return commsIsCustomizableImpl();
}

void CommsLayer::commsSetForcedPseudoField()
{
    m_forcedPseudoField = true;
    if (m_commsMemberField != nullptr) {
        m_commsMemberField->commsSetForcePseudo();
    }
}

void CommsLayer::commsSetForcedFailOnInvalidField()
{
    m_forcedFailedOnInvalidField = true;
    if (m_commsMemberField != nullptr) {
        m_commsMemberField->commsSetForcedFailOnInvalid();
    }    
}  

std::string CommsLayer::commsDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsDefaultOptions,
            nullptr,
            false
        );
}

std::string CommsLayer::commsDataViewDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsDataViewDefaultOptions,
            &CommsLayer::commsExtraDataViewDefaultOptionsInternal,
            true
        );
}

std::string CommsLayer::commsBareMetalDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsBareMetalDefaultOptions,
            &CommsLayer::commsExtraBareMetalDefaultOptionsInternal,
            true
        );
}

std::string CommsLayer::commsMsgFactoryDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            nullptr,
            &CommsLayer::commsExtraMsgFactoryDefaultOptionsInternal,
            true
        );
}

CommsLayer::IncludesList CommsLayer::commsDefIncludesImpl() const
{
    return IncludesList();
}

std::string CommsLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static_cast<void>(prevName);
    assert(false); // Not implemented in derived class
    return strings::emptyString();
}

bool CommsLayer::commsDefHasInputMessagesImpl() const
{
    return false;
}

CommsLayer::StringsList CommsLayer::commsDefExtraOptsImpl() const
{
    return StringsList();
}

bool CommsLayer::commsIsCustomizableImpl() const
{
    return false;
}

CommsLayer::StringsList CommsLayer::commsExtraDataViewDefaultOptionsImpl() const
{
    return StringsList();
}

CommsLayer::StringsList CommsLayer::commsExtraBareMetalDefaultOptionsImpl() const
{
    return StringsList();
}

CommsLayer::StringsList CommsLayer::commsExtraMsgFactoryDefaultOptionsImpl() const
{
    return StringsList();
}

std::string CommsLayer::commsDefFieldType() const
{
    if (m_commsExternalField != nullptr) {
        static const std::string Templ = 
            "#^#SCOPE#$#<\n"
            "    TOpt#^#COMMA#$#\n"
            "    #^#EXTRA_OPTS#$#\n"
            ">";

        util::StringsList opts;
        if (m_forcedPseudoField) {
            opts.push_back("comms::option::def::EmptySerialization");
        }

        if (m_forcedFailedOnInvalidField) {
            opts.push_back("comms::option::def::FailOnInvalid<comms::ErrorStatus::ProtocolError>");
        }

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(m_commsExternalField->field(), m_layer.generator())},
            {"EXTRA_OPTS", util::strListToString(opts, ",\n", "")}
        };

        if (!repl["EXTRA_OPTS"].empty()) {
            repl["COMMA"] = ",";
        }
        return util::processTemplate(Templ, repl);
    }

    assert(m_commsMemberField != nullptr);
    return
        "typename " +
        comms::className(m_layer.dslObj().name()) + strings::membersSuffixStr() +
        "::" + comms::className(m_commsMemberField->field().dslObj().name());    
}

std::string CommsLayer::commsDefExtraOpts() const
{
    StringsList opts = commsDefExtraOptsImpl();

    if (commsIsCustomizable()) {
        auto& gen = static_cast<const CommsGenerator&>(m_layer.generator());
        opts.push_back("typename TOpt::" + comms::scopeFor(m_layer, m_layer.generator(), gen.commsHasMainNamespaceInOptions()));
    }    

    return util::strListToString(opts, ",\n", "");
}

std::string CommsLayer::commsDefMembersCodeInternal() const
{
    if (m_commsMemberField == nullptr) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for field(s) of @ref #^#CLASS_NAME#$# layer.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "    #^#FIELD_DEF#$#\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(m_layer.dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"FIELD_DEF", m_commsMemberField->commsDefCode()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsLayer::commsDefDocInternal() const
{
    auto dslObj = m_layer.dslObj();
    auto str = "/// @brief Definition of layer \"" + dslObj.name() + "\".";
    auto& desc = dslObj.description();
    if (!desc.empty()) {
        str += "\n/// @details\n";
        auto descMultiline = util::strMakeMultiline(desc);
        static const std::string DoxPrefix = strings::doxygenPrefixStr() + strings::indentStr();
        descMultiline = DoxPrefix + util::strReplace(descMultiline, "\n", "\n" + DoxPrefix);
        str += descMultiline;
    }
    return str;
}

std::string CommsLayer::commsCustomizationOptionsInternal(
    FieldOptsFunc fieldOptsFunc, 
    ExtraLayerOptsFunc extraLayerOptsFunc,
    bool hasBase) const
{
    util::StringsList elems;

    // Field portion
    do {
        if ((m_commsMemberField == nullptr) || (fieldOptsFunc == nullptr)) {
            break;
        }

        auto fieldOpts = (m_commsMemberField->*fieldOptsFunc)();
        if (fieldOpts.empty()) {
            break;
        }

        static const std::string Templ =
            "/// @brief Extra options for all the member fields of\n"
            "///     @ref #^#SCOPE#$# layer field.\n"
            "struct #^#CLASS_NAME#$##^#SUFFIX#$##^#EXT#$#\n"
            "{\n"
            "    #^#FIELD_OPT#$#\n"
            "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(m_layer, m_layer.generator())},
            {"CLASS_NAME", comms::className(m_layer.dslObj().name())},
            {"SUFFIX", strings::membersSuffixStr()},
            {"FIELD_OPTS", std::move(fieldOpts)}
        };

        if (hasBase) {
            auto& commsGen = static_cast<const CommsGenerator&>(m_layer.generator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();
            repl["EXT"] = " : public TBase::" + comms::scopeFor(m_layer, m_layer.generator(), hasMainNs) + strings::membersSuffixStr();
        }

        elems.push_back(util::processTemplate(Templ, repl));
    } while (false);

    // Layer itself portion
    do {
        if (!commsIsCustomizable()) {
            break;
        }

        util::StringsList extraOpts;
        if (extraLayerOptsFunc != nullptr) {
            extraOpts = (this->*extraLayerOptsFunc)();
        }

        if (extraOpts.empty() && hasBase) {
            break;
        }        

        if (extraOpts.empty() && (!hasBase)) {
            extraOpts.push_back("comms::option::EmptyOption");
        }

        if ((!extraOpts.empty()) && (hasBase)) {
            auto& commsGen = static_cast<const CommsGenerator&>(m_layer.generator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();            
            extraOpts.push_back("typename TBase::" + comms::scopeFor(m_layer, m_layer.generator(), hasMainNs));
        }

        auto docStr = 
            "/// @brief Extra options for @ref " +
            comms::scopeFor(m_layer, m_layer.generator()) + " layer.";
        docStr = util::strMakeMultiline(docStr, 40);
        docStr = util::strReplace(docStr, "\n", "\n" + strings::doxygenPrefixStr() + strings::indentStr()); 

        util::ReplacementMap repl = {
            {"DOC", std::move(docStr)},
            {"NAME", comms::className(m_layer.dslObj().name())},
        };        

        assert(!extraOpts.empty());
        if (extraOpts.size() == 1U) {
            static const std::string Templ = 
                "#^#DOC#$#\n"
                "using #^#NAME#$# = #^#OPT#$#;\n";
        
            repl["OPT"] = extraOpts.front();
            elems.push_back(util::processTemplate(Templ, repl));
            break;
        }    

        static const std::string Templ = 
            "#^#DOC#$#\n"
            "using #^#NAME#$# =\n"
            "    std::tuple<\n"
            "        #^#OPTS#$#\n"
            "    >;\n";
    
        repl["OPTS"] = util::strListToString(extraOpts, ",\n", "");
        elems.push_back(util::processTemplate(Templ, repl));          
    } while (false);

    return util::strListToString(elems, "\n", "");
}

CommsLayer::StringsList CommsLayer::commsExtraDataViewDefaultOptionsInternal() const
{
    return commsExtraDataViewDefaultOptionsImpl();
}

CommsLayer::StringsList CommsLayer::commsExtraBareMetalDefaultOptionsInternal() const
{
    return commsExtraBareMetalDefaultOptionsImpl();
}

CommsLayer::StringsList CommsLayer::commsExtraMsgFactoryDefaultOptionsInternal() const
{
    return commsExtraMsgFactoryDefaultOptionsImpl();
}

} // namespace commsdsl2comms
