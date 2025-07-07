//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseIntField.h"

#include <string>

namespace commsdsl2swig 
{

class SwigInterface;
class SwigNamespace;

class SwigGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;
    
public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;

    SwigGenerator();
    
    static const std::string& fileGeneratedComment();

    static SwigGenerator& cast(commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<SwigGenerator&>(generator);
    }

    static const SwigGenerator& cast(const commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<const SwigGenerator&>(generator);
    }    
    
    std::string swigInputCodePathFor(const commsdsl::gen::GenElem& elem) const;
    std::string swigInputCodePathForFile(const std::string& name) const;
    std::string swigClassName(const commsdsl::gen::GenElem& elem) const;
    std::string swigScopeNameForRoot(const std::string& name) const;
    std::string swigScopeNameForMsgId(const std::string& name, const SwigNamespace& parent) const;
    std::string swigProtocolClassNameForRoot(const std::string& name) const;
    const std::string& swigConvertCppType(const std::string& str) const;
    const std::string& swigConvertIntType(commsdsl::parse::ParseIntField::ParseType value, std::size_t len) const;

    static std::string swigScopeToName(const std::string& scope);
    static std::string swigDefInclude(const std::string& path);

    void swigSetMainNamespaceInNamesForced(bool value);
    void swigSetForcedInterface(const std::string& value);
    void swigSetHasProtocolVersion(bool value);
    void swigSetMessagesListFile(const std::string& value);
    void swigSetForcedPlatform(const std::string& value);

    bool swigHasProtocolVersion() const;

    const SwigInterface* swigMainInterface() const;
    SwigInterface* swigMainInterface();

protected:
    virtual bool genCreateCompleteImpl() override;
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() override;    

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

private:
    bool swigWriteExtraFilesInternal() const;
    bool swigReferenceRequestedInterfaceInternal();
    bool swigReferenceRequestedMessagesInternal();
    bool swigProcessMessagesListFileInternal();
    bool swigProcessForcedPlatformInternal();

    std::string m_forcedInterface;
    std::string m_messagesListFile;
    std::string m_forcedPlatform;
    bool m_mainNamespaceInNamesForced = false;
    bool m_hasProtocolVersion = false;
};

} // namespace commsdsl2swig
