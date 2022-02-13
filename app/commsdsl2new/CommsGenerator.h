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

#pragma once

#include "commsdsl/gen/Generator.h"

namespace commsdsl2new 
{

class CommsGenerator final : public commsdsl::gen::Generator
{
public:
    using Elem = commsdsl::gen::Elem;
    using FieldPtr = commsdsl::gen::FieldPtr;
    using MessagePtr = commsdsl::gen::MessagePtr;

    enum class CustomizationLevel
    {
        Full,
        Limited,
        None,
        NumOfValues
    };    

    static const std::string& fileGeneratedComment();
    CustomizationLevel getCustomizationLevel() const;
    void setCustomizationLevel(const std::string& opt);

protected:
    // virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent);
    // virtual InterfacePtr createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent);
    virtual MessagePtr createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent) override;
    // virtual FramePtr createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent);

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    // virtual FieldPtr createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    // virtual FieldPtr createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    // virtual FieldPtr createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    // virtual FieldPtr createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    // virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    // virtual FieldPtr createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);

    // virtual LayerPtr createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    // virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);

    virtual bool writeImpl() override;    

private:
    static const CustomizationLevel DefaultCustomizationLevel = CustomizationLevel::Limited;
    CustomizationLevel m_customizationLevel = DefaultCustomizationLevel;    
};

} // namespace commsdsl2new
