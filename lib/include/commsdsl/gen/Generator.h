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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/Logger.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"
#include "commsdsl/parse/Endian.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GeneratorImpl;
class Generator
{
public:
    using FilesList = std::vector<std::string>;
    using LoggerPtr = std::unique_ptr<Logger>;
    using NamespacesList = Namespace::NamespacesList;

    using InterfacesAccessList = Namespace::InterfacesAccessList;
    using FramesAccessList = Namespace::FramesAccessList;

    Generator();
    virtual ~Generator();

    void forceSchemaVersion(unsigned value);
    void setMinRemoteVersion(unsigned value);
    unsigned getMinRemoteVersion() const;
    void setMainNamespaceOverride(const std::string& value);    

    void setOutputDir(const std::string& outDir);
    const std::string& getOutputDir() const;

    void setCodeDir(const std::string& dir);
    const std::string& getCodeDir() const;   

    void setVersionIndependentCodeForced(bool value = true); 
    bool getVersionIndependentCodeForced() const;

    unsigned parsedSchemaVersion() const;
    unsigned schemaVersion() const;
    const std::string& mainNamespace() const;
    const std::string& schemaName() const;
    parse::Endian schemaEndian() const;

    const Field* getMessageIdField() const;
    InterfacesAccessList getAllInterfaces() const;
    FramesAccessList getAllFrames() const;

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

    bool versionDependentCode() const;

    Logger& logger();

    NamespacesList& namespaces();

    NamespacePtr createNamespace(commsdsl::parse::Namespace dslObj, Elem* parent = nullptr);
    InterfacePtr createInterface(commsdsl::parse::Interface dslObj, Elem* parent);
    MessagePtr createMessage(commsdsl::parse::Message dslObj, Elem* parent);
    FramePtr createFrame(commsdsl::parse::Frame dslObj, Elem* parent);

    FieldPtr createIntField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createEnumField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createSetField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createFloatField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createBitfieldField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createBundleField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createStringField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createDataField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createListField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createRefField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createOptionalField(commsdsl::parse::Field dslObj, Elem* parent);
    FieldPtr createVariantField(commsdsl::parse::Field dslObj, Elem* parent);

    LayerPtr createCustomLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createSyncLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createSizeLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createIdLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createValueLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createPayloadLayer(commsdsl::parse::Layer dslObj, Elem* parent);
    LayerPtr createChecksumLayer(commsdsl::parse::Layer dslObj, Elem* parent);

    bool createDirectory(const std::string& path);

protected:
    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent);
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent);
    virtual MessagePtr createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent);
    virtual FramePtr createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent);

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent);

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent);

    virtual bool writeImpl();
    virtual LoggerPtr createLoggerImpl();

private:
    std::unique_ptr<GeneratorImpl> m_impl;    
};

} // namespace gen

} // namespace commsdsl
