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

#include "CommsDefaultOptions.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

using NamespaceOptionsFunc = std::string (CommsNamespace::*)() const;
std::string optionsBodyInternal(
    CommsGenerator& generator,
    NamespaceOptionsFunc nsFunc)
{
    auto& allNs = generator.namespaces();
    util::StringsList opts;
    for (auto& nsPtr : allNs) {
        auto elem = (static_cast<const CommsNamespace*>(nsPtr.get())->*nsFunc)();
        if (!elem.empty()) {
            opts.push_back(std::move(elem));
        }
    }

    return util::strListToString(opts, "\n", "");
}

bool writeFileInternal(
    const std::string& name,
    CommsGenerator& generator,
    const std::string& data)
{
    auto filePath = comms::headerPathForOptions(name, generator);
    generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }      

    std::ofstream stream(filePath);
    if (!stream) {
        generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    stream << data;
    stream.flush();
    return stream.good();
}

const std::string& extOptionsTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol #^#DESC#$# default options.\n\n"
        "#pragma once\n\n"
        "#include \"#^#PROT_NAMESPACE#$#/options/DefaultOptions.h\"\n\n"
        "#^#EXTRA#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "/// @brief Default #^#DESC#$# options of the protocol.\n"
        "template <typename TBase = #^#DEFAULT_OPTS#$#>\n"
        "struct #^#NAME#$#DefaultOptionsT : public TBase\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#NAME#$#DefaultOptionsT with default template parameter.\n"
        "using #^#NAME#$#DefaultOptions#^#ORIG#$# = #^#NAME#$#DefaultOptionsT<>;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    return Templ;
}

util::ReplacementMap extInitialRepl(CommsGenerator& generator)
{
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", generator.mainNamespace()},
        {"DEFAULT_OPTS", comms::scopeForOptions(strings::defaultOptionsClassStr(), generator)}
    };
    return repl;
}

} // namespace 
    

bool CommsDefaultOptions::write(CommsGenerator& generator)
{
    CommsDefaultOptions obj(generator);
    return obj.commsWriteInternal();
}

bool CommsDefaultOptions::commsWriteInternal() const
{
    return
        commsWriteDefaultOptionsInternal() &&
        commsWriteClientDefaultOptionsInternal() &&
        commsWriteServerDefaultOptionsInternal() &&
        commsWriteDataViewDefaultOptionsInternal() &&
        commsWriteBareMetalDefaultOptionsInternal();
}

bool CommsDefaultOptions::commsWriteDefaultOptionsInternal() const
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol default options.\n\n"
        "#pragma once\n\n"
        "#include \"comms/options.h\"\n\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "/// @brief Default (empty) options of the protocol.\n"
        "struct #^#CLASS_NAME#$##^#ORIG#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    auto& name = strings::defaultOptionsClassStr();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.mainNamespace()},
        {"CLASS_NAME", name},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsDefaultOptions)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(strings::defaultOptionsClassStr(), m_generator, util::processTemplate(Templ, repl));
    return true;
}

bool CommsDefaultOptions::commsWriteClientDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = "Client" + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "client"},
        {"NAME", "Client"},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsClientDefaultOptions)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl));
    return true;
}

bool CommsDefaultOptions::commsWriteServerDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = "Server" + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "server"},
        {"NAME", "Server"},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsServerDefaultOptions)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl));
    return true;
}

bool CommsDefaultOptions::commsWriteDataViewDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = strings::dataViewStr() + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "data view"},
        {"NAME", strings::dataViewStr()},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsDataViewDefaultOptions)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl));
    return true;
}

bool CommsDefaultOptions::commsWriteBareMetalDefaultOptionsInternal() const
{
    std::string extra = 
        "#ifndef DEFAULT_SEQ_FIXED_STORAGE_SIZE\n"
        "/// @brief Define default fixed size for various sequence fields\n"
        "/// @details May be defined during compile time to change the default value.\n"
        "#define DEFAULT_SEQ_FIXED_STORAGE_SIZE 32\n"
        "#endif\n";

    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = strings::bareMetalStr() + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "bare metal"},
        {"NAME", strings::bareMetalStr()},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsBareMetalDefaultOptions)},
        {"EXTRA", std::move(extra)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl));
    return true;
}

} // namespace commsdsl2comms