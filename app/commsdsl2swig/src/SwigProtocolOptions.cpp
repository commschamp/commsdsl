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

#include "SwigProtocolOptions.h"

#include "SwigGenerator.h"
#include "SwigSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <limits>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

namespace 
{

std::string swigCodeInternal(const SwigGenerator& generator, std::size_t idx)
{
    assert(idx < generator.schemas().size());

    generator.chooseCurrentSchema(static_cast<unsigned>(idx));
    if (!generator.currentSchema().hasAnyReferencedComponent()) {
        if (idx == 0U) {
            return strings::emptyString();
        }

        return swigCodeInternal(generator, idx - 1U);
    }

    auto scope = comms::scopeForOptions(strings::defaultOptionsClassStr(), generator);

    if (idx == 0U) {
        return scope;
    }

    auto nextScope = swigCodeInternal(generator, idx - 1U);
    if (nextScope.empty()) {
        return scope;
    }    

    static const std::string Templ = 
        "#^#SCOPE#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", std::move(nextScope)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace 

void SwigProtocolOptions::swigAddCodeIncludes(SwigGenerator& generator, StringsList& list)
{
    if (!swigIsDefined(generator)) {
        return;
    }

    assert(generator.isCurrentProtocolSchema());

    list.push_back(comms::relHeaderForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), generator));
    auto& schemas = generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.chooseCurrentSchema(idx);
        if (!generator.currentSchema().hasAnyReferencedComponent()) {
            continue;
        }        
        list.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator));
    }

    generator.chooseProtocolSchema();
}
    

void SwigProtocolOptions::swigAddCode(const SwigGenerator& generator, StringsList& list)
{
    if (!swigIsDefined(generator)) {
        return;
    }

    assert(generator.isCurrentProtocolSchema());

    const std::string Templ = 
        "using #^#OPT_TYPE#$# =\n"
        "    #^#MSG_FACT_OPTS#$#T<\n"
        "        #^#CODE#$#\n"
        "    >;\n\n";

    auto msgFactOptions = comms::scopeForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), generator);
    util::ReplacementMap repl = {
        {"OPT_TYPE", swigClassName(generator)},
        {"CODE", swigCodeInternal(generator, generator.schemas().size() - 1U)},
        {"MSG_FACT_OPTS", std::move(msgFactOptions)}
    };

    generator.chooseProtocolSchema();
    list.push_back(util::processTemplate(Templ, repl));
}

std::string SwigProtocolOptions::swigClassName(const SwigGenerator& generator)
{
    if (!swigIsDefined(generator)) {
        return strings::emptyString();
    }

    return generator.protocolSchema().mainNamespace() + "_ProtocolOptions";
}

bool SwigProtocolOptions::swigIsDefined([[maybe_unused]] const SwigGenerator& generator)
{
    return true;
}

} // namespace commsdsl2swig