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

#include "ChecksumLayer.h"

#include <cassert>
#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "common.h"
#include "Generator.h"

namespace ba = boost::algorithm;

namespace commsdsl2old
{

void ChecksumLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    auto obj = checksumLayerDslObj();
    if (!obj.fromLayer().empty()) {
        assert(obj.untilLayer().empty());
        common::mergeInclude("comms/protocol/ChecksumLayer.h", includes);
    }
    else {
        assert(!obj.untilLayer().empty());
        common::mergeInclude("comms/protocol/ChecksumPrefixLayer.h", includes);
    }

    const std::string ChecksumMap[] = {
        /* Custom */ common::emptyString(),
        /* Sum */ "BasicSum",
        /* Crc_CCITT */ "Crc",
        /* Crc_16 */ "Crc",
        /* Crc_32 */ "Crc"
    };

    const std::size_t ChecksumMapSize = std::extent<decltype(ChecksumMap)>::value;
    static_assert(ChecksumMapSize == static_cast<std::size_t>(commsdsl::parse::ChecksumLayer::Alg::NumOfValues),
            "Invalid map");

    auto idx = static_cast<std::size_t>(obj.alg());
    if (ChecksumMapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        idx = 0U;
    }

    if (!ChecksumMap[idx].empty()) {
        common::mergeInclude("comms/protocol/checksum/" + ChecksumMap[idx] + common::headerSuffix(), includes);
        return;
    }

    assert(!obj.customAlgName().empty());
    common::mergeInclude(generator().headerfileForCustomChecksum(obj.customAlgName(), false), includes);
}

std::string ChecksumLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static const std::string Templ =
        "#^#FIELD_DEF#$#\n"
        "#^#PREFIX#$#\n"
        "#^#TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::Checksum#^#PREFIX_VAR#$#Layer<\n"
        "        #^#FIELD_TYPE#$#,\n"
        "        #^#ALG#$#,\n"
        "        #^#PREV_LAYER#$##^#COMMA#$#\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    auto obj = checksumLayerDslObj();
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("FIELD_DEF", getFieldDefinition(scope)));
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PREV_LAYER", prevLayer));
    replacements.insert(std::make_pair("ALG", getAlg()));

    if (!obj.untilLayer().empty()) {
        replacements.insert(std::make_pair("PREFIX_VAR", "Prefix"));
    }

    if (hasInputMessages) {
        static const std::string TemplParam =
            "template <typename TMessage, typename TAllMessages>";
        replacements.insert(std::make_pair("TEMPL_PARAM", TemplParam));
        replacements["PREV_LAYER"] += "<TMessage, TAllMessages>";
    }

    if (obj.verifyBeforeRead()) {
        replacements.insert(std::make_pair("COMMA", ","));
        replacements.insert(std::make_pair("EXTRA_OPT", "comms::option::def::ChecksumLayerVerifyBeforeRead"));
    }

    prevLayer = common::nameToClassCopy(name());
    return common::processTemplate(Templ, replacements);
}

bool ChecksumLayer::rearangeImpl(Layer::LayersList& layers, bool& success)
{
    if (m_rearanged) {
        success = true;
        return false;
    }

    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [this](auto& l)
            {
                return l.get() == this;
            });

    if (iter == layers.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        generator().logger().error("Internal error");
        return false;
    }

    auto obj = checksumLayerDslObj();
    auto& untilStr = obj.untilLayer();
    do {
        if (untilStr.empty()) {
            break;
        }

        assert(obj.fromLayer().empty());
        auto untilIter =
            std::find_if(
                layers.begin(), layers.end(),
                [&untilStr](auto& l)
                {
                    return l->name() == untilStr;
                });

        if (untilIter == layers.end()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            generator().logger().error("Internal error");
            success = false;
            return false;
        }

        if ((*untilIter)->kind() != commsdsl::parse::Layer::Kind::Payload) {
            generator().logger().error("Checksum prefix must be until payload layer");
            success = false;
            return false;
        }

        success = true;
        m_rearanged = true;
        return false;
    } while (false);

    auto& fromStr = obj.fromLayer();
    if (fromStr.empty()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        generator().logger().error("Info on checksum layer is missing");
        success = false;
        return false;
    }

    auto fromIter =
        std::find_if(
            layers.begin(), layers.end(),
            [&fromStr](auto& l)
            {
                return l->name() == fromStr;
            });

    if (fromIter == layers.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        generator().logger().error("Internal error");
        success = false;
        return false;
    }


    auto thisPtr = std::move(*iter);
    layers.erase(iter);
    layers.insert(fromIter, std::move(thisPtr));
    success = true;
    m_rearanged = true;
    return true;
}

std::string ChecksumLayer::getAlg() const
{
    const std::string ClassMap[] = {
        /* Custom */ common::emptyString(),
        /* Sum */ "BasicSum",
        /* Crc_CCITT */ "Crc_CCITT",
        /* Crc_16 */ "Crc_16",
        /* Crc_32 */ "Crc_32"
    };

    const std::size_t ClassMapSize = std::extent<decltype(ClassMap)>::value;
    static_assert(ClassMapSize == static_cast<std::size_t>(commsdsl::parse::ChecksumLayer::Alg::NumOfValues),
            "Invalid map");


    auto obj = checksumLayerDslObj();
    auto alg = obj.alg();
    auto idx = static_cast<std::size_t>(alg);

    if (ClassMapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        idx = 0U;
    }

    if (ClassMap[idx].empty()) {
        assert(!obj.customAlgName().empty());
        return generator().scopeForCustomChecksum(obj.customAlgName(), true, true);
    }

    auto str = "comms::protocol::checksum::" + ClassMap[idx];
    if (alg != commsdsl::parse::ChecksumLayer::Alg::Sum) {
        return str;
    }

    auto fieldType = getFieldType();
    if (!ba::starts_with(fieldType, "typename")) {
        fieldType = "typename " + fieldType;
    }

    return str + "<" + fieldType  + "::ValueType>";
}

} // namespace commsdsl2old