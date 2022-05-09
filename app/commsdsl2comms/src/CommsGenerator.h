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

namespace commsdsl2comms 
{

class CommsGenerator final : public commsdsl::gen::Generator
{
    using Base = commsdsl::gen::Generator;
public:
    using Elem = commsdsl::gen::Elem;
    using FieldPtr = commsdsl::gen::FieldPtr;
    using FramePtr = commsdsl::gen::FramePtr;
    using InterfacePtr = commsdsl::gen::InterfacePtr;
    using LayerPtr = commsdsl::gen::LayerPtr;
    using MessagePtr = commsdsl::gen::MessagePtr;
    using NamespacePtr = commsdsl::gen::NamespacePtr;
    using ExtraMessageBundle = std::pair<std::string, MessagesAccessList>;
    using ExtraMessageBundlesList = std::vector<ExtraMessageBundle>;

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

    const std::string& getProtocolVersion() const;
    void setProtocolVersion(const std::string& value);

    const std::vector<std::string>& getExtraInputBundles() const;
    void setExtraInputBundles(const std::vector<std::string>& inputBundles);
    const ExtraMessageBundlesList& extraMessageBundles() const;

    static const std::string& minCommsVersion();


protected:
    virtual bool prepareImpl() override;

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

    virtual bool writeImpl() override;    

private:
    bool prepareDefaultInterfaceInternal();
    bool prepareExtraMessageBundlesInternal();
    bool commsWriteExtraFilesInternal() const;
    
    static const CustomizationLevel DefaultCustomizationLevel = CustomizationLevel::Limited;
    CustomizationLevel m_customizationLevel = DefaultCustomizationLevel;    
    std::string m_protocolVersion;
    std::vector<std::string> m_extraInputBundles;
    ExtraMessageBundlesList m_extraMessageBundles;
};

} // namespace commsdsl2comms
