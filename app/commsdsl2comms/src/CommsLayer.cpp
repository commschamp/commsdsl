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

#include "CommsLayer.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"

#include "commsdsl/gen/GenNamespace.h"
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

CommsLayer::CommsLayer(commsdsl::gen::GenLayer& layer) :
    m_layer(layer)
{
}
    
CommsLayer::~CommsLayer() = default;

bool CommsLayer::commsPrepare()
{
    m_commsExternalField = dynamic_cast<CommsField*>(m_layer.genExternalField());
    m_commsMemberField = dynamic_cast<CommsField*>(m_layer.genMemberField());
    assert((m_commsExternalField != nullptr) || (m_layer.genExternalField() == nullptr));
    assert((m_commsMemberField != nullptr) || (m_layer.genMemberField() == nullptr));
    return true;
}

CommsLayer::CommsIncludesList CommsLayer::commsCommonIncludes() const
{
    CommsIncludesList result;
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
        return strings::genEmptyString();
    }

    auto code = m_commsMemberField->commsCommonCode();
    if (code.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for all the common definitions of the fields defined in\n"
        "///     @ref #^#SCOPE#$##^#MEMBERS_SUFFIX#$# struct.\n"
        "struct #^#CLASS_NAME#$##^#MEMBERS_SUFFIX#$##^#COMMON_SUFFIX#$#\n"
        "{\n"
        "    #^#CODE#$#\n"
        "};\n";    

    util::GenReplacementMap repl = {
        {"SCOPE", comms::genScopeFor(m_layer, m_layer.genGenerator())},
        {"MEMBERS_SUFFIX", strings::genMembersSuffixStr()},
        {"COMMON_SUFFIX", strings::genCommonSuffixStr()},
        {"CLASS_NAME", comms::genClassName(m_layer.genParseObj().parseName())},
        {"CODE", std::move(code)},
    };

    return util::genProcessTemplate(Templ, repl);
}

CommsLayer::CommsIncludesList CommsLayer::commsDefIncludes() const
{
    CommsIncludesList result;
    if (m_commsExternalField != nullptr) {
        result.push_back(comms::genRelHeaderPathFor(m_commsExternalField->commsGenField(), m_layer.genGenerator()));
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
        prevName = comms::genClassName(prevLayer->layer().genParseObj().parseName());
    }

    if (hasInputMessages) {
        assert(prevLayer != nullptr);
        prevName.append("<TMessage, TAllMessages>");
    }    

    util::GenReplacementMap repl = {
        {"MEMBERS", commsDefMembersCodeInternal()},
        {"DOC", commsDefDocInternal()},
        {"CLASS_NAME", comms::genClassName(m_layer.genParseObj().parseName())},
        {"BASE", commsDefBaseTypeImpl(prevName)},
    };

    hasInputMessages = hasInputMessages || commsDefHasInputMessagesImpl();
    if (hasInputMessages) {
        repl["TEMPL_PARAMS"] = "template <typename TMessage, typename TAllMessages>";
    }

    return util::genProcessTemplate(Templ, repl);
}

bool CommsLayer::commsIsCustomizable() const
{
    auto& gen = static_cast<CommsGenerator&>(m_layer.genGenerator());
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
            false,
            commsCustomFieldOptsImpl()
        );
}

std::string CommsLayer::commsDataViewDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsDataViewDefaultOptions,
            &CommsLayer::commsExtraDataViewDefaultOptionsInternal,
            true,
            commsCustomFieldDataViewOptsImpl()
        );
}

std::string CommsLayer::commsBareMetalDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsBareMetalDefaultOptions,
            &CommsLayer::commsExtraBareMetalDefaultOptionsInternal,
            true,
            commsCustomFieldBareMetalOptsImpl()
        );
}

std::string CommsLayer::commsMsgFactoryDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            nullptr,
            &CommsLayer::commsExtraMsgFactoryDefaultOptionsInternal,
            true,
            std::string()
        );
}

CommsLayer::CommsIncludesList CommsLayer::commsDefIncludesImpl() const
{
    return CommsIncludesList();
}

std::string CommsLayer::commsDefBaseTypeImpl([[maybe_unused]] const std::string& prevName) const
{
    assert(false); // Not implemented in derived class
    return strings::genEmptyString();
}

bool CommsLayer::commsDefHasInputMessagesImpl() const
{
    return false;
}

CommsLayer::GenStringsList CommsLayer::commsDefExtraOptsImpl() const
{
    return GenStringsList();
}

bool CommsLayer::commsIsCustomizableImpl() const
{
    return false;
}

CommsLayer::GenStringsList CommsLayer::commsExtraDataViewDefaultOptionsImpl() const
{
    return GenStringsList();
}

CommsLayer::GenStringsList CommsLayer::commsExtraBareMetalDefaultOptionsImpl() const
{
    return GenStringsList();
}

CommsLayer::GenStringsList CommsLayer::commsExtraMsgFactoryDefaultOptionsImpl() const
{
    return GenStringsList();
}

std::string CommsLayer::commsCustomDefMembersCodeImpl() const
{
    return std::string();
}

std::string CommsLayer::commsCustomFieldOptsImpl() const
{
    return std::string();
}

std::string CommsLayer::commsCustomFieldDataViewOptsImpl() const
{
    return std::string();
}

std::string CommsLayer::commsCustomFieldBareMetalOptsImpl() const
{
    return std::string();
}

std::string CommsLayer::commsDefFieldType() const
{
    if (m_commsExternalField != nullptr) {
        static const std::string Templ = 
            "#^#SCOPE#$#<\n"
            "    TOpt#^#COMMA#$#\n"
            "    #^#EXTRA_OPTS#$#\n"
            ">";

        util::GenStringsList opts;
        if (m_forcedPseudoField) {
            opts.push_back("comms::option::def::EmptySerialization");
        }

        if (m_forcedFailedOnInvalidField) {
            opts.push_back("comms::option::def::FailOnInvalid<comms::ErrorStatus::ProtocolError>");
        }

        util::GenReplacementMap repl = {
            {"SCOPE", comms::genScopeFor(m_commsExternalField->commsGenField(), m_layer.genGenerator())},
            {"EXTRA_OPTS", util::genStrListToString(opts, ",\n", "")}
        };

        if (!repl["EXTRA_OPTS"].empty()) {
            repl["COMMA"] = std::string(",");
        }
        return util::genProcessTemplate(Templ, repl);
    }

    assert(m_commsMemberField != nullptr);
    return
        "typename " +
        comms::genClassName(m_layer.genParseObj().parseName()) + strings::genMembersSuffixStr() +
        "::" + comms::genClassName(m_commsMemberField->commsGenField().genParseObj().parseName());    
}

std::string CommsLayer::commsDefExtraOpts() const
{
    GenStringsList opts = commsDefExtraOptsImpl();

    if (commsIsCustomizable()) {
        auto& gen = static_cast<const CommsGenerator&>(m_layer.genGenerator());
        opts.push_back("typename TOpt::" + comms::genScopeFor(m_layer, m_layer.genGenerator(), gen.commsHasMainNamespaceInOptions()));
    }    

    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsLayer::commsMsgFactoryAliasInOptions(const commsdsl::gen::GenElem* parent)
{
    do {
        if (parent == nullptr) {
            return std::string();
        }

        if (parent->genElemType() != commsdsl::gen::GenElem::Type_Namespace) {
            break;
        }

        auto type = CommsNamespace::cast(static_cast<const commsdsl::gen::GenNamespace*>(parent))->commsMsgFactoryAliasType();
        if (type.empty()) {
            break;
        }

        return type;
    } while (false);

    return commsMsgFactoryAliasInOptions(parent->genGetParent());
}

std::string CommsLayer::commsDefMembersCodeInternal() const
{
    auto code = commsCustomDefMembersCodeImpl();
    if (!code.empty()) {
        return code;
    }

    if (m_commsMemberField == nullptr) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for field(s) of @ref #^#CLASS_NAME#$# layer.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "    #^#FIELD_DEF#$#\n"
        "};\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(m_layer.genParseObj().parseName())},
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"FIELD_DEF", m_commsMemberField->commsDefCode()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsLayer::commsDefDocInternal() const
{
    auto parseObj = m_layer.genParseObj();
    auto str = "/// @brief Definition of layer \"" + parseObj.parseName() + "\".";
    auto& desc = parseObj.parseDescription();
    if (!desc.empty()) {
        str += "\n/// @details\n";
        auto descMultiline = util::genStrMakeMultiline(desc);
        static const std::string DoxPrefix = strings::genDoxygenPrefixStr() + strings::genIndentStr();
        descMultiline = DoxPrefix + util::genStrReplace(descMultiline, "\n", "\n" + DoxPrefix);
        str += descMultiline;
    }
    return str;
}

std::string CommsLayer::commsCustomizationOptionsInternal(
    CommsFieldOptsFunc fieldOptsFunc, 
    ExtraLayerOptsFunc extraLayerOptsFunc,
    bool hasBase,
    const std::string& customFieldOpts) const
{
    util::GenStringsList elems;

    // Field portion
    do {
        if (!customFieldOpts.empty()) {
            elems.push_back(customFieldOpts);
            break;
        }

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

        util::GenReplacementMap repl = {
            {"SCOPE", comms::genScopeFor(m_layer, m_layer.genGenerator())},
            {"CLASS_NAME", comms::genClassName(m_layer.genParseObj().parseName())},
            {"SUFFIX", strings::genMembersSuffixStr()},
            {"FIELD_OPTS", std::move(fieldOpts)}
        };

        if (hasBase) {
            auto& commsGen = static_cast<const CommsGenerator&>(m_layer.genGenerator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();
            repl["EXT"] = " : public TBase::" + comms::genScopeFor(m_layer, m_layer.genGenerator(), hasMainNs) + strings::genMembersSuffixStr();
        }

        elems.push_back(util::genProcessTemplate(Templ, repl));
    } while (false);

    // Layer itself portion
    do {
        if (!commsIsCustomizable()) {
            break;
        }

        util::GenStringsList extraOpts;
        if (extraLayerOptsFunc != nullptr) {
            extraOpts = (this->*extraLayerOptsFunc)();
        }

        if (extraOpts.empty() && hasBase) {
            break;
        }        

        if (extraOpts.empty() && (!hasBase)) {
            extraOpts.push_back("comms::option::app::EmptyOption");
        }

        if ((!extraOpts.empty()) && (hasBase)) {
            auto& commsGen = static_cast<const CommsGenerator&>(m_layer.genGenerator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();            
            extraOpts.push_back("typename TBase::" + comms::genScopeFor(m_layer, m_layer.genGenerator(), hasMainNs));
        }

        auto docStr = 
            "/// @brief Extra options for @ref " +
            comms::genScopeFor(m_layer, m_layer.genGenerator()) + " layer.";
        docStr = util::genStrMakeMultiline(docStr, 40);
        docStr = util::genStrReplace(docStr, "\n", "\n" + strings::genDoxygenPrefixStr() + strings::genIndentStr()); 

        util::GenReplacementMap repl = {
            {"DOC", std::move(docStr)},
            {"NAME", comms::genClassName(m_layer.genParseObj().parseName())},
        };        

        assert(!extraOpts.empty());
        if (extraOpts.size() == 1U) {
            static const std::string Templ = 
                "#^#DOC#$#\n"
                "using #^#NAME#$# = #^#OPT#$#;\n";
        
            repl["OPT"] = extraOpts.front();
            elems.push_back(util::genProcessTemplate(Templ, repl));
            break;
        }    

        static const std::string Templ = 
            "#^#DOC#$#\n"
            "using #^#NAME#$# =\n"
            "    std::tuple<\n"
            "        #^#OPTS#$#\n"
            "    >;\n";
    
        repl["OPTS"] = util::genStrListToString(extraOpts, ",\n", "");
        elems.push_back(util::genProcessTemplate(Templ, repl));          
    } while (false);

    return util::genStrListToString(elems, "\n", "");
}

CommsLayer::GenStringsList CommsLayer::commsExtraDataViewDefaultOptionsInternal() const
{
    return commsExtraDataViewDefaultOptionsImpl();
}

CommsLayer::GenStringsList CommsLayer::commsExtraBareMetalDefaultOptionsInternal() const
{
    return commsExtraBareMetalDefaultOptionsImpl();
}

CommsLayer::GenStringsList CommsLayer::commsExtraMsgFactoryDefaultOptionsInternal() const
{
    return commsExtraMsgFactoryDefaultOptionsImpl();
}

} // namespace commsdsl2comms
