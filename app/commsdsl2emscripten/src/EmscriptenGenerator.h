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

namespace commsdsl2emscripten 
{

class EmscriptenInterface;
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
    std::string emscriptenProtocolClassNameForRoot(const std::string& name) const;

    std::string emscriptenRelHeaderForRoot(const std::string& name) const;
    std::string emscriptenAbsHeaderForRoot(const std::string& name) const;
    std::string emscriptenRelSourceForRoot(const std::string& name) const;
    std::string emscriptenAbsSourceForRoot(const std::string& name) const;

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

    // virtual SchemaPtr createSchemaImpl(commsdsl::parse::Schema dslObj, Elem* parent) override;
    // virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent) override;
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent) override;
    virtual MessagePtr createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent) override;
    // virtual FramePtr createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent) override;

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

    // virtual LayerPtr createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    // virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;

private:
    bool emscriptenWriteExtraFilesInternal() const;
    bool emscriptenPrepareDefaultInterfaceInternal();
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
