//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2emscripten 
{

class EmscriptenInterface;
class EmscriptenNamespace;

class EmscriptenGenerator final : public commsdsl::gen::GenGenerator
{
    using GenBase = commsdsl::gen::GenGenerator;
    
public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    EmscriptenGenerator();
    
    static const std::string& emscriptenFileGeneratedComment();

    static EmscriptenGenerator& emscriptenCast(commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<EmscriptenGenerator&>(generator);
    }

    static const EmscriptenGenerator& emscriptenCast(const commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<const EmscriptenGenerator&>(generator);
    }    
    
    std::string emscriptenClassName(const commsdsl::gen::GenElem& elem) const;
    std::string emscriptenScopeNameForRoot(const std::string& name) const;
    std::string emscriptenScopeNameForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenProtocolClassNameForRoot(const std::string& name) const;

    std::string emscriptenRelHeaderForRoot(const std::string& name) const;
    std::string emscriptenRelHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenRelHeaderForInput(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenAbsHeaderForRoot(const std::string& name) const;
    std::string emscriptenAbsHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenRelSourceForRoot(const std::string& name) const;
    std::string emscriptenRelSourceForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenAbsSourceForRoot(const std::string& name) const;
    std::string emscriptenAbsSourceForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    
    std::string emscriptenProtocolRelHeaderForRoot(const std::string& name) const;
    std::string emscriptenProtocolRelHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    std::string emscriptenSchemaRelSourceForRoot(unsigned schemaIdx, const std::string& name) const;

    std::string emscriptenRelHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string emscriptenAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string emscriptenRelSourceFor(const commsdsl::gen::GenElem& elem) const;
    std::string emscriptenAbsSourceFor(const commsdsl::gen::GenElem& elem) const;

    std::string emscriptenInputAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    std::string emscriptenInputAbsSourceFor(const commsdsl::gen::GenElem& elem) const;

    static std::string emscriptenScopeToName(const std::string& scope);

    void emscriptenSetMainNamespaceInNamesForced(bool value);
    void emscriptenSetForcedInterface(const std::string& value);
    void emscriptenSetHasProtocolVersion(bool value);
    void emscriptenSetMessagesListFile(const std::string& value);
    void emscriptenSetForcedPlatform(const std::string& value);

    bool emscriptenHasProtocolVersion() const;

    const EmscriptenInterface* emscriptenMainInterface() const;
    EmscriptenInterface* emscriptenMainInterface();

protected:
    virtual bool genCreateCompleteImpl() override;
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() override;    

    virtual GenSchemaPtr genCreateSchemaImpl(commsdsl::parse::ParseSchema parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenNamespacePtr genCreateNamespaceImpl(commsdsl::parse::ParseNamespace parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenInterfacePtr genCreateInterfaceImpl(commsdsl::parse::ParseInterface parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenMessagePtr genCreateMessageImpl(commsdsl::parse::ParseMessage parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFramePtr genCreateFrameImpl(commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent) override;

    virtual GenFieldPtr genCreateIntFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateEnumFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateSetFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateFloatFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateBitfieldFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateBundleFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateStringFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateDataFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateListFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateRefFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateOptionalFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFieldPtr genCreateVariantFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;    

    virtual GenLayerPtr genCreateCustomLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSyncLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSizeLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateIdLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateValueLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreatePayloadLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateChecksumLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    bool emscriptenWriteExtraFilesInternal() const;
    bool emscriptenReferenceRequestedInterfaceInternal();
    bool emscriptenReferenceRequestedMessagesInternal();
    bool emscriptenProcessMessagesListFileInternal();
    bool emscriptenProcessForcedPlatformInternal();

    std::string m_forcedInterface;
    std::string m_messagesListFile;
    std::string m_forcedPlatform;
    bool m_mainNamespaceInNamesForced = false;
    bool m_hasProtocolVersion = false;
};

} // namespace commsdsl2emscripten
