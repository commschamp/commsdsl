//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "ValueLayerImpl.h"

#include <cassert>
#include <map>

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

namespace parse
{

ValueLayerImpl::ValueLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol),
    m_fieldName(&common::emptyString())
{
}

ValueLayerImpl::InterfacesList ValueLayerImpl::interfacesList() const
{
    InterfacesList result;
    result.reserve(m_interfaces.size());
    for (auto* i : m_interfaces) {
        result.push_back(Interface(i));
    }
    return result;
}

std::size_t ValueLayerImpl::fieldIdx() const
{
    if (m_interfaces.empty()) {
        return std::numeric_limits<std::size_t>::max();
    }

    auto* interface = m_interfaces.front();
    assert(!fieldName().empty());
    return interface->findFieldIdx(fieldName());
}

LayerImpl::Kind ValueLayerImpl::kindImpl() const
{
    return Kind::Value;
}

const XmlWrap::NamesList& ValueLayerImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList Names = {
        common::interfacesStr(),
        common::interfaceFieldNameStr(),
        common::pseudoStr()
    };
    return Names;
}

bool ValueLayerImpl::parseImpl()
{
    return
        updateInterfaces() &&
        updateFieldName() &&
        updatePseudo();
}

bool ValueLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    return verifyBeforePayload(layers);
}

bool ValueLayerImpl::updateInterfaces()
{
    if (!validateSinglePropInstance(common::interfacesStr())) {
        return false;
    }

    do {
        auto iter = props().find(common::interfacesStr());
        if (iter == props().end()) {
            auto& namespaces = protocol().currSchema().namespaces();
            for (auto& n : namespaces) {
                auto& interfaces = n.second->interfaces();
                for (auto& i : interfaces) {
                    m_interfaces.push_back(i.second.get());
                }
            }

            break;
        }

        if (iter->second.empty()) {
            reportUnexpectedPropertyValue(common::interfacesStr(), iter->second);
            return false;
        }

        std::size_t pos = 0;
        while (true) {
            auto commaPos = iter->second.find(',', pos);
            std::string ref(iter->second, pos, commaPos - pos);
            common::removeHeadingTrailingWhitespaces(ref);
            if (ref.empty()) {
                reportUnexpectedPropertyValue(common::interfacesStr(), iter->second);
                return false;
            }

            auto interface = protocol().findInterface(ref);
            if (interface == nullptr) {
                logError() << XmlWrap::logPrefix(getNode()) <<
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "No valid interfaces have been defined.";
        return false;
    }
    return true;
}

bool ValueLayerImpl::updateFieldName()
{
    if (!validateSinglePropInstance(common::interfaceFieldNameStr(), true)) {
        return false;
    }

    auto iter = props().find(common::interfaceFieldNameStr());
    assert(iter != props().end());
    m_fieldName = &iter->second;
    if (fieldName().empty()) {
        reportUnexpectedPropertyValue(common::interfaceFieldNameStr(), fieldName());
        return false;
    }

    assert(!m_interfaces.empty());

    static const auto InvalidIdx = std::numeric_limits<std::size_t>::max();
    std::size_t idx = InvalidIdx;
    for (auto i = 0U ; i < m_interfaces.size(); ++i) {
        auto* interface = m_interfaces[i];
        assert(interface != nullptr);
        auto fieldIdx = interface->findFieldIdx(fieldName());
        if (fieldIdx == InvalidIdx) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Interface \"" << interface->name() << "\" doesn't contain "
                "field named \"" << fieldName() << "\"";
            return false;
        }

        if (idx == InvalidIdx) {
            idx = fieldIdx;
            continue;
        }

        if (idx != fieldIdx) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Index of field \"" << fieldName() << "\" (" << fieldIdx << ") in \"" <<
                interface->name() << "\" interface differs from expected (" << idx << ").";
            return false;
        }
    }
    return true;
}

bool ValueLayerImpl::updatePseudo()
{
    if (!validateSinglePropInstance(common::pseudoStr())) {
        return false;
    }

    auto iter = props().find(common::pseudoStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_pseudo = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::pseudoStr(), iter->second);
        return false;
    }
    return true;
}

} // namespace parse

} // namespace commsdsl
