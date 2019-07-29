//
// Copyright 2019 (C). Alex Robenko. All rights reserved.
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

#include "VersionConfigWidget.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool VersionConfigWidget::write(Generator& generator)
{
    VersionConfigWidget obj(generator);
    return obj.writeHeader() /*&& obj.writeSrc()*/;
}

bool VersionConfigWidget::writeHeader() const
{
    auto startInfo = m_generator.startProtocolPluginHeaderWrite(common::versionConfigWidgetStr());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#pragma once\n\n"
        "#include <QtWidgets/QWidget>\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "class #^#CLASS_NAME#$# : public QWidget\n"
        "{\n"
        "public:\n"
        "    explict #^#CLASS_NAME#$#(int version);\n"
        "signals:\n"
        "    void versionChanged(int);\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n"
    ;

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginHeaderInPlugin(common::versionConfigWidgetStr())));

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2comms
