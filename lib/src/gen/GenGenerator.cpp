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

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <fstream>
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
    using LoggerPtr = GenGenerator::LoggerPtr;
    using FilesList = GenGenerator::FilesList;
    using NamespacesList = GenGenerator::NamespacesList;
    using PlatformNamesList = GenGenerator::PlatformNamesList;
    using SchemasList = GenGenerator::SchemasList;
    using InterfacesAccessList = GenGenerator::InterfacesAccessList;

    explicit GenGeneratorImpl(GenGenerator& generator) :
        m_generator(generator),
        m_outputDir(fs::current_path().string())
    {
    }

    LoggerPtr& getLogger()
    {
        return m_logger;
    }

    const LoggerPtr& getLogger() const
    {
        return m_logger;
    }    

    void setLogger(LoggerPtr logger)
    {
        m_logger = std::move(logger);
    }

    const SchemasList& schemas() const
    {
        return m_schemas;
    }

    GenSchema& currentSchema()
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    const GenSchema& currentSchema() const
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    GenSchema& protocolSchema()
    {
        assert(!m_schemas.empty());
        return *m_schemas.back();
    }

    const GenSchema& protocolSchema() const
    {
        assert(!m_schemas.empty());
        return *m_schemas.back();
    }

    void chooseCurrentSchema(unsigned idx)
    {
        assert(idx < m_schemas.size());
        m_currentSchema = m_schemas[idx].get();
    }

    unsigned currentSchemaIdx() const
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

    void forceSchemaVersion(unsigned value)
    {
        m_forcedSchemaVersion = static_cast<decltype(m_forcedSchemaVersion)>(value);
    }

    void setMinRemoteVersion(unsigned value)
    {
        m_minRemoteVersion = value;
    }

    unsigned getMinRemoteVersion() const
    {
        return m_minRemoteVersion;
    }

    InterfacesAccessList getAllInterfaces() const
    {
        return currentSchema().getAllInterfaces();
    }    

    GenNamespace* addDefaultNamespace()
    {
        return currentSchema().addDefaultNamespace();
    }
    

    void setNamespaceOverride(const std::string& value)
    {
        m_namespaceOverrides.clear();
        if (value.empty()) {
            return;
        }

        auto elems = util::strSplitByAnyChar(value, ",");
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

    void setTopNamespace(const std::string& value)
    {
        m_topNamespace = value;
    }

    const std::string& getTopNamespace() const
    {
        return m_topNamespace;
    }

    void setOutputDir(const std::string& outDir)
    {
        if (!outDir.empty()) {
            m_outputDir = outDir;
        }
    }

    const std::string& getOutputDir() const
    {
        return m_outputDir;
    }

    void setCodeDir(const std::string& dir)
    {
        m_codeDir = dir;
    }

    const std::string& getCodeDir() const
    {
        return m_codeDir;
    }

    void setMultipleSchemasEnabled(bool enabled)
    {
        m_protocol.setMultipleSchemasEnabled(enabled);
    }

    bool getMultipleSchemasEnabled() const
    {
        return m_protocol.getMultipleSchemasEnabled();
    }

    void setVersionIndependentCodeForced(bool value)
    {
        m_versionIndependentCodeForced = value;
    }

    bool getVersionIndependentCodeForced() const
    {
        return m_versionIndependentCodeForced;
    }

    const GenField* findField(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findField(parsedRef.second);
    }

    GenField* findField(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<GenSchema*>(parsedRef.first)->findField(parsedRef.second);
    }

    const GenMessage* findMessage(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findMessage(parsedRef.second);
    }  

    GenMessage* findMessage(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<GenSchema*>(parsedRef.first)->findMessage(parsedRef.second);
    }

    const GenFrame* findFrame(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findFrame(parsedRef.second);
    }

    const GenInterface* findInterface(const std::string& externalRef) const
    {
        if (externalRef.empty()) {
            return currentSchema().findInterface(externalRef);
        }
        
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findInterface(parsedRef.second);
    }                

    using CreateCompleteFunc = std::function<bool ()>;
    bool prepare(const FilesList& files, CreateCompleteFunc createCompleteCb = CreateCompleteFunc())
    {
        m_protocol.setErrorReportCallback(
            [this](commsdsl::parse::ParseErrorLevel level, const std::string& msg)
            {
                assert(m_logger);
                m_logger->log(level, msg);
            });

        assert(m_logger);
        for (auto& f : files) {
            m_logger->info("Parsing " + f);
            if (!m_protocol.parse(f)) {
                return false;
            }

            if (m_logger->hadWarning()) {
                m_logger->error("Warning treated as error");
                return false;
            }
        }

        if (!m_protocol.validate()) {
            return false;
        }

        if (m_logger->hadWarning()) {
            m_logger->error("Warning treated as error");
            return false;
        }

        auto allSchemas = m_protocol.schemas();
        if (allSchemas.empty()) {
            m_logger->error("No schemas available");
            return false;
        }

        for (auto& s : allSchemas) {
            auto schema = m_generator.createSchema(s);
            schema->setVersionIndependentCodeForced(m_versionIndependentCodeForced);

            auto renameIter = m_namespaceOverrides.find(util::strToName(s.name()));
            if (renameIter != m_namespaceOverrides.end()) {
                schema->setMainNamespaceOverride(renameIter->second);
            }
            m_schemas.push_back(std::move(schema));
        }

        assert(!m_schemas.empty());
        auto& protocolSchemaPtr = m_schemas.back();
        
        protocolSchemaPtr->setMinRemoteVersion(m_minRemoteVersion);
        if (0 <= m_forcedSchemaVersion) {
            protocolSchemaPtr->forceSchemaVersion(static_cast<unsigned>(m_forcedSchemaVersion));
        }

        auto renameIter = m_namespaceOverrides.find(strings::emptyString());
        if (renameIter != m_namespaceOverrides.end()) {
            protocolSchemaPtr->setMainNamespaceOverride(renameIter->second);
        }

        for (auto& s : m_schemas) {
            m_currentSchema = s.get();
            if (!s->createAll()) {
                m_logger->error("Failed to create elements inside schema \"" + s->dslObj().name() + "\"");
                return false;
            }       

            if (m_allInterfacesReferencedByDefault) {
                s->setAllInterfacesReferenced();
            }                 

            if (m_allMessagesReferencedByDefault) {
                s->setAllMessagesReferenced();
            }
        }   
        
        if (createCompleteCb && (!createCompleteCb())) {
            return false;
        }

        for (auto& s : m_schemas) {
            m_currentSchema = s.get();
            if (!s->prepare()) {
                m_logger->error("Failed to prepare elements inside schema \"" + s->dslObj().name() + "\"");
                return false;
            }            
        }               


        if (m_logger->hadWarning()) {
            m_logger->error("Warning treated as error");
            return false;
        }      
        
        if (!prepareDefaultInterfaceInternal()) {
            return false;
        }

        return true;
    }

    bool write()
    {
        return std::all_of(
            m_schemas.begin(), m_schemas.end(),
            [this](auto& s)
            {
                m_currentSchema = s.get();
                return s->write();
            });
    }

    bool wasDirectoryCreated(const std::string& path) const
    {
        auto iter = 
            std::find(m_createdDirectories.begin(), m_createdDirectories.end(), path);

        return iter != m_createdDirectories.end();
    }

    void recordCreatedDirectory(const std::string& path) const
    {
        m_createdDirectories.push_back(path);
    }

    const commsdsl::parse::ParseProtocol& protocol() const
    {
        return m_protocol;
    }

    void referenceAllMessages()
    {
        for (auto& sPtr : m_schemas) {
            sPtr->setAllMessagesReferenced();
        }
    }

    void referenceAllInterfaces()
    {
        for (auto& sPtr : m_schemas) {
            sPtr->setAllInterfacesReferenced();
        }
    }    

    bool getAllMessagesReferencedByDefault() const
    {
        return m_allMessagesReferencedByDefault;
    }

    void setAllMessagesReferencedByDefault(bool value)
    {
        m_allMessagesReferencedByDefault = value;
    }

    bool getAllInterfacesReferencedByDefault() const
    {
        return m_allInterfacesReferencedByDefault;
    }

    void setAllInterfacesReferencedByDefault(bool value)
    {
        m_allInterfacesReferencedByDefault = value;
    }

private:
    std::pair<const GenSchema*, std::string> parseExternalRef(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        if (externalRef[0] != strings::schemaRefPrefix()) {
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
                    return schemaName == s->name();
                });

        if (iter == m_schemas.end()) {
            return std::make_pair(nullptr, std::move(restRef));
        }

        return std::make_pair(iter->get(), std::move(restRef));
    }

    bool prepareDefaultInterfaceInternal()
    {
        auto allInterfaces = getAllInterfaces();
        if (!allInterfaces.empty()) {
            return true;
        }
    
        auto* defaultNamespace = addDefaultNamespace();
        auto* interface = defaultNamespace->addDefaultInterface();
        if (interface == nullptr) {
            m_logger->error("Failed to create default interface");
            return false;
        }
    
        return true;        
    }

    GenGenerator& m_generator;
    commsdsl::parse::ParseProtocol m_protocol;
    LoggerPtr m_logger;
    SchemasList m_schemas;
    GenSchema* m_currentSchema = nullptr;
    std::map<std::string, std::string> m_namespaceOverrides;
    std::string m_topNamespace;
    int m_forcedSchemaVersion = -1;
    unsigned m_minRemoteVersion = 0U;
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

void GenGenerator::forceSchemaVersion(unsigned value)
{
    m_impl->forceSchemaVersion(value);
}

void GenGenerator::setMinRemoteVersion(unsigned value)
{
    m_impl->setMinRemoteVersion(value);
}

unsigned GenGenerator::getMinRemoteVersion() const
{
    return m_impl->getMinRemoteVersion();
}

void GenGenerator::setNamespaceOverride(const std::string& value)
{
    m_impl->setNamespaceOverride(value);
}

void GenGenerator::setTopNamespace(const std::string& value)
{
    m_impl->setTopNamespace(value);
}

const std::string& GenGenerator::getTopNamespace() const
{
    return m_impl->getTopNamespace();
}

void GenGenerator::setOutputDir(const std::string& outDir)
{
    m_impl->setOutputDir(outDir);
}

const std::string& GenGenerator::getOutputDir() const
{
    return m_impl->getOutputDir();
}

void GenGenerator::setCodeDir(const std::string& dir)
{
    m_impl->setCodeDir(dir);
}

const std::string& GenGenerator::getCodeDir() const
{
    return m_impl->getCodeDir();
}

void GenGenerator::setMultipleSchemasEnabled(bool enabled)
{
    m_impl->setMultipleSchemasEnabled(enabled);
}

bool GenGenerator::getMultipleSchemasEnabled() const
{
    return m_impl->getMultipleSchemasEnabled();
}

void GenGenerator::setVersionIndependentCodeForced(bool value)
{
    m_impl->setVersionIndependentCodeForced(value);
}

bool GenGenerator::getVersionIndependentCodeForced() const
{
    return m_impl->getVersionIndependentCodeForced();
}

const GenField* GenGenerator::findField(const std::string& externalRef) const
{
    auto* field = m_impl->findField(externalRef);
    assert((field == nullptr) || (field->isPrepared()));
    return field;
}

GenField* GenGenerator::findField(const std::string& externalRef)
{
    auto* field = m_impl->findField(externalRef);
    do {
        if (field->isPrepared()) {
            break;
        }    

        if (field->prepare()) {
            break;
        }
         
        logger().warning("Failed to prepare field: " + field->dslObj().externalRef());
        field = nullptr;
    } while (false);
    return field;
}

const GenMessage* GenGenerator::findMessage(const std::string& externalRef) const
{
    return m_impl->findMessage(externalRef);
}

GenMessage* GenGenerator::findMessage(const std::string& externalRef) 
{
    auto* msg = m_impl->findMessage(externalRef);
    do {
        if (msg->isPrepared()) {
            break;
        }

        if (msg->prepare()) {
            break;
        }

        logger().warning("Failed to prepare message: " + msg->dslObj().externalRef());
        msg = nullptr;
    } while (false);
    return msg;
}

const GenFrame* GenGenerator::findFrame(const std::string& externalRef) const
{
    return m_impl->findFrame(externalRef);
}

const GenInterface* GenGenerator::findInterface(const std::string& externalRef) const
{
    return m_impl->findInterface(externalRef);
}

const GenSchema& GenGenerator::schemaOf(const GenElem& elem)
{
    auto* parent = elem.getParent();
    assert(parent != nullptr);
    if (parent->elemType() == GenElem::Type_Schema) {
        return static_cast<const GenSchema&>(*parent);
    }

    return schemaOf(*parent);
}

GenGenerator::NamespacesAccessList GenGenerator::getAllNamespaces() const
{
    return currentSchema().getAllNamespaces();
}

GenGenerator::InterfacesAccessList GenGenerator::getAllInterfaces() const
{
    return currentSchema().getAllInterfaces();
}

GenGenerator::MessagesAccessList GenGenerator::getAllMessages() const
{
    return currentSchema().getAllMessages();
}

void GenGenerator::sortMessages(MessagesAccessList& list)
{
    std::sort(
        list.begin(), list.end(),
        [](auto* msg1, auto* msg2)
        {
            auto id1 = msg1->dslObj().id();
            auto id2 = msg2->dslObj().id();

            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1->dslObj().order() < msg2->dslObj().order();
        });
}

GenGenerator::MessagesAccessList GenGenerator::getAllMessagesIdSorted() const
{
    auto result = getAllMessages();
    sortMessages(result);
    return result;
}

GenGenerator::FramesAccessList GenGenerator::getAllFrames() const
{
    return currentSchema().getAllFrames();
}

GenGenerator::FieldsAccessList GenGenerator::getAllFields() const
{
    return currentSchema().getAllFields();
}

GenGenerator::NamespacesAccessList GenGenerator::getAllNamespacesFromAllSchemas() const
{
    NamespacesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllNamespaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::InterfacesAccessList GenGenerator::getAllInterfacesFromAllSchemas() const
{
    InterfacesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllInterfaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::MessagesAccessList GenGenerator::getAllMessagesFromAllSchemas() const
{
    MessagesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::MessagesAccessList GenGenerator::getAllMessagesIdSortedFromAllSchemas() const
{
    auto result = getAllMessagesFromAllSchemas();
    sortMessages(result);
    return result;    
}

GenGenerator::FramesAccessList GenGenerator::getAllFramesFromAllSchemas() const
{
    FramesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllFrames();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

GenGenerator::FieldsAccessList GenGenerator::getAllFieldsFromAllSchemas() const
{
    FieldsAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllFields();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

bool GenGenerator::prepare(const FilesList& files)
{
    // Make sure the logger is created
    [[maybe_unused]] auto& l = logger();

    auto createCompleteFunc = 
        [this]()
        {
            return createCompleteImpl();
        };

    if (!m_impl->prepare(files, createCompleteFunc)) {
        return false;
    }

    return prepareImpl();
}

bool GenGenerator::write()
{
    auto& outDir = getOutputDir();
    if ((!outDir.empty()) && (!createDirectory(outDir))) {
        return false;
    }

    if (!m_impl->write()) {
        return false;
    }
    
    return writeImpl();
}

bool GenGenerator::doesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return currentSchema().doesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenGenerator::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return currentSchema().isElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenGenerator::isElementDeprecated(unsigned deprecatedSince) const
{
    return currentSchema().isElementDeprecated(deprecatedSince);
} 

GenLogger& GenGenerator::logger()
{
    auto& loggerPtr = m_impl->getLogger();
    if (loggerPtr) {
        return *loggerPtr;
    }

    auto newLogger = createLoggerImpl();
    if (!newLogger) {
        newLogger = GenGenerator::createLoggerImpl();
        assert(newLogger);
    }
    
    auto& logger = *newLogger;
    m_impl->setLogger(std::move(newLogger));
    return logger;    
}

const GenLogger& GenGenerator::logger() const
{
    auto& loggerPtr = m_impl->getLogger();
    assert(loggerPtr);
    return *loggerPtr;
}

const GenGenerator::SchemasList& GenGenerator::schemas() const
{
    return m_impl->schemas();
}

GenSchema& GenGenerator::currentSchema()
{
    return m_impl->currentSchema();
}

const GenSchema& GenGenerator::currentSchema() const
{
    return m_impl->currentSchema();
}

GenSchema& GenGenerator::protocolSchema()
{
    return m_impl->protocolSchema();
}

const GenSchema& GenGenerator::protocolSchema() const
{
    return m_impl->protocolSchema();
}

bool GenGenerator::isCurrentProtocolSchema() const
{
    return &currentSchema() == &protocolSchema();
}

SchemaPtr GenGenerator::createSchema(commsdsl::parse::ParseSchema dslObj, GenElem* parent)
{
    return createSchemaImpl(dslObj, parent);
}

NamespacePtr GenGenerator::createNamespace(commsdsl::parse::ParseNamespace dslObj, GenElem* parent)
{
    return createNamespaceImpl(dslObj, parent);
}

InterfacePtr GenGenerator::createInterface(commsdsl::parse::ParseInterface dslObj, GenElem* parent)
{
    return createInterfaceImpl(dslObj, parent);
}

MessagePtr GenGenerator::createMessage(commsdsl::parse::ParseMessage dslObj, GenElem* parent)
{
    return createMessageImpl(dslObj, parent);
}

FramePtr GenGenerator::createFrame(commsdsl::parse::ParseFrame dslObj, GenElem* parent)
{
    return createFrameImpl(dslObj, parent);
}

FieldPtr GenGenerator::createIntField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Int);
    return createIntFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createEnumField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Enum);
    return createEnumFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createSetField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Set);
    return createSetFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createFloatField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Float);
    return createFloatFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createBitfieldField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Bitfield);
    return createBitfieldFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createBundleField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Bundle);
    return createBundleFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createStringField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::String);
    return createStringFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createDataField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Data);
    return createDataFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createListField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::List);
    return createListFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createRefField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Ref);
    return createRefFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createOptionalField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Optional);
    return createOptionalFieldImpl(dslObj, parent);
}

FieldPtr GenGenerator::createVariantField(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Variant);
    return createVariantFieldImpl(dslObj, parent);
}

LayerPtr GenGenerator::createCustomLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Custom);
    return createCustomLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createSyncLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Sync);
    return createSyncLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createSizeLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Size);
    return createSizeLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createIdLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Id);
    return createIdLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createValueLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Value);
    return createValueLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createPayloadLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Payload);
    return createPayloadLayerImpl(dslObj, parent);
}

LayerPtr GenGenerator::createChecksumLayer(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::ParseLayer::Kind::Checksum);
    return createChecksumLayerImpl(dslObj, parent);
}

unsigned GenGenerator::currentSchemaIdx() const
{
    return m_impl->currentSchemaIdx();
}

void GenGenerator::chooseCurrentSchema(unsigned idx)
{
    m_impl->chooseCurrentSchema(idx);
}

void GenGenerator::chooseCurrentSchema(unsigned idx) const
{
    const_cast<GenGenerator*>(this)->chooseCurrentSchema(idx);
}

void GenGenerator::chooseProtocolSchema()
{
    assert(!schemas().empty());
    chooseCurrentSchema(static_cast<unsigned>(schemas().size() - 1U));
}

void GenGenerator::chooseProtocolSchema() const
{
    const_cast<GenGenerator*>(this)->chooseProtocolSchema();
}

bool GenGenerator::createDirectory(const std::string& path) const
{
    if (m_impl->wasDirectoryCreated(path)) {
        return true;
    }

    std::error_code ec;
    if (fs::is_directory(path, ec)) {
        m_impl->recordCreatedDirectory(path);
        return true;
    }

    fs::create_directories(path, ec);
    if (ec) {
        logger().error("Failed to create directory \"" + path + "\" with error: " + ec.message());
        return false;
    }

    m_impl->recordCreatedDirectory(path);
    return true;
}

void GenGenerator::referenceAllMessages()
{
    m_impl->referenceAllMessages();
}

bool GenGenerator::getAllMessagesReferencedByDefault() const
{
    return m_impl->getAllMessagesReferencedByDefault();
}

void GenGenerator::setAllMessagesReferencedByDefault(bool value)
{
    m_impl->setAllMessagesReferencedByDefault(value);
}

void GenGenerator::referenceAllInterfaces()
{
    m_impl->referenceAllInterfaces();
}

bool GenGenerator::getAllInterfacesReferencedByDefault() const
{
    return m_impl->getAllInterfacesReferencedByDefault();
}

void GenGenerator::setAllInterfacesReferencedByDefault(bool value)
{
    m_impl->setAllInterfacesReferencedByDefault(value);
}    

bool GenGenerator::createCompleteImpl()
{
    return true;
}

bool GenGenerator::prepareImpl()
{
    return true;
}

SchemaPtr GenGenerator::createSchemaImpl(commsdsl::parse::ParseSchema dslObj, GenElem* parent)
{
    return std::make_unique<GenSchema>(*this, dslObj, parent);
}

NamespacePtr GenGenerator::createNamespaceImpl(commsdsl::parse::ParseNamespace dslObj, GenElem* parent)
{
    return std::make_unique<GenNamespace>(*this, dslObj, parent);
}

InterfacePtr GenGenerator::createInterfaceImpl(commsdsl::parse::ParseInterface dslObj, GenElem* parent)
{
    return std::make_unique<GenInterface>(*this, dslObj, parent);
}

MessagePtr GenGenerator::createMessageImpl(commsdsl::parse::ParseMessage dslObj, GenElem* parent)
{
    return std::make_unique<GenMessage>(*this, dslObj, parent);
}

FramePtr GenGenerator::createFrameImpl(commsdsl::parse::ParseFrame dslObj, GenElem* parent)
{
    return std::make_unique<GenFrame>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createIntFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenIntField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createEnumFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenEnumField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createSetFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenSetField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createFloatFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenFloatField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createBitfieldFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenBitfieldField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createBundleFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenBundleField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createStringFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenStringField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createDataFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenDataField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createListFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenListField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createRefFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenRefField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createOptionalFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenOptionalField>(*this, dslObj, parent);
}

FieldPtr GenGenerator::createVariantFieldImpl(commsdsl::parse::ParseField dslObj, GenElem* parent)
{
    return std::make_unique<GenVariantField>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createCustomLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenCustomLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createSyncLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenSyncLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createSizeLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenSizeLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createIdLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenIdLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createValueLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenValueLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createPayloadLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenPayloadLayer>(*this, dslObj, parent);
}

LayerPtr GenGenerator::createChecksumLayerImpl(commsdsl::parse::ParseLayer dslObj, GenElem* parent)
{
    return std::make_unique<GenChecksumLayer>(*this, dslObj, parent);
}

bool GenGenerator::writeImpl()
{
    return true;
}

GenGenerator::LoggerPtr GenGenerator::createLoggerImpl()
{
    return std::make_unique<GenLogger>();
}

GenNamespace* GenGenerator::addDefaultNamespace()
{
    return currentSchema().addDefaultNamespace();
}

bool GenGenerator::copyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const
{
    auto& inputDir = getCodeDir();
    if (inputDir.empty()) {
        return true;
    }

    auto& outputDir = getOutputDir();
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
        auto& protSchema = protocolSchema();
        auto schemaNs = util::strToName(protSchema.schemaName());
        do {
            if (protSchema.mainNamespace() == schemaNs) {
                break;
            }

            auto srcPrefix = (fs::path(strings::includeDirStr()) / schemaNs).string();
            if (!util::strStartsWith(relPath, srcPrefix)) {
                break;
            }

            auto dstPrefix = (fs::path(strings::includeDirStr()) / protSchema.mainNamespace()).string();
            relPath = dstPrefix + std::string(relPath, srcPrefix.size());
        } while (false);

        auto destPath = fs::path(outputDir) / relPath;
        logger().info("Copying " + destPath.string());

        if (!createDirectory(destPath.parent_path().string())) {
            return false;
        }

        std::error_code ec;
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            logger().error("Failed to copy with reason: " + ec.message());
            return false;
        }

        if (protSchema.mainNamespace() != schemaNs) {
            // The namespace has changed

            auto destStr = destPath.string();
            std::ifstream stream(destStr);
            if (!stream) {
                logger().error("Failed to open " + destStr + " for modification.");
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            stream.close();

            content = util::strReplace(content, "namespace " + schemaNs, "namespace " + protSchema.mainNamespace());
            std::ofstream outStream(destStr, std::ios_base::trunc);
            if (!outStream) {
                logger().error("Failed to modify " + destStr + ".");
                return false;
            }

            outStream << content;
            logger().info("Updated " + destStr + " to have proper main namespace.");
        }
    }
    return true;    
}

} // namespace gen

} // namespace commsdsl
