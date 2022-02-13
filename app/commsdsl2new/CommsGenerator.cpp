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

#include "CommsGenerator.h"

#include "CommsBitfieldField.h"
#include "CommsCmake.h"
#include "CommsEnumField.h"
#include "CommsFloatField.h"
#include "CommsIntField.h"
#include "CommsMessage.h"
#include "CommsMsgId.h"
#include "CommsSetField.h"
#include "CommsStringField.h"

#include "commsdsl/version.h"

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace commsdsl2new
{

const std::string& CommsGenerator::fileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2new v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

CommsGenerator::CustomizationLevel CommsGenerator::getCustomizationLevel() const
{
    return m_customizationLevel;
}

void CommsGenerator::setCustomizationLevel(const std::string& value)
{
    if (value.empty()) {
        return;
    }

    static const std::string Map[] = {
        /* Full */ "full",
        /* Limited */ "limited",
        /* None */ "none",        
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(CustomizationLevel::NumOfValues));

    auto iter = std::find(std::begin(Map), std::end(Map), value);
    if (iter == std::end(Map)) {
        logger().warning("Unknown customization level \"" + value + "\", using default.");
        return;
    }

    m_customizationLevel = static_cast<CustomizationLevel>(std::distance(std::begin(Map), iter));
}

CommsGenerator::MessagePtr CommsGenerator::createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsMessage>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsIntField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsEnumField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsSetField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsFloatField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsBitfieldField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2new::CommsStringField>(*this, dslObj, parent);
}

bool CommsGenerator::writeImpl()
{
    return 
        CommsCmake::write(*this) &&
        CommsMsgId::write(*this);
}

} // namespace commsdsl2new
