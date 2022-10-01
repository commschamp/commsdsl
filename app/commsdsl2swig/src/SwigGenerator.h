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

#include "commsdsl/parse/IntField.h"

#include <string>

namespace commsdsl2swig 
{

class SwigInterface;
class SwigGenerator final : public commsdsl::gen::Generator
{
    using Base = commsdsl::gen::Generator;
    
public:
    using Elem = commsdsl::gen::Elem;
    using FieldPtr = commsdsl::gen::FieldPtr;
    using NamespacePtr = commsdsl::gen::NamespacePtr;
    using InterfacePtr = commsdsl::gen::InterfacePtr;
    using MessagePtr = commsdsl::gen::MessagePtr;
    using FramePtr = commsdsl::gen::FramePtr;
    using LayerPtr = commsdsl::gen::LayerPtr;
    using SchemaPtr = commsdsl::gen::SchemaPtr;
    
    static const std::string& fileGeneratedComment();

    static SwigGenerator& cast(commsdsl::gen::Generator& generator)
    {
        return static_cast<SwigGenerator&>(generator);
    }

    static const SwigGenerator& cast(const commsdsl::gen::Generator& generator)
    {
        return static_cast<const SwigGenerator&>(generator);
    }    
    
    std::string swigInputCodePathFor(const Elem& elem) const;
    std::string swigInputCodePathForFile(const std::string& name) const;
    std::string swigClassName(const Elem& elem) const;
    std::string swigClassNameForRoot(const std::string& name) const;
    std::string swigProtocolClassNameForRoot(const std::string& name) const;
    const std::string& swigConvertCppType(const std::string& str) const;
    const std::string& swigConvertIntType(commsdsl::parse::IntField::Type value, std::size_t len) const;

    static std::string swigScopeToName(const std::string& scope);
    static std::string swigDefInclude(const std::string& path);

    void setMainNamespaceInNamesForced(bool value);
    void setForcedInterface(const std::string& value);

    const SwigInterface* swigMainInterface() const;

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() override;    

    virtual SchemaPtr createSchemaImpl(commsdsl::parse::Schema dslObj, Elem* parent) override;
    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent) override;
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent) override;
    virtual MessagePtr createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent) override;
    virtual FramePtr createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent) override;

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

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;

private:
    bool swigWriteExtraFilesInternal() const;
    bool prepareDefaultInterfaceInternal();

    std::string m_forcedInterface;
    bool m_mainNamespaceInNamesForced = false;
};

} // namespace commsdsl2swig
