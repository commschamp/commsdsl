//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Generator.h"

#include "commsdsl/gen/BitfieldField.h"
#include "commsdsl/gen/BundleField.h"
#include "commsdsl/gen/ChecksumLayer.h"
#include "commsdsl/gen/CustomLayer.h"
#include "commsdsl/gen/DataField.h"
#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/FloatField.h"
#include "commsdsl/gen/IdLayer.h"
#include "commsdsl/gen/IntField.h"
#include "commsdsl/gen/ListField.h"
#include "commsdsl/gen/OptionalField.h"
#include "commsdsl/gen/PayloadLayer.h"
#include "commsdsl/gen/RefField.h"
#include "commsdsl/gen/Schema.h"
#include "commsdsl/gen/SetField.h"
#include "commsdsl/gen/SizeLayer.h"
#include "commsdsl/gen/StringField.h"
#include "commsdsl/gen/SyncLayer.h"
#include "commsdsl/gen/ValueLayer.h"
#include "commsdsl/gen/VariantField.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Protocol.h"

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


class GeneratorImpl
{
public:
    using LoggerPtr = Generator::LoggerPtr;
    using FilesList = Generator::FilesList;
    using NamespacesList = Generator::NamespacesList;
    using PlatformNamesList = Generator::PlatformNamesList;
    using SchemasList = Generator::SchemasList;

    explicit GeneratorImpl(Generator& generator) :
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

    Schema& currentSchema()
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    const Schema& currentSchema() const
    {
        assert(m_currentSchema != nullptr);
        return *m_currentSchema;
    }

    Schema& protocolSchema()
    {
        assert(!m_schemas.empty());
        return *m_schemas.back();
    }

    const Schema& protocolSchema() const
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

    const Field* findField(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findField(parsedRef.second);
    }

    Field* findField(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<Schema*>(parsedRef.first)->findField(parsedRef.second);
    }

    const Message* findMessage(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findMessage(parsedRef.second);
    }  

    Message* findMessage(const std::string& externalRef)
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return const_cast<Schema*>(parsedRef.first)->findMessage(parsedRef.second);
    }

    const Frame* findFrame(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto parsedRef = parseExternalRef(externalRef);
        if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
            return nullptr;
        }

        return parsedRef.first->findFrame(parsedRef.second);
    }

    const Interface* findInterface(const std::string& externalRef) const
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
            [this](commsdsl::parse::ErrorLevel level, const std::string& msg)
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

    const commsdsl::parse::Protocol& protocol() const
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
    std::pair<const Schema*, std::string> parseExternalRef(const std::string& externalRef) const
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

    Generator& m_generator;
    commsdsl::parse::Protocol m_protocol;
    LoggerPtr m_logger;
    SchemasList m_schemas;
    Schema* m_currentSchema = nullptr;
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

Generator::Generator() : 
    m_impl(std::make_unique<GeneratorImpl>(*this))
{
}

Generator::~Generator() = default;

void Generator::forceSchemaVersion(unsigned value)
{
    m_impl->forceSchemaVersion(value);
}

void Generator::setMinRemoteVersion(unsigned value)
{
    m_impl->setMinRemoteVersion(value);
}

unsigned Generator::getMinRemoteVersion() const
{
    return m_impl->getMinRemoteVersion();
}

void Generator::setNamespaceOverride(const std::string& value)
{
    m_impl->setNamespaceOverride(value);
}

void Generator::setTopNamespace(const std::string& value)
{
    m_impl->setTopNamespace(value);
}

const std::string& Generator::getTopNamespace() const
{
    return m_impl->getTopNamespace();
}

void Generator::setOutputDir(const std::string& outDir)
{
    m_impl->setOutputDir(outDir);
}

const std::string& Generator::getOutputDir() const
{
    return m_impl->getOutputDir();
}

void Generator::setCodeDir(const std::string& dir)
{
    m_impl->setCodeDir(dir);
}

const std::string& Generator::getCodeDir() const
{
    return m_impl->getCodeDir();
}

void Generator::setMultipleSchemasEnabled(bool enabled)
{
    m_impl->setMultipleSchemasEnabled(enabled);
}

bool Generator::getMultipleSchemasEnabled() const
{
    return m_impl->getMultipleSchemasEnabled();
}

void Generator::setVersionIndependentCodeForced(bool value)
{
    m_impl->setVersionIndependentCodeForced(value);
}

bool Generator::getVersionIndependentCodeForced() const
{
    return m_impl->getVersionIndependentCodeForced();
}

const Field* Generator::findField(const std::string& externalRef) const
{
    auto* field = m_impl->findField(externalRef);
    assert((field == nullptr) || (field->isPrepared()));
    return field;
}

Field* Generator::findField(const std::string& externalRef)
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

const Message* Generator::findMessage(const std::string& externalRef) const
{
    return m_impl->findMessage(externalRef);
}

Message* Generator::findMessage(const std::string& externalRef) 
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

const Frame* Generator::findFrame(const std::string& externalRef) const
{
    return m_impl->findFrame(externalRef);
}

const Interface* Generator::findInterface(const std::string& externalRef) const
{
    return m_impl->findInterface(externalRef);
}

const Schema& Generator::schemaOf(const Elem& elem)
{
    auto* parent = elem.getParent();
    assert(parent != nullptr);
    if (parent->elemType() == Elem::Type_Schema) {
        return static_cast<const Schema&>(*parent);
    }

    return schemaOf(*parent);
}

Generator::NamespacesAccessList Generator::getAllNamespaces() const
{
    return currentSchema().getAllNamespaces();
}

Generator::InterfacesAccessList Generator::getAllInterfaces() const
{
    return currentSchema().getAllInterfaces();
}

Generator::MessagesAccessList Generator::getAllMessages() const
{
    return currentSchema().getAllMessages();
}

Generator::MessagesAccessList Generator::getAllMessagesIdSorted() const
{
    auto result = getAllMessages();
    std::sort(
        result.begin(), result.end(),
        [](auto* msg1, auto* msg2)
        {
            auto id1 = msg1->dslObj().id();
            auto id2 = msg2->dslObj().id();

            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1->dslObj().order() < msg2->dslObj().order();
        });
    return result;
}

Generator::FramesAccessList Generator::getAllFrames() const
{
    return currentSchema().getAllFrames();
}

Generator::FieldsAccessList Generator::getAllFields() const
{
    return currentSchema().getAllFields();
}

Generator::NamespacesAccessList Generator::getAllNamespacesFromAllSchemas() const
{
    NamespacesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllNamespaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

Generator::InterfacesAccessList Generator::getAllInterfacesFromAllSchemas() const
{
    InterfacesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllInterfaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

Generator::MessagesAccessList Generator::getAllMessagesFromAllSchemas() const
{
    MessagesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

Generator::MessagesAccessList Generator::getAllMessagesIdSortedFromAllSchemas() const
{
    auto result = getAllMessagesFromAllSchemas();
    std::sort(
        result.begin(), result.end(),
        [](auto* msg1, auto* msg2)
        {
            auto id1 = msg1->dslObj().id();
            auto id2 = msg2->dslObj().id();

            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1->dslObj().order() < msg2->dslObj().order();
        });
    return result;    
}

Generator::FramesAccessList Generator::getAllFramesFromAllSchemas() const
{
    FramesAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllFrames();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

Generator::FieldsAccessList Generator::getAllFieldsFromAllSchemas() const
{
    FieldsAccessList result;
    for (auto& sPtr : schemas()) {
        auto list = sPtr->getAllFields();
        result.insert(result.end(), list.begin(), list.end());
    }

    return result;
}

bool Generator::prepare(const FilesList& files)
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

bool Generator::write()
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

bool Generator::doesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return currentSchema().doesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool Generator::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return currentSchema().isElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool Generator::isElementDeprecated(unsigned deprecatedSince) const
{
    return currentSchema().isElementDeprecated(deprecatedSince);
} 

Logger& Generator::logger()
{
    auto& loggerPtr = m_impl->getLogger();
    if (loggerPtr) {
        return *loggerPtr;
    }

    auto newLogger = createLoggerImpl();
    if (!newLogger) {
        newLogger = Generator::createLoggerImpl();
        assert(newLogger);
    }
    
    auto& logger = *newLogger;
    m_impl->setLogger(std::move(newLogger));
    return logger;    
}

const Logger& Generator::logger() const
{
    auto& loggerPtr = m_impl->getLogger();
    assert(loggerPtr);
    return *loggerPtr;
}

const Generator::SchemasList& Generator::schemas() const
{
    return m_impl->schemas();
}

Schema& Generator::currentSchema()
{
    return m_impl->currentSchema();
}

const Schema& Generator::currentSchema() const
{
    return m_impl->currentSchema();
}

Schema& Generator::protocolSchema()
{
    return m_impl->protocolSchema();
}

const Schema& Generator::protocolSchema() const
{
    return m_impl->protocolSchema();
}

bool Generator::isCurrentProtocolSchema() const
{
    return &currentSchema() == &protocolSchema();
}

SchemaPtr Generator::createSchema(commsdsl::parse::Schema dslObj, Elem* parent)
{
    return createSchemaImpl(dslObj, parent);
}

NamespacePtr Generator::createNamespace(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return createNamespaceImpl(dslObj, parent);
}

InterfacePtr Generator::createInterface(commsdsl::parse::Interface dslObj, Elem* parent)
{
    return createInterfaceImpl(dslObj, parent);
}

MessagePtr Generator::createMessage(commsdsl::parse::Message dslObj, Elem* parent)
{
    return createMessageImpl(dslObj, parent);
}

FramePtr Generator::createFrame(commsdsl::parse::Frame dslObj, Elem* parent)
{
    return createFrameImpl(dslObj, parent);
}

FieldPtr Generator::createIntField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Int);
    return createIntFieldImpl(dslObj, parent);
}

FieldPtr Generator::createEnumField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Enum);
    return createEnumFieldImpl(dslObj, parent);
}

FieldPtr Generator::createSetField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Set);
    return createSetFieldImpl(dslObj, parent);
}

FieldPtr Generator::createFloatField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Float);
    return createFloatFieldImpl(dslObj, parent);
}

FieldPtr Generator::createBitfieldField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Bitfield);
    return createBitfieldFieldImpl(dslObj, parent);
}

FieldPtr Generator::createBundleField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Bundle);
    return createBundleFieldImpl(dslObj, parent);
}

FieldPtr Generator::createStringField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::String);
    return createStringFieldImpl(dslObj, parent);
}

FieldPtr Generator::createDataField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Data);
    return createDataFieldImpl(dslObj, parent);
}

FieldPtr Generator::createListField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::List);
    return createListFieldImpl(dslObj, parent);
}

FieldPtr Generator::createRefField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Ref);
    return createRefFieldImpl(dslObj, parent);
}

FieldPtr Generator::createOptionalField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Optional);
    return createOptionalFieldImpl(dslObj, parent);
}

FieldPtr Generator::createVariantField(commsdsl::parse::Field dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Variant);
    return createVariantFieldImpl(dslObj, parent);
}

LayerPtr Generator::createCustomLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Custom);
    return createCustomLayerImpl(dslObj, parent);
}

LayerPtr Generator::createSyncLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Sync);
    return createSyncLayerImpl(dslObj, parent);
}

LayerPtr Generator::createSizeLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Size);
    return createSizeLayerImpl(dslObj, parent);
}

LayerPtr Generator::createIdLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Id);
    return createIdLayerImpl(dslObj, parent);
}

LayerPtr Generator::createValueLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Value);
    return createValueLayerImpl(dslObj, parent);
}

LayerPtr Generator::createPayloadLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Payload);
    return createPayloadLayerImpl(dslObj, parent);
}

LayerPtr Generator::createChecksumLayer(commsdsl::parse::Layer dslObj, Elem* parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Checksum);
    return createChecksumLayerImpl(dslObj, parent);
}

unsigned Generator::currentSchemaIdx() const
{
    return m_impl->currentSchemaIdx();
}

void Generator::chooseCurrentSchema(unsigned idx)
{
    m_impl->chooseCurrentSchema(idx);
}

void Generator::chooseCurrentSchema(unsigned idx) const
{
    const_cast<Generator*>(this)->chooseCurrentSchema(idx);
}

void Generator::chooseProtocolSchema()
{
    assert(!schemas().empty());
    chooseCurrentSchema(static_cast<unsigned>(schemas().size() - 1U));
}

void Generator::chooseProtocolSchema() const
{
    const_cast<Generator*>(this)->chooseProtocolSchema();
}

bool Generator::createDirectory(const std::string& path) const
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

void Generator::referenceAllMessages()
{
    m_impl->referenceAllMessages();
}

bool Generator::getAllMessagesReferencedByDefault() const
{
    return m_impl->getAllMessagesReferencedByDefault();
}

void Generator::setAllMessagesReferencedByDefault(bool value)
{
    m_impl->setAllMessagesReferencedByDefault(value);
}

void Generator::referenceAllInterfaces()
{
    m_impl->referenceAllInterfaces();
}

bool Generator::getAllInterfacesReferencedByDefault() const
{
    return m_impl->getAllInterfacesReferencedByDefault();
}

void Generator::setAllInterfacesReferencedByDefault(bool value)
{
    m_impl->setAllInterfacesReferencedByDefault(value);
}    

bool Generator::createCompleteImpl()
{
    return true;
}

bool Generator::prepareImpl()
{
    return true;
}

SchemaPtr Generator::createSchemaImpl(commsdsl::parse::Schema dslObj, Elem* parent)
{
    return std::make_unique<Schema>(*this, dslObj, parent);
}

NamespacePtr Generator::createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return std::make_unique<Namespace>(*this, dslObj, parent);
}

InterfacePtr Generator::createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent)
{
    return std::make_unique<Interface>(*this, dslObj, parent);
}

MessagePtr Generator::createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent)
{
    return std::make_unique<Message>(*this, dslObj, parent);
}

FramePtr Generator::createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent)
{
    return std::make_unique<Frame>(*this, dslObj, parent);
}

FieldPtr Generator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<IntField>(*this, dslObj, parent);
}

FieldPtr Generator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EnumField>(*this, dslObj, parent);
}

FieldPtr Generator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<SetField>(*this, dslObj, parent);
}

FieldPtr Generator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<FloatField>(*this, dslObj, parent);
}

FieldPtr Generator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<BitfieldField>(*this, dslObj, parent);
}

FieldPtr Generator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<BundleField>(*this, dslObj, parent);
}

FieldPtr Generator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<StringField>(*this, dslObj, parent);
}

FieldPtr Generator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<DataField>(*this, dslObj, parent);
}

FieldPtr Generator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<ListField>(*this, dslObj, parent);
}

FieldPtr Generator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<RefField>(*this, dslObj, parent);
}

FieldPtr Generator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<OptionalField>(*this, dslObj, parent);
}

FieldPtr Generator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<VariantField>(*this, dslObj, parent);
}

LayerPtr Generator::createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<CustomLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<SyncLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<SizeLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<IdLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<ValueLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<PayloadLayer>(*this, dslObj, parent);
}

LayerPtr Generator::createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<ChecksumLayer>(*this, dslObj, parent);
}

bool Generator::writeImpl()
{
    return true;
}

Generator::LoggerPtr Generator::createLoggerImpl()
{
    return std::make_unique<Logger>();
}

Namespace* Generator::addDefaultNamespace()
{
    return currentSchema().addDefaultNamespace();
}

bool Generator::copyExtraSourceFiles(const std::vector<std::string>& reservedExtensions) const
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

            util::strReplace(content, "namespace " + schemaNs, "namespace " + protSchema.mainNamespace());
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
