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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/GenLogger.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/GenSchema.h"
#include "commsdsl/parse/ParseEndian.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenGeneratorImpl;
class GenGenerator
{
public:
    using FilesList = std::vector<std::string>;
    using LoggerPtr = std::unique_ptr<GenLogger>;
    using NamespacesList = GenNamespace::NamespacesList;
    using PlatformNamesList = std::vector<std::string>;
    using SchemasList = std::vector<SchemaPtr>;

    using NamespacesAccessList = GenNamespace::NamespacesAccessList;
    using InterfacesAccessList = GenNamespace::InterfacesAccessList;
    using MessagesAccessList = GenNamespace::MessagesAccessList;
    using FramesAccessList = GenNamespace::FramesAccessList;
    using FieldsAccessList = GenNamespace::FieldsAccessList;

    GenGenerator();
    virtual ~GenGenerator();

    void forceSchemaVersion(unsigned value);
    void setMinRemoteVersion(unsigned value);
    unsigned getMinRemoteVersion() const;
    void setNamespaceOverride(const std::string& value);    

    void setTopNamespace(const std::string& value);
    const std::string& getTopNamespace() const;

    void setOutputDir(const std::string& outDir);
    const std::string& getOutputDir() const;

    void setCodeDir(const std::string& dir);
    const std::string& getCodeDir() const;   

    void parseSetMultipleSchemasEnabled(bool enabled);
    bool parseGetMultipleSchemasEnabled() const;

    void setVersionIndependentCodeForced(bool value = true); 
    bool getVersionIndependentCodeForced() const;

    const GenField* findField(const std::string& externalRef) const;
    GenField* findField(const std::string& externalRef);
    const GenMessage* findMessage(const std::string& externalRef) const;
    GenMessage* findMessage(const std::string& externalRef);
    const GenFrame* findFrame(const std::string& externalRef) const;
    const GenInterface* findInterface(const std::string& externalRef) const;
    static const GenSchema& schemaOf(const GenElem& elem);

    NamespacesAccessList getAllNamespaces() const;
    InterfacesAccessList getAllInterfaces() const;
    MessagesAccessList getAllMessages() const;
    static void sortMessages(MessagesAccessList& list);
    MessagesAccessList getAllMessagesIdSorted() const;
    FramesAccessList getAllFrames() const;
    FieldsAccessList getAllFields() const;

    NamespacesAccessList getAllNamespacesFromAllSchemas() const;
    InterfacesAccessList getAllInterfacesFromAllSchemas() const;
    MessagesAccessList getAllMessagesFromAllSchemas() const;
    MessagesAccessList getAllMessagesIdSortedFromAllSchemas() const;
    FramesAccessList getAllFramesFromAllSchemas() const;
    FieldsAccessList getAllFieldsFromAllSchemas() const;    

    bool prepare(const FilesList& files);
    bool write();

    bool doesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool isElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool isElementDeprecated(unsigned deprecatedSince) const;

    GenLogger& logger();
    const GenLogger& logger() const;

    const SchemasList& schemas() const;

    GenSchema& currentSchema();
    const GenSchema& currentSchema() const;
    GenSchema& protocolSchema();
    const GenSchema& protocolSchema() const;    
    bool isCurrentProtocolSchema() const;

    SchemaPtr createSchema(commsdsl::parse::ParseSchema dslObj, GenElem* parent = nullptr);
    NamespacePtr createNamespace(commsdsl::parse::ParseNamespace dslObj, GenElem* parent = nullptr);
    InterfacePtr createInterface(commsdsl::parse::ParseInterface dslObj, GenElem* parent);
    MessagePtr createMessage(commsdsl::parse::ParseMessage dslObj, GenElem* parent);
    FramePtr createFrame(commsdsl::parse::ParseFrame dslObj, GenElem* parent);

    FieldPtr createIntField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createEnumField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createSetField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createFloatField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createBitfieldField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createBundleField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createStringField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createDataField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createListField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createRefField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createOptionalField(commsdsl::parse::ParseField dslObj, GenElem* parent);
    FieldPtr createVariantField(commsdsl::parse::ParseField dslObj, GenElem* parent);

    LayerPtr createCustomLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createSyncLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createSizeLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createIdLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createValueLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createPayloadLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    LayerPtr createChecksumLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent);

    unsigned currentSchemaIdx() const;
    void chooseCurrentSchema(unsigned idx);
    void chooseCurrentSchema(unsigned idx) const;
    void chooseProtocolSchema();
    void chooseProtocolSchema() const;

    bool createDirectory(const std::string& path) const;

    void referenceAllMessages();
    bool getAllMessagesReferencedByDefault() const;
    void setAllMessagesReferencedByDefault(bool value = true);

    void referenceAllInterfaces();
    bool getAllInterfacesReferencedByDefault() const;
    void setAllInterfacesReferencedByDefault(bool value = true);    

protected:
    virtual bool createCompleteImpl();
    virtual bool prepareImpl();

    virtual SchemaPtr createSchemaImpl(commsdsl::parse::ParseSchema dslObj, GenElem* parent);
    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::ParseNamespace dslObj, GenElem* parent);
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::ParseInterface dslObj, GenElem* parent);
    virtual MessagePtr createMessageImpl(commsdsl::parse::ParseMessage dslObj, GenElem* parent);
    virtual FramePtr createFrameImpl(commsdsl::parse::ParseFrame dslObj, GenElem* parent);

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createListFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent);

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent);

    virtual bool writeImpl();
    virtual LoggerPtr createLoggerImpl();

    GenNamespace* addDefaultNamespace();

    bool copyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const;

private:
    std::unique_ptr<GenGeneratorImpl> m_impl;    
};

} // namespace gen

} // namespace commsdsl
