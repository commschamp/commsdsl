//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "CommsGenerator.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsPayloadLayer::CommsPayloadLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsPayloadLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::commsPrepare();
}

CommsPayloadLayer::IncludesList CommsPayloadLayer::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/protocol/MsgDataLayer.h"
    };

    return result;
}

std::string CommsPayloadLayer::commsDefBaseTypeImpl([[maybe_unused]] const std::string& prevName) const
{
    assert(prevName.empty());

    static const std::string Templ =
        "comms::protocol::MsgDataLayer<\n"
        "    comms::option::def::FieldType<typename #^#CLASS_NAME#$##^#SUFFIX#$#::Field>\n"
        ">";
    
    util::ReplacementMap repl {
        {"SUFFIX", strings::membersSuffixStr()},
        {"CLASS_NAME", comms::className(dslObj().name())},
    };

    return util::processTemplate(Templ, repl);    
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

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"PROT_NAMESPACE", generator().schemaOf(*this).mainNamespace()},
        {"OPTS", "typename TOpt::" + comms::scopeFor(*this, generator(), false, true) + strings::membersSuffixStr() + "::Field"},
        {"NAME", dslObj().name()},
        {"SCOPE", comms::scopeFor(*this, generator())}
    };     

    return util::processTemplate(Templ, repl);
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

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"SCOPE", comms::scopeFor(*this, generator())}
    };     

    return util::processTemplate(Templ, repl);        
}

std::string CommsPayloadLayer::commsCustomFieldDataViewOptsImpl() const
{
    static const std::string Templ = 
        "/// @brief Extra options for @ref\n"
        "///     #^#SCOPE#$##^#SUFFIX#$#::Field field.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$# : public TBase::#^#OPT_SCOPE#$#\n"
        "{\n"
        "    using Field =\n"
        "        std::tuple<\n"
        "            comms::option::app::OrigDataView,\n"
        "            typename TBase::#^#OPT_SCOPE#$#::Field\n"
        "        >;\n"
        "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"OPT_SCOPE", comms::scopeFor(*this, generator(), false, true)}
    };     

    return util::processTemplate(Templ, repl);  
}

std::string CommsPayloadLayer::commsCustomFieldBareMetalOptsImpl() const
{
    static const std::string Templ = 
        "/// @brief Extra options for @ref\n"
        "///     #^#SCOPE#$##^#SUFFIX#$#::Field field.\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$# : public TBase::#^#OPT_SCOPE#$#\n"
        "{\n"
        "    using Field =\n"
        "        std::tuple<\n"
        "            comms::option::app::FixedSizeStorage<DEFAULT_SEQ_FIXED_STORAGE_SIZE * 8>,\n"
        "            typename TBase::#^#OPT_SCOPE#$#::Field\n"
        "        >;\n"
        "}; // struct #^#CLASS_NAME#$##^#SUFFIX#$#\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"OPT_SCOPE", comms::scopeFor(*this, generator(), false, true)}
    };     

    return util::processTemplate(Templ, repl);  
}

} // namespace commsdsl2comms
