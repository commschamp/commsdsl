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
#include "commsdsl/gen/GenProgramOptions.h"
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
    using ParseSchema = commsdsl::parse::ParseSchema;
    using ParseNamespace = commsdsl::parse::ParseNamespace;
    using ParseInterface = commsdsl::parse::ParseInterface;
    using ParseMessage = commsdsl::parse::ParseMessage;
    using ParseFrame = commsdsl::parse::ParseFrame;
    using ParseField = commsdsl::parse::ParseField;
    using ParseLayer = commsdsl::parse::ParseLayer;

    using GenFilesList = std::vector<std::string>;
    using GenLoggerPtr = std::unique_ptr<GenLogger>;
    using GenNamespacesList = GenNamespace::GenNamespacesList;
    using GenPlatformNamesList = std::vector<std::string>;
    using GenSchemasList = std::vector<GenSchemaPtr>;

    using GenNamespacesAccessList = GenNamespace::GenNamespacesAccessList;
    using GenInterfacesAccessList = GenNamespace::GenInterfacesAccessList;
    using GenMessagesAccessList = GenNamespace::GenMessagesAccessList;
    using GenFramesAccessList = GenNamespace::GenFramesAccessList;
    using GenFieldsAccessList = GenNamespace::GenFieldsAccessList;

    enum OptsProcessResult
    {
        OptsProcessResult_Failure,
        OptsProcessResult_Continue,
        OptsProcessResult_EarlyExit,
        OptsProcessResult_NumOfValues
    };

    GenGenerator();
    virtual ~GenGenerator();

    int genExec(const GenProgramOptions& options);

    void genForceSchemaVersion(unsigned value);
    void genSetMinRemoteVersion(unsigned value);
    unsigned genGetMinRemoteVersion() const;
    const std::string& genGetCodeVersion() const;
    void genSetCodeVersion(const std::string& value);
    void genSetMessagesListFile(const std::string& value);
    const std::string& genGetMessagesListFile() const;
    void genSetForcedPlatform(const std::string& value);
    const std::string& genGetForcedPlatform() const;
    void genSetForcedInterface(const std::string& value);
    const std::string& genGetForcedInterface() const;
    const GenInterface* genForcedInterface() const;
    void genSetNamespaceOverride(const std::string& value);

    void genSetTopNamespace(const std::string& value);
    const std::string& genGetTopNamespace() const;

    void genSetOutputDir(const std::string& outDir);
    const std::string& genGetOutputDir() const;

    void genSetCodeDir(const std::string& dir);
    const std::string& genGetCodeDir() const;

    void genSetMultipleSchemasEnabled(bool enabled);
    bool genGetMultipleSchemasEnabled() const;

    void genSetVersionIndependentCodeForced(bool value = true);
    bool genGetVersionIndependentCodeForced() const;

    const GenField* genFindField(const std::string& externalRef) const;
    GenField* genFindField(const std::string& externalRef);
    const GenMessage* genGindMessage(const std::string& externalRef) const;
    GenMessage* genGindMessage(const std::string& externalRef);
    const GenFrame* genFindFrame(const std::string& externalRef) const;
    const GenInterface* genFindInterface(const std::string& externalRef) const;
    static const GenSchema& genSchemaOf(const GenElem& elem);

    GenNamespacesAccessList genGetAllNamespaces() const;
    GenInterfacesAccessList genGetAllInterfaces() const;
    GenMessagesAccessList genGetAllMessages() const;
    static void genSortMessages(GenMessagesAccessList& list);
    GenMessagesAccessList genGetAllMessagesIdSorted() const;
    GenFramesAccessList genGetAllFrames() const;
    GenFieldsAccessList genGetAllFields() const;

    GenNamespacesAccessList genGetAllNamespacesFromAllSchemas() const;
    GenInterfacesAccessList genGetAllInterfacesFromAllSchemas() const;
    GenMessagesAccessList genGetAllMessagesFromAllSchemas() const;
    GenMessagesAccessList genGetAllMessagesIdSortedFromAllSchemas() const;
    GenFramesAccessList genGetAllFramesFromAllSchemas() const;
    GenFieldsAccessList genGetAllFieldsFromAllSchemas() const;

    bool genPrepare(const GenFilesList& files);
    bool genWrite();

    bool genDoesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool genIsElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool genIsElementDeprecated(unsigned deprecatedSince) const;

    GenLogger& genLogger();
    const GenLogger& genLogger() const;

    const GenSchemasList& genSchemas() const;

    GenSchema& genCurrentSchema();
    const GenSchema& genCurrentSchema() const;
    GenSchema& genProtocolSchema();
    const GenSchema& genProtocolSchema() const;
    bool genIsCurrentProtocolSchema() const;

    GenSchemaPtr genCreateSchema(ParseSchema parseObj, GenElem* parent = nullptr);
    GenNamespacePtr genCreateNamespace(ParseNamespace parseObj, GenElem* parent = nullptr);
    GenInterfacePtr genCreateInterface(ParseInterface parseObj, GenElem* parent);
    GenMessagePtr genCreateMessage(ParseMessage parseObj, GenElem* parent);
    GenFramePtr genCreateFrame(ParseFrame parseObj, GenElem* parent);

    GenFieldPtr genCreateIntField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateEnumField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateSetField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateFloatField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateBitfieldField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateBundleField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateStringField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateDataField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateListField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateRefField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateOptionalField(ParseField parseObj, GenElem* parent);
    GenFieldPtr genCreateVariantField(ParseField parseObj, GenElem* parent);

    GenLayerPtr genCreateCustomLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreateSyncLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreateSizeLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreateIdLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreateValueLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreatePayloadLayer(ParseLayer parseObj, GenElem* parent);
    GenLayerPtr genCreateChecksumLayer(ParseLayer parseObj, GenElem* parent);

    unsigned genCurrentSchemaIdx() const;
    void genChooseCurrentSchema(unsigned idx);
    void genChooseCurrentSchema(unsigned idx) const;
    void genChooseProtocolSchema();
    void genChooseProtocolSchema() const;

    bool genCreateDirectory(const std::string& path) const;

    void genReferenceAllMessages();

    void genReferenceAllInterfaces();

    OptsProcessResult genProcessOptions(const GenProgramOptions& options);

protected:
    virtual bool genCreateCompleteImpl();
    virtual bool genPrepareImpl();

    virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent);
    virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent);
    virtual GenInterfacePtr genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent);
    virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent);
    virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent);

    virtual GenFieldPtr genCreateIntFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateSetFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateStringFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateDataFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateListFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateRefFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent);
    virtual GenFieldPtr genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent);

    virtual GenLayerPtr genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent);
    virtual GenLayerPtr genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent);

    virtual bool genWriteImpl();
    virtual GenLoggerPtr genCreateLoggerImpl();

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options);

    GenNamespace* genAddDefaultNamespace();

    bool genCopyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const;

private:
    std::unique_ptr<GenGeneratorImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
