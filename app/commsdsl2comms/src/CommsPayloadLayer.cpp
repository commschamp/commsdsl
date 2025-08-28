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

#include "CommsPayloadLayer.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsPayloadLayer::CommsPayloadLayer(CommsGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsPayloadLayer::genPrepareImpl()
{
    return GenBase::genPrepareImpl() && CommsBase::commsPrepare();
}

CommsPayloadLayer::CommsIncludesList CommsPayloadLayer::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/frame/MsgDataLayer.h"
    };

    return result;
}

std::string CommsPayloadLayer::commsDefBaseTypeImpl([[maybe_unused]] const std::string& prevName) const
{
    assert(prevName.empty());

    static const std::string Templ =
        "comms::frame::MsgDataLayer<\n"
        "    comms::option::def::FieldType<typename #^#CLASS_NAME#$##^#SUFFIX#$#::Field>\n"
        ">";
    
    util::GenReplacementMap repl {
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
    };

    return util::genProcessTemplate(Templ, repl);    
}

std::string CommsPayloadLayer::commsCustomDefMembersCodeImpl() const
{
    static const std::string Templ = 
        "/// @brief Scope for field(s) of @ref #^#CLASS_NAME#$# layer.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "    /// @brief Custom field for @ref #^#SCOPE#$# layer\n"
        "    class Field : public\n"
        "        comms::field::ArrayList<\n"
        "            #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "            std::uint8_t,\n"
        "            #^#OPTS#$#,\n"
        "            comms::option::def::HasName\n"
        "        >\n"
        "    {\n"
        "    public:\n"
        "        static const char* name()\n"
        "        {\n"
        "            return \"#^#NAME#$#\";\n"
        "        }\n"
        "    };\n"
        "};\n";

    auto& gen = CommsGenerator::commsCast(genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"PROT_NAMESPACE", genGenerator().genSchemaOf(*this).genMainNamespace()},
        {"OPTS", "typename TOpt::" + comms::genScopeFor(*this, gen, gen.commsHasMainNamespaceInOptions(), true) + strings::genMembersSuffixStr() + "::Field"},
        {"NAME", genParseObj().parseName()},
        {"SCOPE", comms::genScopeFor(*this, gen)}
    };     

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsPayloadLayer::commsCustomFieldOptsImpl() const
{
    static const std::string Templ = 
        "/// @brief Extra options for @ref\n"
        "///     #^#SCOPE#$##^#SUFFIX#$#::Field field.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Field = comms::option::app::EmptyOption;\n"
        "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"SCOPE", comms::genScopeFor(*this, genGenerator())}
    };     

    return util::genProcessTemplate(Templ, repl);        
}

std::string CommsPayloadLayer::commsCustomFieldDataViewOptsImpl() const
{
    static const std::string Templ = 
        "/// @brief Extra options for @ref\n"
        "///     #^#SCOPE#$##^#SUFFIX#$#::Field field.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$# : public TBase::#^#OPT_SCOPE#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Field =\n"
        "        std::tuple<\n"
        "            comms::option::app::OrigDataView,\n"
        "            typename TBase::#^#OPT_SCOPE#$##^#SUFFIX#$#::Field\n"
        "        >;\n"
        "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"SCOPE", comms::genScopeFor(*this, genGenerator())},
        {"OPT_SCOPE", comms::genScopeFor(*this, genGenerator(), false, true)}
    };     

    return util::genProcessTemplate(Templ, repl);  
}

std::string CommsPayloadLayer::commsCustomFieldBareMetalOptsImpl() const
{
    static const std::string Templ = 
        "/// @brief Extra options for @ref\n"
        "///     #^#SCOPE#$##^#SUFFIX#$#::Field field.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$# : public TBase::#^#OPT_SCOPE#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Field =\n"
        "        std::tuple<\n"
        "            comms::option::app::FixedSizeStorage<DEFAULT_SEQ_FIXED_STORAGE_SIZE * 8>,\n"
        "            typename TBase::#^#OPT_SCOPE#$##^#SUFFIX#$#::Field\n"
        "        >;\n"
        "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", strings::genMembersSuffixStr()},
        {"SCOPE", comms::genScopeFor(*this, genGenerator())},
        {"OPT_SCOPE", comms::genScopeFor(*this, genGenerator(), false, true)}
    };     

    return util::genProcessTemplate(Templ, repl);  
}

} // namespace commsdsl2comms
