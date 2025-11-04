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

#include "commsdsl/gen/GenGenerator.h"

#include "commsdsl/gen/GenBitfieldField.h"
#include "commsdsl/gen/GenBundleField.h"
#include "commsdsl/gen/GenChecksumLayer.h"
#include "commsdsl/gen/GenCustomLayer.h"
#include "commsdsl/gen/GenDataField.h"
#include "commsdsl/gen/GenEnumField.h"
#include "commsdsl/gen/GenFloatField.h"
#include "commsdsl/gen/GenIdLayer.h"
#include "commsdsl/gen/GenIntField.h"
#include "commsdsl/gen/GenListField.h"
#include "commsdsl/gen/GenOptionalField.h"
#include "commsdsl/gen/GenPayloadLayer.h"
#include "commsdsl/gen/GenRefField.h"
#include "commsdsl/gen/GenSchema.h"
#include "commsdsl/gen/GenSetField.h"
#include "commsdsl/gen/GenSizeLayer.h"
#include "commsdsl/gen/GenStringField.h"
#include "commsdsl/gen/GenSyncLayer.h"
#include "commsdsl/gen/GenValueLayer.h"
#include "commsdsl/gen/GenVariantField.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "commsdsl/version.h"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <system_error>

namespace fs = std::filesystem;

namespace commsdsl
{

namespace gen
{

class GenGeneratorImpl
{
public:
    using GenLoggerPtr = GenGenerator::GenLoggerPtr;
    using GenFilesList = GenGenerator::GenFilesList;
    using GenNamespacesList = GenGenerator::GenNamespacesList;
    using GenPlatformNamesList = GenGenerator::GenPlatformNamesList;
    using GenSchemasList = GenGenerator::GenSchemasList;
    using GenInterfacesAccessList = GenGenerator::GenInterfacesAccessList;

    explicit GenGeneratorImpl(GenGenerator& generator) :
        m_generator(generator),
        m_outputDir(fs::current_path().string())
    {
    }

    GenLoggerPtr& genGetLogger()
    {
        return m_logger;
    }

    const GenLoggerPtr& genGetLogger() const
    {
        return m_logger;
    }

    void genSetLogger(GenLoggerPtr logger)
    {
        m_logger = std::move(logger);
    }

    const GenSchemasList& genSchemas() const
    {
        return m_schemas;
    }

    GenSchema& genCurrentSchema()
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    const GenSchema& genCurrentSchema() const
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    GenSchema& genProtocolSchema()
    {
        assert(!m_schemas.empty());
        return *m_schemas.back();
    }

    const GenSchema& genProtocolSchema() const
    {
        assert(!m_schemas.empty());
        return *m_schemas.back();
    }

    void genChooseCurrentSchema(unsigned idx)
    {
        assert(idx < m_schemas.size());
        m_currentSchema = m_schemas[idx].get();
    }

    unsigned genCurrentSchemaIdx() const
    {
        assert(m_currentSchema != nullptr);
        auto iter =
            std::find_if(
                m_schemas.begin(), m_schemas.end(),
                [this](auto& sPtr)
                {
                    return m_currentSchema == sPtr.get();
                });
        assert(iter != m_schemas.end());
        return static_cast<unsigned>(std::distance(m_schemas.begin(), iter));
    }

    void genForceSchemaVersion(unsigned value)
    {
        m_forcedSchemaVersion = static_cast<decltype(m_forcedSchemaVersion)>(value);
    }

    void genSetMinRemoteVersion(unsigned value)
    {
        m_minRemoteVersion = value;
    }

    unsigned genGetMinRemoteVersion() const
    {
        return m_minRemoteVersion;
    }

    const std::string& genGetCodeVersion() const
    {
        return m_codeVersion;
    }

    void genSetCodeVersion(const std::string& value)
    {
        m_codeVersion = value;
    }

    GenInterfacesAccessList genGetAllInterfaces() const
    {
        return genCurrentSchema().genGetAllInterfaces();
    }

    GenNamespace* genAddDefaultNamespace()
    {
        return genCurrentSchema().genAddDefaultNamespace();
    }

    void genSetNamespaceOverride(const std::string& value)
    {
        m_namespaceOverrides.clear();
        if (value.empty()) {
            return;
        }

        auto elems = util::genStrSplitByAnyChar(value, ",");
        for (auto& e : elems) {
            if (e.empty()) {
                continue;
            }

            auto pos = e.find_first_of(":");
            std::string first;
            std::string second = e;
            do {
                if ((e.size() - 1U) <= pos) {
                    break;
                }

                first = e.substr(0, pos);
                second = e.substr(pos + 1);
            } while (false);

            m_namespaceOverrides[first] = second;
        }
    }

    void genSetTopNamespace(const std::string& value)
    {
        m_topNamespace = value;
    }

    const std::string& genGetTopNamespace() const
    {
        return m_topNamespace;
    }

    void genSetOutputDir(const std::string& outDir)
    {
        if (!outDir.empty()) {
            m_outputDir = outDir;
        }
    }

    const std::string& genGetOutputDir() const
    {
        return m_outputDir;
    }

    void genSetCodeDir(const std::string& dir)
    {
        m_codeDir = dir;
    }

    const std::string& genGetCodeDir() const
    {
        return m_codeDir;
    }

    void genSetMultipleSchemasEnabled(bool enabled)
    {
        m_protocol.parseSetMultipleSchemasEnabled(enabled);
    }

    bool genGetMultipleSchemasEnabled() const
    {
        return m_protocol.parseGetMultipleSchemasEnabled();
    }

    void genSetVersionIndependentCodeForced(bool value)
    {
        m_versionIndependentCodeForced = value;
    }

    bool genGetVersionIndependentCodeForced() const
    {
        return m_versionIndependentCodeForced;
    }

    const GenField* genFindField(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->genFindField(parsedRef.second);
    }

    GenField* genFindField(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<GenSchema*>(parsedRef.first)->genFindField(parsedRef.second);
    }

    const GenMessage* genGindMessage(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->genGindMessage(parsedRef.second);
    }

    GenMessage* genGindMessage(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<GenSchema*>(parsedRef.first)->genGindMessage(parsedRef.second);
    }

    const GenFrame* genFindFrame(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->genFindFrame(parsedRef.second);
    }

    const GenInterface* genFindInterface(const std::string& externalRef) const
    {
        if (externalRef.empty()) {
            return genCurrentSchema().genFindInterface(externalRef);
        }

        assert(!externalRef.empty());
        auto parsedRef = genParseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->genFindInterface(parsedRef.second);
    }

    using CreateCompleteFunc = std::function<bool ()>;
    bool genPrepare(const GenFilesList& files, CreateCompleteFunc createCompleteCb = CreateCompleteFunc())
    {
        m_protocol.parseSetErrorReportCallback(
            [this](commsdsl::parse::ParseErrorLevel level, const std::string& msg)
            {
                assert(m_logger);
                m_logger->genLog(level, msg);
            });

        assert(m_logger);
        for (auto& f : files) {
            m_logger->genInfo("Parsing " + f);
            if (!m_protocol.parse(f)) {
                return false;
            }

            if (m_logger->genHadWarning()) {
                m_logger->genError("Warning treated as error");
                return false;
            }
        }

        if (!m_protocol.parseValidate()) {
            return false;
        }

        if (m_logger->genHadWarning()) {
            m_logger->genError("Warning treated as error");
            return false;
        }

        auto allSchemas = m_protocol.parseSchemas();
        if (allSchemas.empty()) {
            m_logger->genError("No schemas available");
            return false;
        }

        for (auto& s : allSchemas) {
            auto schema = m_generator.genCreateSchema(s);
            schema->genSetVersionIndependentCodeForced(m_versionIndependentCodeForced);

            auto renameIter = m_namespaceOverrides.find(util::genStrToName(s.parseName()));
            if (renameIter != m_namespaceOverrides.end()) {
                schema->genSetMainNamespaceOverride(renameIter->second);
            }
            m_schemas.push_back(std::move(schema));
        }

        assert(!m_schemas.empty());
        auto& protocolSchemaPtr = m_schemas.back();

        protocolSchemaPtr->genSetMinRemoteVersion(m_minRemoteVersion);
        if (0 <= m_forcedSchemaVersion) {
            protocolSchemaPtr->genForceSchemaVersion(static_cast<unsigned>(m_forcedSchemaVersion));
        }

        auto renameIter = m_namespaceOverrides.find(strings::genEmptyString());
        if (renameIter != m_namespaceOverrides.end()) {
            protocolSchemaPtr->genSetMainNamespaceOverride(renameIter->second);
        }

        for (auto& s : m_schemas) {
            m_currentSchema = s.get();
            if (!s->genCreateAll()) {
                m_logger->genError("Failed to genCreate elements inside schema \"" + s->genParseObj().parseName() + "\"");
                return false;
            }

            if (m_allInterfacesReferencedByDefault) {
                s->genSetAllInterfacesReferenced();
            }

            if (m_allMessagesReferencedByDefault) {
                s->genSetAllMessagesReferenced();
            }
        }

        if (createCompleteCb && (!createCompleteCb())) {
            return false;
        }

        if (!genPrepareDefaultInterfaceInternal()) {
            return false;
        }

        for (auto& s : m_schemas) {
            m_currentSchema = s.get();
            if (!s->genPrepare()) {
                m_logger->genError("Failed to prepare elements inside schema \"" + s->genParseObj().parseName() + "\"");
                return false;
            }
        }

        if (m_logger->genHadWarning()) {
            m_logger->genError("Warning treated as error");
            return false;
        }

        return true;
    }

    bool genWrite()
    {
        return std::all_of(
            m_schemas.begin(), m_schemas.end(),
            [this](auto& s)
            {
                m_currentSchema = s.get();
                return s->genWrite();
            });
    }

    bool genWasDirectoryCreated(const std::string& path) const
    {
        auto iter =
            std::find(m_createdDirectories.begin(), m_createdDirectories.end(), path);

        return iter != m_createdDirectories.end();
    }

    void genRecordCreatedDirectory(const std::string& path) const
    {
        m_createdDirectories.push_back(path);
    }

    const commsdsl::parse::ParseProtocol& genProtocol() const
    {
        return m_protocol;
    }

    void genReferenceAllMessages()
    {
        for (auto& sPtr : m_schemas) {
            sPtr->genSetAllMessagesReferenced();
        }
    }

    void genReferenceAllInterfaces()
    {
        for (auto& sPtr : m_schemas) {
            sPtr->genSetAllInterfacesReferenced();
        }
    }

    bool genGetAllMessagesReferencedByDefault() const
    {
        return m_allMessagesReferencedByDefault;
    }

    void genSetAllMessagesReferencedByDefault(bool value)
    {
        m_allMessagesReferencedByDefault = value;
    }

    bool genGetAllInterfacesReferencedByDefault() const
    {
        return m_allInterfacesReferencedByDefault;
    }

    void genSetAllInterfacesReferencedByDefault(bool value)
    {
        m_allInterfacesReferencedByDefault = value;
    }

private:
    std::pair<const GenSchema*, std::string> genParseExternalRef(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        if (externalRef[0] != strings::genSchemaRefPrefix()) {
            return std::make_pair(m_currentSchema, externalRef);
        }

        std::string schemaName;
        std::string restRef;

        auto dotPos = externalRef.find('.');
        if (externalRef.size() <= dotPos) {
            schemaName = externalRef.substr(1);
        }
        else {
            schemaName = externalRef.substr(1, dotPos - 1);
            restRef = externalRef.substr(dotPos + 1);
        }

        auto iter =
            std::find_if(
                m_schemas.begin(), m_schemas.end(),
                [&schemaName](auto& s)
                {
                    return schemaName == s->genName();
                });

        if (iter == m_schemas.end()) {
            return std::make_pair(nullptr, std::move(restRef));
        }

        return std::make_pair(iter->get(), std::move(restRef));
    }

    bool genPrepareDefaultInterfaceInternal()
    {
        auto allInterfaces = genGetAllInterfaces();
        if (!allInterfaces.empty()) {
            return true;
        }

        auto* defaultNamespace = genAddDefaultNamespace();
        auto* interface = defaultNamespace->genAddDefaultInterface();
        if (interface == nullptr) {
            m_logger->genError("Failed to genCreate default interface");
            return false;
        }

        return true;
    }

    GenGenerator& m_generator;
    commsdsl::parse::ParseProtocol m_protocol;
    GenLoggerPtr m_logger;
    GenSchemasList m_schemas;
    GenSchema* m_currentSchema = nullptr;
    std::map<std::string, std::string> m_namespaceOverrides;
    std::string m_topNamespace;
    int m_forcedSchemaVersion = -1;
    unsigned m_minRemoteVersion = 0U;
    std::string m_codeVersion;
    std::string m_outputDir;
    std::string m_codeDir;
    mutable std::vector<std::string> m_createdDirectories;
    bool m_versionIndependentCodeForced = false;
    bool m_allMessagesReferencedByDefault = true;
    bool m_allInterfacesReferencedByDefault = true;
};

GenGenerator::GenGenerator() :
    m_impl(std::make_unique<GenGeneratorImpl>(*this))
{
}

GenGenerator::~GenGenerator() = default;

int GenGenerator::genExec(const GenProgramOptions& options)
{
    auto optsResult = genProcessOptions(options);
    if (optsResult == commsdsl::gen::GenGenerator::OptsProcessResult_EarlyExit) {
        return 0;
    }

    if (optsResult == commsdsl::gen::GenGenerator::OptsProcessResult_Failure) {
        return -1;
    }

    auto files = options.genGetInputFiles();
    if (files.empty()) {
        genLogger().genError("No input files are provided");
        return -1;
    }

    if (!genPrepare(files)) {
        return -1;
    }

    if (!genWrite()) {
        return -2;
    }

    return 0;
}

void GenGenerator::genForceSchemaVersion(unsigned value)
{
    m_impl->genForceSchemaVersion(value);
}

void GenGenerator::genSetMinRemoteVersion(unsigned value)
{
    m_impl->genSetMinRemoteVersion(value);
}

unsigned GenGenerator::genGetMinRemoteVersion() const
{
    return m_impl->genGetMinRemoteVersion();
}

const std::string& GenGenerator::genGetCodeVersion() const
{
    return m_impl->genGetCodeVersion();
}

void GenGenerator::genSetCodeVersion(const std::string& value)
{
    m_impl->genSetCodeVersion(value);
}

void GenGenerator::genSetNamespaceOverride(const std::string& value)
{
    m_impl->genSetNamespaceOverride(value);
}

void GenGenerator::genSetTopNamespace(const std::string& value)
{
    m_impl->genSetTopNamespace(value);
}

const std::string& GenGenerator::genGetTopNamespace() const
{
    return m_impl->genGetTopNamespace();
}

void GenGenerator::genSetOutputDir(const std::string& outDir)
{
    m_impl->genSetOutputDir(outDir);
}

const std::string& GenGenerator::genGetOutputDir() const
{
    return m_impl->genGetOutputDir();
}

void GenGenerator::genSetCodeDir(const std::string& dir)
{
    m_impl->genSetCodeDir(dir);
}

const std::string& GenGenerator::genGetCodeDir() const
{
    return m_impl->genGetCodeDir();
}

void GenGenerator::genSetMultipleSchemasEnabled(bool enabled)
{
    m_impl->genSetMultipleSchemasEnabled(enabled);
}

bool GenGenerator::genGetMultipleSchemasEnabled() const
{
    return m_impl->genGetMultipleSchemasEnabled();
}

void GenGenerator::genSetVersionIndependentCodeForced(bool value)
{
    m_impl->genSetVersionIndependentCodeForced(value);
}

bool GenGenerator::genGetVersionIndependentCodeForced() const
{
    return m_impl->genGetVersionIndependentCodeForced();
}

const GenField* GenGenerator::genFindField(const std::string& externalRef) const
{
    auto* field = m_impl->genFindField(externalRef);
    assert((field == nullptr) || (field->genIsPrepared()));
    return field;
}

GenField* GenGenerator::genFindField(const std::string& externalRef)
{
    auto* field = m_impl->genFindField(externalRef);
    do {
        if (field->genIsPrepared()) {
            break;
        }

        if (field->genPrepare()) {
            break;
        }

        genLogger().genWarning("Failed to prepare field: " + field->genParseObj().parseExternalRef());
        field = nullptr;
    } while (false);
    return field;
}

const GenMessage* GenGenerator::genGindMessage(const std::string& externalRef) const
{
    return m_impl->genGindMessage(externalRef);
}

GenMessage* GenGenerator::genGindMessage(const std::string& externalRef)
{
    auto* msg = m_impl->genGindMessage(externalRef);
    do {
        if (msg->genIsPrepared()) {
            break;
        }

        if (msg->genPrepare()) {
            break;
        }

        genLogger().genWarning("Failed to prepare message: " + msg->genParseObj().parseExternalRef());
        msg = nullptr;
    } while (false);
    return msg;
}

const GenFrame* GenGenerator::genFindFrame(const std::string& externalRef) const
{
    return m_impl->genFindFrame(externalRef);
}

const GenInterface* GenGenerator::genFindInterface(const std::string& externalRef) const
{
    return m_impl->genFindInterface(externalRef);
}

const GenSchema& GenGenerator::genSchemaOf(const GenElem& elem)
{
    auto* parent = elem.genGetParent();
    assert(parent != nullptr);
    if (parent->genElemType() == GenElem::GenType_Schema) {
        return static_cast<const GenSchema&>(*parent);
    }

    return genSchemaOf(*parent);
}

GenGenerator::GenNamespacesAccessList GenGenerator::genGetAllNamespaces() const
{
    return genCurrentSchema().genGetAllNamespaces();
}

GenGenerator::GenInterfacesAccessList GenGenerator::genGetAllInterfaces() const
{
    return genCurrentSchema().genGetAllInterfaces();
}

GenGenerator::GenMessagesAccessList GenGenerator::genGetAllMessages() const
{
    return genCurrentSchema().genGetAllMessages();
}

void GenGenerator::genSortMessages(GenMessagesAccessList& list)
{
    std::sort(
        list.begin(), list.end(),
        [](auto* msg1, auto* msg2)
        {
            auto id1 = msg1->genParseObj().parseId();
            auto id2 = msg2->genParseObj().parseId();

            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1->genParseObj().parseOrder() < msg2->genParseObj().parseOrder();
        });
}

GenGenerator::GenMessagesAccessList GenGenerator::genGetAllMessagesIdSorted() const
{
    auto result = genGetAllMessages();
    genSortMessages(result);
    return result;
}

GenGenerator::GenFramesAccessList GenGenerator::genGetAllFrames() const
{
    return genCurrentSchema().genGetAllFrames();
}

GenGenerator::GenFieldsAccessList GenGenerator::genGetAllFields() const
{
    return genCurrentSchema().genGetAllFields();
}

GenGenerator::GenNamespacesAccessList GenGenerator::genGetAllNamespacesFromAllSchemas() const
{
    GenNamespacesAccessList result;
    for (auto& sPtr : genSchemas()) {
        auto list = sPtr->genGetAllNamespaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::GenInterfacesAccessList GenGenerator::genGetAllInterfacesFromAllSchemas() const
{
    GenInterfacesAccessList result;
    for (auto& sPtr : genSchemas()) {
        auto list = sPtr->genGetAllInterfaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::GenMessagesAccessList GenGenerator::genGetAllMessagesFromAllSchemas() const
{
    GenMessagesAccessList result;
    for (auto& sPtr : genSchemas()) {
        auto list = sPtr->genGetAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::GenMessagesAccessList GenGenerator::genGetAllMessagesIdSortedFromAllSchemas() const
{
    auto result = genGetAllMessagesFromAllSchemas();
    genSortMessages(result);
    return result;
}

GenGenerator::GenFramesAccessList GenGenerator::genGetAllFramesFromAllSchemas() const
{
    GenFramesAccessList result;
    for (auto& sPtr : genSchemas()) {
        auto list = sPtr->genGetAllFrames();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::GenFieldsAccessList GenGenerator::genGetAllFieldsFromAllSchemas() const
{
    GenFieldsAccessList result;
    for (auto& sPtr : genSchemas()) {
        auto list = sPtr->genGetAllFields();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

bool GenGenerator::genPrepare(const GenFilesList& files)
{
    // Make sure the logger is created
    [[maybe_unused]] auto& l = genLogger();

    auto createCompleteFunc =
        [this]()
        {
            return genCreateCompleteImpl();
        };

    if (!m_impl->genPrepare(files, createCompleteFunc)) {
        return false;
    }

    return genPrepareImpl();
}

bool GenGenerator::genWrite()
{
    auto& outDir = genGetOutputDir();
    if ((!outDir.empty()) && (!genCreateDirectory(outDir))) {
        return false;
    }

    if (!m_impl->genWrite()) {
        return false;
    }

    return genWriteImpl();
}

bool GenGenerator::genDoesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return genCurrentSchema().genDoesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenGenerator::genIsElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return genCurrentSchema().genIsElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenGenerator::genIsElementDeprecated(unsigned deprecatedSince) const
{
    return genCurrentSchema().genIsElementDeprecated(deprecatedSince);
}

GenLogger& GenGenerator::genLogger()
{
    auto& loggerPtr = m_impl->genGetLogger();
    if (loggerPtr) {
        return *loggerPtr;
    }

    auto newLogger = genCreateLoggerImpl();
    if (!newLogger) {
        newLogger = GenGenerator::genCreateLoggerImpl();
        assert(newLogger);
    }

    auto& logger = *newLogger;
    m_impl->genSetLogger(std::move(newLogger));
    return logger;
}

const GenLogger& GenGenerator::genLogger() const
{
    auto& loggerPtr = m_impl->genGetLogger();
    assert(loggerPtr);
    return *loggerPtr;
}

const GenGenerator::GenSchemasList& GenGenerator::genSchemas() const
{
    return m_impl->genSchemas();
}

GenSchema& GenGenerator::genCurrentSchema()
{
    return m_impl->genCurrentSchema();
}

const GenSchema& GenGenerator::genCurrentSchema() const
{
    return m_impl->genCurrentSchema();
}

GenSchema& GenGenerator::genProtocolSchema()
{
    return m_impl->genProtocolSchema();
}

const GenSchema& GenGenerator::genProtocolSchema() const
{
    return m_impl->genProtocolSchema();
}

bool GenGenerator::genIsCurrentProtocolSchema() const
{
    return &genCurrentSchema() == &genProtocolSchema();
}

GenSchemaPtr GenGenerator::genCreateSchema(ParseSchema parseObj, GenElem* parent)
{
    return genCreateSchemaImpl(parseObj, parent);
}

GenNamespacePtr GenGenerator::genCreateNamespace(ParseNamespace parseObj, GenElem* parent)
{
    return genCreateNamespaceImpl(parseObj, parent);
}

GenInterfacePtr GenGenerator::genCreateInterface(ParseInterface parseObj, GenElem* parent)
{
    return genCreateInterfaceImpl(parseObj, parent);
}

GenMessagePtr GenGenerator::genCreateMessage(ParseMessage parseObj, GenElem* parent)
{
    return genCreateMessageImpl(parseObj, parent);
}

GenFramePtr GenGenerator::genCreateFrame(ParseFrame parseObj, GenElem* parent)
{
    return genCreateFrameImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateIntField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Int);
    return genCreateIntFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateEnumField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Enum);
    return genCreateEnumFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateSetField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Set);
    return genCreateSetFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateFloatField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Float);
    return genCreateFloatFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateBitfieldField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Bitfield);
    return genCreateBitfieldFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateBundleField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Bundle);
    return genCreateBundleFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateStringField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::String);
    return genCreateStringFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateDataField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Data);
    return genCreateDataFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateListField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::List);
    return genCreateListFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateRefField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Ref);
    return genCreateRefFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateOptionalField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Optional);
    return genCreateOptionalFieldImpl(parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateVariantField(ParseField parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Variant);
    return genCreateVariantFieldImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateCustomLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Custom);
    return genCreateCustomLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateSyncLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Sync);
    return genCreateSyncLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateSizeLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Size);
    return genCreateSizeLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateIdLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Id);
    return genCreateIdLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateValueLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Value);
    return genCreateValueLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreatePayloadLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Payload);
    return genCreatePayloadLayerImpl(parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateChecksumLayer(ParseLayer parseObj, GenElem* parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Checksum);
    return genCreateChecksumLayerImpl(parseObj, parent);
}

unsigned GenGenerator::genCurrentSchemaIdx() const
{
    return m_impl->genCurrentSchemaIdx();
}

void GenGenerator::genChooseCurrentSchema(unsigned idx)
{
    m_impl->genChooseCurrentSchema(idx);
}

void GenGenerator::genChooseCurrentSchema(unsigned idx) const
{
    const_cast<GenGenerator*>(this)->genChooseCurrentSchema(idx);
}

void GenGenerator::genChooseProtocolSchema()
{
    assert(!genSchemas().empty());
    genChooseCurrentSchema(static_cast<unsigned>(genSchemas().size() - 1U));
}

void GenGenerator::genChooseProtocolSchema() const
{
    const_cast<GenGenerator*>(this)->genChooseProtocolSchema();
}

bool GenGenerator::genCreateDirectory(const std::string& path) const
{
    if (m_impl->genWasDirectoryCreated(path)) {
        return true;
    }

    std::error_code ec;
    if (fs::is_directory(path, ec)) {
        m_impl->genRecordCreatedDirectory(path);
        return true;
    }

    fs::create_directories(path, ec);
    if (ec) {
        genLogger().genError("Failed to genCreate directory \"" + path + "\" with error: " + ec.message());
        return false;
    }

    m_impl->genRecordCreatedDirectory(path);
    return true;
}

void GenGenerator::genReferenceAllMessages()
{
    m_impl->genReferenceAllMessages();
}

bool GenGenerator::genGetAllMessagesReferencedByDefault() const
{
    return m_impl->genGetAllMessagesReferencedByDefault();
}

void GenGenerator::genSetAllMessagesReferencedByDefault(bool value)
{
    m_impl->genSetAllMessagesReferencedByDefault(value);
}

void GenGenerator::genReferenceAllInterfaces()
{
    m_impl->genReferenceAllInterfaces();
}

bool GenGenerator::genGetAllInterfacesReferencedByDefault() const
{
    return m_impl->genGetAllInterfacesReferencedByDefault();
}

void GenGenerator::genSetAllInterfacesReferencedByDefault(bool value)
{
    m_impl->genSetAllInterfacesReferencedByDefault(value);
}

GenGenerator::OptsProcessResult GenGenerator::genProcessOptions(const GenProgramOptions& options)
{
    if (options.genHelpRequested()) {
        std::cout << "Usage:\n\t" << options.genApp() << " [OPTIONS] schema_file1 [schema_file2] [schema_file3] ...\n\n";
        std::cout << options.genHelpStr();
        return OptsProcessResult_EarlyExit;
    }

    if (options.genVersionRequested()) {
        std::cout <<
            commsdsl::versionMajor() << '.' <<
            commsdsl::versionMinor() << '.' <<
            commsdsl::versionPatch() << std::endl;
        return OptsProcessResult_EarlyExit;
    }

    auto& logger = genLogger();
    if (options.genQuietRequested() && options.genDebugRequested()) {
        logger.genError("Cannot use both --quiet and --debug options at the same time");
        return OptsProcessResult_Failure;
    }

    if (options.genQuietRequested()) {
        logger.genSetMinLevel(commsdsl::parse::ParseErrorLevel_Warning);
    }

    if (options.genDebugRequested()) {
        logger.genSetMinLevel(commsdsl::parse::ParseErrorLevel_Debug);
    }

    if (options.genWarnAsErrRequested()) {
        logger.genSetWarnAsError();
    }

    if (options.genHasNamespaceOverride()) {
        genSetNamespaceOverride(options.genGetNamespace());
    }

    if (options.genHasForcedSchemaVersion()) {
        genForceSchemaVersion(options.genGetForcedSchemaVersion());
    }

    genSetOutputDir(options.genGetOutputDirectory());
    genSetCodeDir(options.genGetCodeInputDirectory());
    genSetMultipleSchemasEnabled(options.genMultipleSchemasEnabled());
    genSetMinRemoteVersion(options.genGetMinRemoteVersion());
    genSetCodeVersion(options.genGetCodeVersion());

    return genProcessOptionsImpl(options);
}

bool GenGenerator::genCreateCompleteImpl()
{
    return true;
}

bool GenGenerator::genPrepareImpl()
{
    return true;
}

GenSchemaPtr GenGenerator::genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent)
{
    return std::make_unique<GenSchema>(*this, parseObj, parent);
}

GenNamespacePtr GenGenerator::genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent)
{
    return std::make_unique<GenNamespace>(*this, parseObj, parent);
}

GenInterfacePtr GenGenerator::genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent)
{
    return std::make_unique<GenInterface>(*this, parseObj, parent);
}

GenMessagePtr GenGenerator::genCreateMessageImpl(ParseMessage parseObj, GenElem* parent)
{
    return std::make_unique<GenMessage>(*this, parseObj, parent);
}

GenFramePtr GenGenerator::genCreateFrameImpl(ParseFrame parseObj, GenElem* parent)
{
    return std::make_unique<GenFrame>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateIntFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenIntField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenEnumField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateSetFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenSetField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenFloatField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenBitfieldField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenBundleField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateStringFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenStringField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateDataFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenDataField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateListFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenListField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateRefFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenRefField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenOptionalField>(*this, parseObj, parent);
}

GenFieldPtr GenGenerator::genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<GenVariantField>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenCustomLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenSyncLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenSizeLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenIdLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenValueLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenPayloadLayer>(*this, parseObj, parent);
}

GenLayerPtr GenGenerator::genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<GenChecksumLayer>(*this, parseObj, parent);
}

bool GenGenerator::genWriteImpl()
{
    return true;
}

GenGenerator::GenLoggerPtr GenGenerator::genCreateLoggerImpl()
{
    return std::make_unique<GenLogger>();
}

GenGenerator::OptsProcessResult GenGenerator::genProcessOptionsImpl([[maybe_unused]] const GenProgramOptions& options)
{
    return OptsProcessResult_Continue;
}

GenNamespace* GenGenerator::genAddDefaultNamespace()
{
    return genCurrentSchema().genAddDefaultNamespace();
}

bool GenGenerator::genCopyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const
{
    auto& inputDir = genGetCodeDir();
    if (inputDir.empty()) {
        return true;
    }

    auto& outputDir = genGetOutputDir();
    auto pos = inputDir.size();
    auto endIter = fs::recursive_directory_iterator();
    for (auto iter = fs::recursive_directory_iterator(inputDir); iter != endIter; ++iter) {
        if (!iter->is_regular_file()) {
            continue;
        }

        auto srcPath = iter->path();
        auto ext = srcPath.extension().string();

        auto extIter = std::find(reservedExtensions.begin(), reservedExtensions.end(), ext);
        if (extIter != reservedExtensions.end()) {
            continue;
        }

        auto pathStr = srcPath.string();
        auto posTmp = pos;
        while (posTmp < pathStr.size()) {
            if (pathStr[posTmp] == fs::path::preferred_separator) {
                ++posTmp;
                continue;
            }
            break;
        }

        if (pathStr.size() <= posTmp) {
            continue;
        }

        std::string relPath(pathStr, posTmp);
        auto& protSchema = genProtocolSchema();
        auto schemaNs = util::genStrToName(protSchema.genSchemaName());
        do {
            if (protSchema.genMainNamespace() == schemaNs) {
                break;
            }

            auto srcPrefix = (fs::path(strings::genIncludeDirStr()) / schemaNs).string();
            if (!util::genStrStartsWith(relPath, srcPrefix)) {
                break;
            }

            auto dstPrefix = (fs::path(strings::genIncludeDirStr()) / protSchema.genMainNamespace()).string();
            relPath = dstPrefix + std::string(relPath, srcPrefix.size());
        } while (false);

        auto destPath = fs::path(outputDir) / relPath;
        genLogger().genInfo("Copying " + destPath.string());

        if (!genCreateDirectory(destPath.parent_path().string())) {
            return false;
        }

        std::error_code ec;
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            genLogger().genError("Failed to copy with reason: " + ec.message());
            return false;
        }

        if (protSchema.genMainNamespace() != schemaNs) {
            // The namespace has changed

            auto destStr = destPath.string();
            std::ifstream stream(destStr);
            if (!stream) {
                genLogger().genError("Failed to open " + destStr + " for modification.");
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            stream.close();

            content = util::genStrReplace(content, "namespace " + schemaNs, "namespace " + protSchema.genMainNamespace());
            std::ofstream outStream(destStr, std::ios_base::trunc);
            if (!outStream) {
                genLogger().genError("Failed to modify " + destStr + ".");
                return false;
            }

            outStream << content;
            genLogger().genInfo("Updated " + destStr + " to have proper main namespace.");
        }
    }
    return true;
}

} // namespace gen

} // namespace commsdsl
