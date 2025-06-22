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

#include "commsdsl/gen/Generator.h"

#include "commsdsl/parse/ParseIntField.h"

#include <string>

namespace commsdsl2emscripten 
{

class EmscriptenInterface;
class EmscriptenNamespace;

class EmscriptenGenerator final : public commsdsl::gen::Generator
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

    EmscriptenGenerator();
    
    static const std::string& fileGeneratedComment();

    static EmscriptenGenerator& cast(commsdsl::gen::Generator& generator)
    {
        return static_cast<EmscriptenGenerator&>(generator);
    }

    static const EmscriptenGenerator& cast(const commsdsl::gen::Generator& generator)
    {
        return static_cast<const EmscriptenGenerator&>(generator);
    }    
    
    std::string emscriptenClassName(const Elem& elem) const;
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

    std::string emscriptenRelHeaderFor(const Elem& elem) const;
    std::string emscriptenAbsHeaderFor(const Elem& elem) const;
    std::string emscriptenRelSourceFor(const Elem& elem) const;
    std::string emscriptenAbsSourceFor(const Elem& elem) const;

    std::string emspriptenInputAbsHeaderFor(const Elem& elem) const;
    std::string emspriptenInputAbsSourceFor(const Elem& elem) const;


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
    virtual bool createCompleteImpl() override;
    virtual bool prepareImpl() override;
    virtual bool writeImpl() override;    

    virtual SchemaPtr createSchemaImpl(commsdsl::parse::ParseSchema dslObj, Elem* parent) override;
    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::ParseNamespace dslObj, Elem* parent) override;
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::ParseInterface dslObj, Elem* parent) override;
    virtual MessagePtr createMessageImpl(commsdsl::parse::ParseMessage dslObj, Elem* parent) override;
    virtual FramePtr createFrameImpl(commsdsl::parse::ParseFrame dslObj, Elem* parent) override;

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createListFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::ParseField dslObj, Elem* parent) override;    

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;

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
