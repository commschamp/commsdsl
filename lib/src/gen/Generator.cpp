//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Generator.h"

#include "commsdsl/gen/BitfieldField.h"
#include "commsdsl/gen/BundleField.h"
#include "commsdsl/gen/DataField.h"
#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/FloatField.h"
#include "commsdsl/gen/IntField.h"
#include "commsdsl/gen/ListField.h"
#include "commsdsl/gen/OptionalField.h"
#include "commsdsl/gen/RefField.h"
#include "commsdsl/gen/SetField.h"
#include "commsdsl/gen/StringField.h"
#include "commsdsl/gen/VariantField.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

Generator::Generator()
{
}

Generator::~Generator() = default;

NamespacePtr Generator::createNamespace(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return createNamespaceImpl(dslObj, parent);
}

FieldPtr Generator::createIntField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Int);
    return createIntFieldImpl(dslObj, parent);
}

FieldPtr Generator::createEnumField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Enum);
    return createEnumFieldImpl(dslObj, parent);
}

FieldPtr Generator::createSetField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Set);
    return createSetFieldImpl(dslObj, parent);
}

FieldPtr Generator::createFloatField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Float);
    return createFloatFieldImpl(dslObj, parent);
}

FieldPtr Generator::createBitfieldField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Bitfield);
    return createBitfieldFieldImpl(dslObj, parent);
}

FieldPtr Generator::createBundleField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Bundle);
    return createBundleFieldImpl(dslObj, parent);
}

FieldPtr Generator::createStringField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::String);
    return createStringFieldImpl(dslObj, parent);
}

FieldPtr Generator::createDataField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Data);
    return createStringFieldImpl(dslObj, parent);
}

FieldPtr Generator::createListField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::List);
    return createListFieldImpl(dslObj, parent);
}

FieldPtr Generator::createRefField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Ref);
    return createRefFieldImpl(dslObj, parent);
}

FieldPtr Generator::createOptionalField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Optional);
    return createOptionalFieldImpl(dslObj, parent);
}

FieldPtr Generator::createVariantField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Variant);
    return createVariantFieldImpl(dslObj, parent);
}

NamespacePtr Generator::createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return std::make_unique<Namespace>(*this, dslObj, parent);
}

FieldPtr Generator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<IntField>(*this, dslObj, parent);
}

FieldPtr Generator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EnumField>(*this, dslObj, parent);
}

FieldPtr Generator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SetField>(*this, dslObj, parent);
}

FieldPtr Generator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<FloatField>(*this, dslObj, parent);
}

FieldPtr Generator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<BitfieldField>(*this, dslObj, parent);
}

FieldPtr Generator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<BundleField>(*this, dslObj, parent);
}

FieldPtr Generator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<StringField>(*this, dslObj, parent);
}

FieldPtr Generator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<DataField>(*this, dslObj, parent);
}

FieldPtr Generator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<ListField>(*this, dslObj, parent);
}

FieldPtr Generator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<RefField>(*this, dslObj, parent);
}

FieldPtr Generator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<OptionalField>(*this, dslObj, parent);
}

FieldPtr Generator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<VariantField>(*this, dslObj, parent);
}

} // namespace gen

} // namespace commsdsl
