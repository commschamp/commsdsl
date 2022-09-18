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

std::string swigCodeInternal(SwigGenerator& generator, std::size_t idx)
{
    assert(idx < generator.schemas().size());

    generator.chooseCurrentSchema(static_cast<unsigned>(idx));
    auto scope = comms::scopeForOptions(strings::defaultOptionsClassStr(), generator);

    if (idx == 0U) {
        return scope;
    }

    static const std::string Templ = 
        "#^#SCOPE#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", swigCodeInternal(generator, idx - 1U)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace 

void SwigProtocolOptions::swigAddCodeIncludes(const SwigGenerator& generator, StringsList& list)
{
    if (!swigIsDefined(generator)) {
        return;
    }

    assert(generator.isCurrentProtocolSchema());

    auto& gen = const_cast<SwigGenerator&>(generator);
    auto& schemas = gen.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        gen.chooseCurrentSchema(idx);
        list.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator));
    }

    gen.chooseProtocolSchema();
}
    

void SwigProtocolOptions::swigAddCode(const SwigGenerator& generator, StringsList& list)
{
    if (!swigIsDefined(generator)) {
        return;
    }

    assert(generator.isCurrentProtocolSchema());

    auto& gen = const_cast<SwigGenerator&>(generator);

    const std::string Templ = 
        "using #^#SWIG_TYPE#$# =\n"
        "    #^#CODE#$#;\n\n";

    auto commsType = comms::scopeForRoot(strings::msgIdEnumNameStr(), generator);
    util::ReplacementMap repl = {
        {"SWIG_TYPE", swigClassName(generator)},
        {"CODE", swigCodeInternal(gen, gen.schemas().size() - 1U)}
    };

    gen.chooseProtocolSchema();
    list.push_back(util::processTemplate(Templ, repl));
}

std::string SwigProtocolOptions::swigClassName(const SwigGenerator& generator)
{
    if (!swigIsDefined(generator)) {
        return strings::emptyString();
    }

    return generator.protocolSchema().mainNamespace() + "_ProtocolOptions";
}

bool SwigProtocolOptions::swigIsDefined(const SwigGenerator& generator)
{
    return 1U < generator.schemas().size();
}

} // namespace commsdsl2swig