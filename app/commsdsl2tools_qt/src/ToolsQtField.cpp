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

#include "ToolsQtField.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtField::ToolsQtField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

ToolsQtField::~ToolsQtField() = default;

ToolsQtField::ToolsQtFieldsList ToolsQtField::toolsTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    ToolsQtFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* toolsField = 
            const_cast<ToolsQtField*>(
                dynamic_cast<const ToolsQtField*>(fPtr.get()));

        assert(toolsField != nullptr);
        result.push_back(toolsField);
    }

    return result;
}

bool ToolsQtField::toolsIsPseudo() const
{
    if (m_forcedPseudo || m_field.dslObj().isPseudo()) {
        return true;
    }

    auto* parent = m_field.getParent();
    while (parent != nullptr) {
        if (parent->elemType() != commsdsl::gen::Elem::Type_Field) {
            break;
        }

        auto* parentField = dynamic_cast<const ToolsQtField*>(parent);
        assert(parentField != nullptr);
        if (parentField->toolsIsPseudo()) {
            return true;
        }

        parent = parent->getParent();
    }

    return false;
}

std::string ToolsQtField::toolsCommsScope(const std::string& extraTemplParams) const
{
    auto parent = m_field.getParent();
    while (parent != nullptr) {
        if (parent->elemType() == commsdsl::gen::Elem::Type::Type_Layer) {
            parent = parent->getParent();
        }

        if (parent->elemType() != commsdsl::gen::Elem::Type::Type_Field) {
            break;
        }

        if (comms::isGlobalField(*parent)) {
            break;
        }

        parent = parent->getParent();
    }

    assert(parent != nullptr);

    auto& generator = static_cast<const ToolsQtGenerator&>(m_field.generator());
    std::string scope = comms::scopeFor(m_field, generator);
    bool globalField = comms::isGlobalField(m_field);
    do {
        if (globalField) {
            scope += ToolsQtDefaultOptions::toolsTemplParam(generator, extraTemplParams);
            break;
        }

        auto parentScope = comms::scopeFor(*parent, generator);

        if ((parent->elemType() == commsdsl::gen::Elem::Type::Type_Message) ||
            (parent->elemType() == commsdsl::gen::Elem::Type::Type_Interface)) {
            parentScope += strings::fieldsSuffixStr();
        }    
        else if (parent->elemType() == commsdsl::gen::Elem::Type::Type_Frame) {
            parentScope += strings::layersSuffixStr();
        }          
        else {
            parentScope += strings::membersSuffixStr();
        }

        assert(parentScope.size() <= scope.size());
        auto suffix = scope.substr(parentScope.size());
        if (parent->elemType() != commsdsl::gen::Elem::Type::Type_Interface) {
            parentScope += ToolsQtDefaultOptions::toolsTemplParam(generator, extraTemplParams);
        }

        scope = parentScope + suffix;
    } while (false);

    return scope;
}

} // namespace commsdsl2tools_qt
