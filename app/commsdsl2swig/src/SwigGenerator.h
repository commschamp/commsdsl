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

#include "commsdsl/gen/Generator.h"

namespace commsdsl2swig 
{

class SwigGenerator final : public commsdsl::gen::Generator
{
public:
    using Elem = commsdsl::gen::Elem;
    using FieldPtr = commsdsl::gen::FieldPtr;
    
    static const std::string& fileGeneratedComment();

protected:
    virtual bool writeImpl() override;    

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;    

private:
    bool swigWriteExtraFilesInternal() const;
};

} // namespace commsdsl2swig
