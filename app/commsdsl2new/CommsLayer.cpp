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

namespace commsdsl2new
{

CommsLayer::CommsLayer(commsdsl::gen::Layer& layer) :
    m_layer(layer)
{
    static_cast<void>(m_layer);
}
    
CommsLayer::~CommsLayer() = default;

bool CommsLayer::prepare()
{
    m_commsExternalField = dynamic_cast<CommsField*>(m_layer.externalField());
    m_commsMemberField = dynamic_cast<CommsField*>(m_layer.memberField());
    assert((m_commsExternalField != nullptr) || (m_layer.externalField() == nullptr));
    assert((m_commsMemberField != nullptr) || (m_layer.memberField() == nullptr));
    return true;
}

bool CommsLayer::commsReorder(CommsLayersList& siblings, bool& success) const
{
    return commsReorderImpl(siblings, success);
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
        "#^#DOC#$#\n"
        "#^#TEMPL_PARAMS#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    #^#BASE#$#;\n";    

    std::string prevName;
    if (prevLayer != nullptr) {
        prevName = comms::className(prevLayer->layer().dslObj().name());
    }

    util::ReplacementMap repl = {
        {"DOC", commsDefDocInternal()},
        {"CLASS_NAME", comms::className(m_layer.dslObj().name())},
        {"BASE", commsDefBaseTypeImpl(prevName, hasInputMessages)},
    };

    hasInputMessages = hasInputMessages || commsDefHasInputMessagesImpl();
    if (hasInputMessages) {
        repl["TEMPL_PARAMS"] = "template <typename TMessage, typename TAllMessages>";
    }

    // TODO: member field code

    return util::processTemplate(Templ, repl);
}

bool CommsLayer::commsIsCustomizable() const
{
    auto& gen = static_cast<CommsGenerator&>(m_layer.generator());
    auto level = gen.getCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }
        
    return commsIsCustomizableImpl();
}

bool CommsLayer::commsReorderImpl(CommsLayersList& siblings, bool& success) const
{
    static_cast<void>(siblings);
    success = true;
    return false;
}

CommsLayer::IncludesList CommsLayer::commsDefIncludesImpl() const
{
    return IncludesList();
}

std::string CommsLayer::commsDefBaseTypeImpl(const std::string& prevName, bool hasInputMessages) const
{
    static_cast<void>(prevName);
    static_cast<void>(hasInputMessages);
    assert(false); // Not implemented in derived class
    return strings::emptyString();
}

bool CommsLayer::commsDefHasInputMessagesImpl() const
{
    return false;
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

bool CommsLayer::commsIsCustomizableImpl() const
{
    return false;
}

} // namespace commsdsl2new
