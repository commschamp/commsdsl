//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "PayloadLayer.h"

#include <cassert>

#include "common.h"

namespace commsdsl2comms
{

void PayloadLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/MsgDataLayer.h"
    };

    common::mergeIncludes(List, includes);
}

std::string PayloadLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static_cast<void>(hasInputMessages);
    assert(!hasInputMessages);
    assert(prevLayer.empty());
    prevLayer = common::nameToClassCopy(name());

    static const std::string Templ =
        "#^#PREFIX#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::MsgDataLayer<\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_OPT", getExtraOpt(scope)));
    return common::processTemplate(Templ, replacements);
}

std::string PayloadLayer::getBareMetalOptionStrImpl(const std::string& base) const
{
    static const std::string Str("comms::option::app::FixedSizeStorage<" + common::seqDefaultSizeStr() + " * 8>");
    if (base.empty()) {
        return Str;
    }
    
    return 
        "std::tuple<\n"
        "    " + Str + ",\n"
        "    typename " + base + "::" + common::nameToClassCopy(name()) + "\n"
        ">";
}

std::string PayloadLayer::getDataViewOptionStrImpl(const std::string& base) const
{
    static const std::string Str("comms::option::app::OrigDataView");
    if (base.empty()) {
        return Str;
    }
    
    return 
        "std::tuple<\n"
        "    " + Str + ",\n"
        "    typename " + base + "::" + common::nameToClassCopy(name()) + "\n"
        ">";
}

bool PayloadLayer::isCustomizableImpl() const
{
    return true;
}

} // namespace commsdsl2comms
