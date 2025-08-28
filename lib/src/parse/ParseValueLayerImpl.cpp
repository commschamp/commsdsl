//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ParseValueLayerImpl.h"

#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>
#include <map>

namespace commsdsl
{

namespace parse
{

ParseValueLayerImpl::ParseValueLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol),
    m_fieldName(&common::parseEmptyString())
{
}

ParseValueLayerImpl::ParseInterfacesList ParseValueLayerImpl::parseInterfacesList() const
{
    ParseInterfacesList result;
    result.reserve(m_interfaces.size());
    for (auto* i : m_interfaces) {
        result.push_back(ParseInterface(i));
    }
    return result;
}

std::size_t ParseValueLayerImpl::parseFieldIdx() const
{
    if (m_interfaces.empty()) {
        return std::numeric_limits<std::size_t>::max();
    }

    auto* interface = m_interfaces.front();
    assert(!parseFieldName().empty());
    return interface->parseFindFieldIdx(parseFieldName());
}

ParseLayerImpl::ParseKind ParseValueLayerImpl::parseKindImpl() const
{
    return ParseKind::Value;
}

const ParseXmlWrap::ParseNamesList& ParseValueLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseInterfacesStr(),
        common::parseInterfaceFieldNameStr(),
        common::pseudoStr()
    };
    return Names;
}

bool ParseValueLayerImpl::parseImpl()
{
    return
        parseUpdateInterfaces() &&
        parseUpdateFieldName() &&
        parseUpdatePseudo();
}

bool ParseValueLayerImpl::parseVerifyImpl(const ParseLayerImpl::ParseLayersList& layers)
{
    return parseVerifyBeforePayload(layers);
}

bool ParseValueLayerImpl::parseUpdateInterfaces()
{
    if (!parseValidateSinglePropInstance(common::parseInterfacesStr())) {
        return false;
    }

    do {
        auto iter = parseProps().find(common::parseInterfacesStr());
        if (iter == parseProps().end()) {
            auto& namespaces = parseProtocol().parseCurrSchema().parseNamespaces();
            for (auto& n : namespaces) {
                auto& interfaces = n.second->parseInterfaces();
                for (auto& i : interfaces) {
                    m_interfaces.push_back(i.second.get());
                }
            }

            break;
        }

        if (iter->second.empty()) {
            parseReportUnexpectedPropertyValue(common::parseInterfacesStr(), iter->second);
            return false;
        }

        std::size_t pos = 0;
        while (true) {
            auto commaPos = iter->second.find(',', pos);
            std::string ref(iter->second, pos, commaPos - pos);
            common::parseRemoveHeadingTrailingWhitespaces(ref);
            if (ref.empty()) {
                parseReportUnexpectedPropertyValue(common::parseInterfacesStr(), iter->second);
                return false;
            }

            auto interface = parseProtocol().parseFindInterface(ref);
            if (interface == nullptr) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                    "Unknown interface \"" << ref << "\".";
                return false;
            }

            m_interfaces.push_back(interface);
            if (commaPos == std::string::npos) {
                break;
            }
            pos = commaPos + 1;
        }
    } while (false);

    if (m_interfaces.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "No valid interfaces have been defined.";
        return false;
    }
    return true;
}

bool ParseValueLayerImpl::parseUpdateFieldName()
{
    if (!parseValidateSinglePropInstance(common::parseInterfaceFieldNameStr(), true)) {
        return false;
    }

    auto iter = parseProps().find(common::parseInterfaceFieldNameStr());
    assert(iter != parseProps().end());
    m_fieldName = &iter->second;
    if (parseFieldName().empty()) {
        parseReportUnexpectedPropertyValue(common::parseInterfaceFieldNameStr(), parseFieldName());
        return false;
    }

    assert(!m_interfaces.empty());

    static const auto InvalidIdx = std::numeric_limits<std::size_t>::max();
    std::size_t idx = InvalidIdx;
    for (auto i = 0U ; i < m_interfaces.size(); ++i) {
        auto* interface = m_interfaces[i];
        assert(interface != nullptr);
        auto fieldIdx = interface->parseFindFieldIdx(parseFieldName());
        if (fieldIdx == InvalidIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Interface \"" << interface->parseName() << "\" doesn't contain "
                "field named \"" << parseFieldName() << "\"";
            return false;
        }

        if (idx == InvalidIdx) {
            idx = fieldIdx;
            continue;
        }

        if (idx != fieldIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Index of field \"" << parseFieldName() << "\" (" << fieldIdx << ") in \"" <<
                interface->parseName() << "\" interface differs from expected (" << idx << ").";
            return false;
        }
    }
    return true;
}

bool ParseValueLayerImpl::parseUpdatePseudo()
{
    if (!parseValidateSinglePropInstance(common::pseudoStr())) {
        return false;
    }

    auto iter = parseProps().find(common::pseudoStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_pseudo = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::pseudoStr(), iter->second);
        return false;
    }
    return true;
}

} // namespace parse

} // namespace commsdsl
