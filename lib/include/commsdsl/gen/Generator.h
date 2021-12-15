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

#pragma once

#include <functional>

#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/Field.h"

namespace commsdsl
{

namespace gen
{

class Generator
{
public:

    Generator();
    virtual ~Generator();

    NamespacePtr createNamespace(commsdsl::parse::Namespace dslObj, Elem* parent = nullptr);
    FieldPtr createIntField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createEnumField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createSetField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createFloatField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createBitfieldField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createBundleField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createStringField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createDataField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createListField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createRefField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createOptionalField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    FieldPtr createVariantField(commsdsl::parse::Field dslObj, Elem* parent = nullptr);

protected:
    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent);
    virtual FieldPtr createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
private:
    
};

} // namespace gen

} // namespace commsdsl
