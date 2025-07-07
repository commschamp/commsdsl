//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenGenerator.h"

namespace commsdsl2comms 
{

class CommsGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;
public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using ExtraMessageBundle = std::pair<std::string, GenMessagesAccessList>;
    using ExtraMessageBundlesList = std::vector<ExtraMessageBundle>;

    enum class CustomizationLevel
    {
        Full,
        Limited,
        None,
        NumOfValues
    };    

    static const CommsGenerator& cast(const commsdsl::gen::GenGenerator& ref)
    {
        return static_cast<const CommsGenerator&>(ref);
    }

    static const std::string& commsFileGeneratedComment();
    CustomizationLevel commsGetCustomizationLevel() const;
    void commsSetCustomizationLevel(const std::string& opt);

    const std::string& commsGetProtocolVersion() const;
    void commsSetProtocolVersion(const std::string& value);

    bool commsGetMainNamespaceInOptionsForced() const;
    void commsSetMainNamespaceInOptionsForced(bool value);
    bool commsHasMainNamespaceInOptions() const;

    const std::vector<std::string>& commsGetExtraInputBundles() const;
    void commsSetExtraInputBundles(const std::vector<std::string>& inputBundles);
    const ExtraMessageBundlesList& commsExtraMessageBundles() const;

    static const std::string& commsMinCommsVersion();

protected:
    virtual bool genPrepareImpl() override;

    virtual GenSchemaPtr genCreateSchemaImpl(commsdsl::parse::ParseSchema dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenNamespacePtr genCreateNamespaceImpl(commsdsl::parse::ParseNamespace dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenInterfacePtr genCreateInterfaceImpl(commsdsl::parse::ParseInterface dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenMessagePtr genCreateMessageImpl(commsdsl::parse::ParseMessage dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFramePtr genCreateFrameImpl(commsdsl::parse::ParseFrame dslObj, commsdsl::gen::GenElem* parent) override;

    virtual GenFieldPtr genCreateIntFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateEnumFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateSetFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateFloatFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateBitfieldFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateBundleFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateStringFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateDataFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateListFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateRefFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateOptionalFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateVariantFieldImpl(commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) override;

    virtual GenLayerPtr genCreateCustomLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSyncLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSizeLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateIdLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateValueLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreatePayloadLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateChecksumLayerImpl(commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) override;

    virtual bool genWriteImpl() override;    

private:
    bool commsPrepareExtraMessageBundlesInternal();
    bool commsWriteExtraFilesInternal() const;
    
    static const CustomizationLevel DefaultCustomizationLevel = CustomizationLevel::Limited;
    CustomizationLevel m_customizationLevel = DefaultCustomizationLevel;    
    std::string m_protocolVersion;
    std::vector<std::string> m_extraInputBundles;
    ExtraMessageBundlesList m_commsExtraMessageBundles;
    bool m_mainNamespaceInOptionsForced = false;
};

} // namespace commsdsl2comms
