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
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Protocol.h"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <system_error>

namespace commsdsl
{

namespace gen
{

namespace 
{

const unsigned MaxDslVersion = 5U;

} // namespace 


class GeneratorImpl
{
public:
    using LoggerPtr = Generator::LoggerPtr;
    using FilesList = Generator::FilesList;
    using NamespacesList = Generator::NamespacesList;
    using PlatformNamesList = Generator::PlatformNamesList;

    explicit GeneratorImpl(Generator& generator) :
        m_generator(generator),
        m_outputDir(std::filesystem::current_path().string())
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

    Schema& schema()
    {
        assert(m_schema);
        return *m_schema;
    }

    const Schema& schema() const
    {
        assert(m_schema);
        return *m_schema;
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

    void setMainNamespaceOverride(const std::string& value)
    {
        m_mainNamespace = value;
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

    void setVersionIndependentCodeForced(bool value)
    {
        m_versionIndependentCodeForced = value;
    }

    bool getVersionIndependentCodeForced() const
    {
        return m_versionIndependentCodeForced;
    }

    unsigned parsedSchemaVersion() const
    {
        return m_parsedSchemaVersion;
    }

    unsigned schemaVersion() const
    {
        if (0 <= m_forcedSchemaVersion) {
            return static_cast<unsigned>(m_forcedSchemaVersion);
        }

        return parsedSchemaVersion();
    }

    const std::string& mainNamespace() const
    {
        return m_mainNamespace;
    }

    const std::string& schemaName() const
    {
        return m_protocol.schema().name();
    }

    parse::Endian schemaEndian() const
    {
        return m_protocol.schema().endian();
    }

    const PlatformNamesList& platformNames() const
    {
        return m_protocol.platforms();
    }

    const Field* getMessageIdField() const
    {
        return m_messageIdField;
    }

    const Field* findField(const std::string& externalRef) const
    {
        assert(m_schema);
        return m_schema->findField(externalRef);
    }

    Field* findField(const std::string& externalRef)
    {
        assert(m_schema);
        return m_schema->findField(externalRef);        
    }

    const Message* findMessage(const std::string& externalRef) const
    {
        assert(m_schema);
        return m_schema->findMessage(externalRef);
    }  

    Message* findMessage(const std::string& externalRef)
    {
        assert(m_schema);
        return m_schema->findMessage(externalRef);
    }

    const Frame* findFrame(const std::string& externalRef) const
    {
        assert(m_schema);
        return m_schema->findFrame(externalRef);
    }

    const Interface* findInterface(const std::string& externalRef) const
    {
        assert(m_schema);
        return m_schema->findInterface(externalRef);
    }                

    bool prepare(const FilesList& files)
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

        auto schemaDsl = m_protocol.schema();
        m_schema = m_generator.createSchema(schemaDsl);
        m_schemaNamespace = util::strToName(schemaDsl.name());
        if (m_mainNamespace.empty()) {
            assert(!schemaDsl.name().empty());
            m_mainNamespace = m_schemaNamespace;
        }

        m_schemaEndian = schemaDsl.endian();
        m_parsedSchemaVersion = schemaDsl.version();
        if ((0 <= m_forcedSchemaVersion) && 
            (m_parsedSchemaVersion < static_cast<decltype(m_parsedSchemaVersion)>(m_forcedSchemaVersion))) {
            m_logger->error("Cannot force version to be greater than " + util::numToString(m_parsedSchemaVersion));
            return false;
        }

        auto dslVersion = schemaDsl.dslVersion();
        if (MaxDslVersion < dslVersion) {
            m_logger->error(
                "Required DSL version is too big (" + std::to_string(dslVersion) +
                "), upgrade your code generator.");
            return false;
        }

        if (!m_schema->createAll()) {
            m_logger->error("Failed to create elements inside schema \"" + schemaDsl.name() + "\"");
            return false;
        }


        if (!m_versionIndependentCodeForced) {
            m_versionDependentCode = anyInterfaceHasVersion();
        }

        if (!m_schema->prepare()) {
            m_logger->error("Failed to prepare elements inside schema \"" + schemaDsl.name() + "\"");
            return false;
        }

        m_messageIdField = findMessageIdField();

        if (m_logger->hadWarning()) {
            m_logger->error("Warning treated as error");
            return false;
        }        

        return true;
    }

    bool write()
    {
        assert(m_schema);
        return m_schema->write();
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

    bool versionDependentCode() const
    {
        return m_versionDependentCode;
    }

private:
    const Field* findMessageIdField() const
    {
        assert(m_schema);
        return m_schema->findMessageIdField();
    }

    bool anyInterfaceHasVersion() const
    {
        assert(m_schema);
        return m_schema->anyInterfaceHasVersion();
    }

    Generator& m_generator;
    commsdsl::parse::Protocol m_protocol;
    LoggerPtr m_logger;
    SchemaPtr m_schema;
    std::string m_schemaNamespace;
    std::string m_mainNamespace;
    std::string m_topNamespace;
    commsdsl::parse::Endian m_schemaEndian = commsdsl::parse::Endian_Little;
    unsigned m_parsedSchemaVersion = 0U;
    int m_forcedSchemaVersion = -1;
    unsigned m_minRemoteVersion = 0U;
    std::string m_outputDir;
    std::string m_codeDir;
    const Field* m_messageIdField = nullptr;
    mutable std::vector<std::string> m_createdDirectories;
    bool m_versionIndependentCodeForced = false;
    bool m_versionDependentCode = false;
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

void Generator::setMainNamespaceOverride(const std::string& value)
{
    m_impl->setMainNamespaceOverride(value);
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

void Generator::setVersionIndependentCodeForced(bool value)
{
    m_impl->setVersionIndependentCodeForced(value);
}

bool Generator::getVersionIndependentCodeForced() const
{
    return m_impl->getVersionIndependentCodeForced();
}

unsigned Generator::parsedSchemaVersion() const
{
    return m_impl->parsedSchemaVersion();
}

unsigned Generator::schemaVersion() const
{
    return m_impl->schemaVersion();
}

const std::string& Generator::mainNamespace() const
{
    return m_impl->mainNamespace();
}

const std::string& Generator::schemaName() const
{
    return m_impl->schemaName();
}

parse::Endian Generator::schemaEndian() const
{
    return m_impl->schemaEndian();
}

const Generator::PlatformNamesList& Generator::platformNames() const
{
    return m_impl->platformNames();
}

const Field* Generator::getMessageIdField() const
{
    return m_impl->getMessageIdField();
}

const Field* Generator::findField(const std::string& externalRef) const
{
    auto* field = m_impl->findField(externalRef);
    assert(field->isPrepared());
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

Generator::NamespacesAccessList Generator::getAllNamespaces() const
{
    return schema().getAllNamespaces();
}

Generator::InterfacesAccessList Generator::getAllInterfaces() const
{
    return schema().getAllInterfaces();
}

Generator::MessagesAccessList Generator::getAllMessages() const
{
    return schema().getAllMessages();
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
    return schema().getAllFrames();
}

bool Generator::prepare(const FilesList& files)
{
    // Make sure the logger is created
    auto& l = logger();
    static_cast<void>(l);

    if (!m_impl->prepare(files)) {
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
    unsigned sVersion = schemaVersion();

    if (sVersion < sinceVersion) {
        return false;
    }

    if (deprecatedRemoved && (deprecatedSince <= getMinRemoteVersion())) {
        return false;
    }

    return true;
}

bool Generator::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    if (getMinRemoteVersion() < sinceVersion) {
        return true;
    }

    if (deprecatedRemoved && (deprecatedSince < commsdsl::parse::Protocol::notYetDeprecated())) {
        return true;
    }

    return false;
}

bool Generator::isElementDeprecated(unsigned deprecatedSince) const
{
    return deprecatedSince < schemaVersion();
} 

bool Generator::versionDependentCode() const
{
    return m_impl->versionDependentCode();
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

Schema& Generator::schema()
{
    return m_impl->schema();
}

const Schema& Generator::schema() const
{
    return m_impl->schema();
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

bool Generator::createDirectory(const std::string& path) const
{
    if (m_impl->wasDirectoryCreated(path)) {
        return true;
    }

    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        m_impl->recordCreatedDirectory(path);
        return true;
    }

    if (!std::filesystem::create_directories(path, ec)) {
        logger().error("Failed to create directory \"" + path + "\" with error: " + ec.message());
        return false;
    }

    m_impl->recordCreatedDirectory(path);
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
    return schema().addDefaultNamespace();
}

} // namespace gen

} // namespace commsdsl
