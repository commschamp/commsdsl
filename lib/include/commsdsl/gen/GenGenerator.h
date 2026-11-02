//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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
#include <string>
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

    enum GenVersionIdx
    {
        GenVersionIdx_Major,
        GenVersionIdx_Minor,
        GenVersionIdx_Patch,
        GenVersionIdx_NumOfValues
    };

    GenGenerator();
    virtual ~GenGenerator();

    [[nodiscard]] int genExec(const GenProgramOptions& options);

    void genForceSchemaVersion(unsigned value);
    void genSetMinRemoteVersion(unsigned value);
    [[nodiscard]] unsigned genGetMinRemoteVersion() const;
    [[nodiscard]] const std::string& genGetCodeVersion() const;
    [[nodiscard]] std::vector<std::string> genGetCodeVersionTokens() const;
    void genSetCodeVersion(const std::string& value);
    void genSetMessagesListFile(const std::string& value);
    [[nodiscard]] const std::string& genGetMessagesListFile() const;
    void genSetForcedPlatform(const std::string& value);
    [[nodiscard]] const std::string& genGetForcedPlatform() const;
    void genSetForcedInterface(const std::string& value);
    [[nodiscard]] const std::string& genGetForcedInterface() const;
    [[nodiscard]] const GenInterface* genForcedInterface() const;
    void genSetNamespaceOverride(const std::string& value);

    void genSetTopNamespace(const std::string& value);
    [[nodiscard]] const std::string& genGetTopNamespace() const;

    void genSetOutputDir(const std::string& outDir);
    [[nodiscard]] const std::string& genGetOutputDir() const;

    void genSetCodeDir(const std::string& dir);
    [[nodiscard]] const std::string& genGetCodeDir() const;

    void genSetMultipleSchemasEnabled(bool enabled);
    [[nodiscard]] bool genGetMultipleSchemasEnabled() const;

    void genSetVersionIndependentCodeForced(bool value = true);
    [[nodiscard]] bool genGetVersionIndependentCodeForced() const;

    void genSetCodeInjectCommentsRequested(bool value = true);
    [[nodiscard]] bool genGetCodeInjectCommentsRequested() const;

    [[nodiscard]] const GenField* genFindField(const std::string& externalRef) const;
    [[nodiscard]] GenField* genFindField(const std::string& externalRef);
    [[nodiscard]] const GenMessage* genFindMessage(const std::string& externalRef) const;
    [[nodiscard]] GenMessage* genFindMessage(const std::string& externalRef);
    [[nodiscard]] const GenFrame* genFindFrame(const std::string& externalRef) const;
    [[nodiscard]] const GenInterface* genFindInterface(const std::string& externalRef) const;
    [[nodiscard]] static const GenSchema& genSchemaOf(const GenElem& elem);

    [[nodiscard]] GenNamespacesAccessList genGetAllNamespaces() const;
    [[nodiscard]] GenInterfacesAccessList genGetAllInterfaces() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessages() const;
    static void genSortMessages(GenMessagesAccessList& list);
    [[nodiscard]] GenMessagesAccessList genGetAllMessagesIdSorted() const;
    [[nodiscard]] GenFramesAccessList genGetAllFrames() const;
    [[nodiscard]] GenFieldsAccessList genGetAllFields() const;
    [[nodiscard]] bool genHasAnyReferencedMessage() const;
    [[nodiscard]] bool genHasAnyReferencedMessageAllSchemas() const;

    [[nodiscard]] GenNamespacesAccessList genGetAllNamespacesFromAllSchemas() const;
    [[nodiscard]] GenInterfacesAccessList genGetAllInterfacesFromAllSchemas() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessagesFromAllSchemas() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessagesIdSortedFromAllSchemas() const;
    [[nodiscard]] GenFramesAccessList genGetAllFramesFromAllSchemas() const;
    [[nodiscard]] GenFieldsAccessList genGetAllFieldsFromAllSchemas() const;

    [[nodiscard]] bool genPrepare(const GenFilesList& files);
    [[nodiscard]] bool genWrite();

    [[nodiscard]] bool genDoesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    [[nodiscard]] bool genIsElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    [[nodiscard]] bool genIsElementDeprecated(unsigned deprecatedSince) const;

    [[nodiscard]] GenLogger& genLogger();
    [[nodiscard]] const GenLogger& genLogger() const;

    [[nodiscard]] const GenSchemasList& genSchemas() const;

    [[nodiscard]] GenSchema& genCurrentSchema();
    [[nodiscard]] const GenSchema& genCurrentSchema() const;
    [[nodiscard]] GenSchema& genProtocolSchema();
    [[nodiscard]] const GenSchema& genProtocolSchema() const;
    [[nodiscard]] bool genIsCurrentProtocolSchema() const;

    [[nodiscard]] GenSchemaPtr genCreateSchema(ParseSchema parseObj, GenElem* parent = nullptr);
    [[nodiscard]] GenNamespacePtr genCreateNamespace(ParseNamespace parseObj, GenElem* parent = nullptr);
    [[nodiscard]] GenInterfacePtr genCreateInterface(ParseInterface parseObj, GenElem* parent);
    [[nodiscard]] GenMessagePtr genCreateMessage(ParseMessage parseObj, GenElem* parent);
    [[nodiscard]] GenFramePtr genCreateFrame(ParseFrame parseObj, GenElem* parent);

    [[nodiscard]] GenFieldPtr genCreateIntField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateEnumField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateSetField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateFloatField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateBitfieldField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateBundleField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateStringField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateDataField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateListField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateRefField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateOptionalField(ParseField parseObj, GenElem* parent);
    [[nodiscard]] GenFieldPtr genCreateVariantField(ParseField parseObj, GenElem* parent);

    [[nodiscard]] GenLayerPtr genCreateCustomLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreateSyncLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreateSizeLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreateIdLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreateValueLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreatePayloadLayer(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] GenLayerPtr genCreateChecksumLayer(ParseLayer parseObj, GenElem* parent);

    [[nodiscard]] unsigned genCurrentSchemaIdx() const;
    void genChooseCurrentSchema(unsigned idx);
    void genChooseCurrentSchema(unsigned idx) const;
    void genChooseProtocolSchema();
    void genChooseProtocolSchema() const;

    [[nodiscard]] bool genCreateDirectory(const std::string& path) const;

    void genReferenceAllMessages();

    void genReferenceAllInterfaces();

    [[nodiscard]] OptsProcessResult genProcessOptions(const GenProgramOptions& options);

    [[nodiscard]] std::string genReadCodeInjectCode(
        const std::string& relPath,
        const std::string& comment,
        bool* realCodeInjected = nullptr,
        const std::string& forcedCommentPrefix = std::string()) const;

    [[nodiscard]] std::string genReadScriptCodeInjectCode(
        const std::string& relPath,
        const std::string& comment,
        bool* realCodeInjected = nullptr) const;

protected:
    [[nodiscard]] virtual bool genCreateCompleteImpl();
    [[nodiscard]] virtual bool genPrepareImpl();

    [[nodiscard]] virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent);
    [[nodiscard]] virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent);
    [[nodiscard]] virtual GenInterfacePtr genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent);
    [[nodiscard]] virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent);

    [[nodiscard]] virtual GenFieldPtr genCreateIntFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateSetFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateStringFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateDataFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateListFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateRefFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent);
    [[nodiscard]] virtual GenFieldPtr genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent);

    [[nodiscard]] virtual GenLayerPtr genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent);
    [[nodiscard]] virtual GenLayerPtr genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent);

    [[nodiscard]] virtual bool genWriteImpl();
    [[nodiscard]] virtual GenLoggerPtr genCreateLoggerImpl();

    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options);

    [[nodiscard]] virtual const std::string& genCommentPrefixImpl() const;

    [[nodiscard]] GenNamespace* genAddDefaultNamespace();

    [[nodiscard]] bool genCopyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const;

    [[nodiscard]] static std::string genVersionStr();

private:
    std::unique_ptr<GenGeneratorImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
