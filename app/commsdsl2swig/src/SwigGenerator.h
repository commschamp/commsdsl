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
    using NamespacePtr = commsdsl::gen::NamespacePtr;
    
    static const std::string& fileGeneratedComment();

    static SwigGenerator& cast(commsdsl::gen::Generator& generator)
    {
        return static_cast<SwigGenerator&>(generator);
    }

    static const SwigGenerator& cast(const commsdsl::gen::Generator& generator)
    {
        return static_cast<const SwigGenerator&>(generator);
    }    
    
    std::string swigInputCodePathForFile(const std::string& name) const;
    std::string swigClassName(const Elem& elem) const;
    static std::string swigScopeToName(const std::string& scope);

    void setMainNamespaceInNamesForced(bool value);

protected:
    virtual bool writeImpl() override;    

    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent) override;

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

    bool m_mainNamespaceInNamesForced = false;
};

} // namespace commsdsl2swig
