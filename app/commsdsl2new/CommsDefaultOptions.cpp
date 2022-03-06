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

namespace commsdsl2new
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

} // namespace 
    

bool CommsDefaultOptions::write(CommsGenerator& generator)
{
    CommsDefaultOptions obj(generator);
    return obj.writeInternal();
}

bool CommsDefaultOptions::writeInternal() const
{
    static_cast<void>(m_generator);
    return
        writeDefaultOptionsInternal();
}

bool CommsDefaultOptions::writeDefaultOptionsInternal() const
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
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.mainNamespace()},
        {"CLASS_NAME", strings::defaultOptionsClassStr()},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsDefaultOptions)}
    };

    writeFileInternal(strings::defaultOptionsClassStr(), m_generator, util::processTemplate(Templ, repl));
    return true;
}

} // namespace commsdsl2new