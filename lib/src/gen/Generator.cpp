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

namespace commsdsl
{

namespace gen
{

namespace 
{

const unsigned MaxDslVersion = 3U;

} // namespace 


class GeneratorImpl
{
public:
    using LoggerPtr = Generator::LoggerPtr;
    using FilesList = Generator::FilesList;
    using NamespacesList = Generator::NamespacesList;

    LoggerPtr& getLogger()
    {
        return m_logger;
    }

    void setLogger(LoggerPtr logger)
    {
        m_logger = std::move(logger);
    }

    NamespacesList& namespaces()
    {
        return m_namespaces;
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

        auto schema = m_protocol.schema();
        m_schemaNamespace = util::strToName(schema.name());
        if (m_mainNamespace.empty()) {
            assert(!schema.name().empty());
            m_mainNamespace = m_schemaNamespace;
        }

        m_schemaEndian = schema.endian();
        m_parsedSchemaVersion = schema.version();
        if ((0 <= m_forcedSchemaVersion) && 
            (m_parsedSchemaVersion < static_cast<decltype(m_parsedSchemaVersion)>(m_forcedSchemaVersion))) {
            m_logger->error("Cannot force version to be greater than " + util::numToString(m_parsedSchemaVersion));
            return false;
        }

        auto dslVersion = schema.dslVersion();
        if (MaxDslVersion < dslVersion) {
            m_logger->error(
                "Required DSL version is too big (" + std::to_string(dslVersion) +
                "), upgrade your code generator.");
            return false;
        }

        return true;
    }

    bool write()
    {
        return 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->write();
                });
    }

private:
    commsdsl::parse::Protocol m_protocol;
    LoggerPtr m_logger;
    NamespacesList m_namespaces;
    std::string m_schemaNamespace;
    std::string m_mainNamespace;
    commsdsl::parse::Endian m_schemaEndian = commsdsl::parse::Endian_Little;
    unsigned m_parsedSchemaVersion = 0U;
    int m_forcedSchemaVersion = -1;
    unsigned m_minRemoteVersion = 0U;
}; 

Generator::Generator() : 
    m_impl(std::make_unique<GeneratorImpl>())
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

unsigned Generator::parsedSchemaVersion() const
{
    return m_impl->parsedSchemaVersion();
}

unsigned Generator::schemaVersion() const
{
    return m_impl->schemaVersion();
}

bool Generator::prepare(const FilesList& files)
{
    // Make sure the logger is created
    auto& l = logger();
    static_cast<void>(l);

    return m_impl->prepare(files);
}

bool Generator::write()
{
    if (!m_impl->write()) {
        return false;
    }
    
    return writeImpl();
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

Generator::NamespacesList& Generator::namespaces()
{
    return m_impl->namespaces();
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
    return createStringFieldImpl(dslObj, parent);
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

} // namespace gen

} // namespace commsdsl
