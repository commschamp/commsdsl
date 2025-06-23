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

#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/parse/ParseSchema.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GenSchemaImpl;
class GenSchema : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenSchema>;
    using NamespacesList = GenNamespace::NamespacesList;
    using PlatformNamesList = std::vector<std::string>;

    using NamespacesAccessList = GenNamespace::NamespacesAccessList;
    using InterfacesAccessList = GenNamespace::InterfacesAccessList;
    using MessagesAccessList = GenNamespace::MessagesAccessList;
    using FramesAccessList = GenNamespace::FramesAccessList;
    using FieldsAccessList = GenNamespace::FieldsAccessList;

    GenSchema(GenGenerator& generator, commsdsl::parse::ParseSchema dslObj, GenElem* parent = nullptr);
    virtual ~GenSchema();

    const commsdsl::parse::ParseSchema& dslObj() const;

    const std::string& schemaName() const;
    parse::ParseEndian schemaEndian() const;
    unsigned schemaVersion() const;

    FieldsAccessList getAllMessageIdFields() const;
    const GenField* findField(const std::string& externalRef) const;
    GenField* findField(const std::string& externalRef);
    const GenMessage* findMessage(const std::string& externalRef) const;
    GenMessage* findMessage(const std::string& externalRef);
    const GenFrame* findFrame(const std::string& externalRef) const;
    const GenInterface* findInterface(const std::string& externalRef) const;

    bool anyInterfaceHasVersion() const;
    NamespacesAccessList getAllNamespaces() const;
    InterfacesAccessList getAllInterfaces() const;
    MessagesAccessList getAllMessages() const;
    MessagesAccessList getAllMessagesIdSorted() const;
    FramesAccessList getAllFrames() const;
    FieldsAccessList getAllFields() const;

    bool createAll();
    bool prepare();
    bool write();

    NamespacesList& namespaces();
    const NamespacesList& namespaces() const;
    const PlatformNamesList& platformNames() const;

    bool versionDependentCode() const;
    const std::string& mainNamespace() const;
    const std::string& origNamespace() const;

    GenNamespace* addDefaultNamespace();
    void forceSchemaVersion(unsigned value);
    void setVersionIndependentCodeForced(bool value);
    void setMainNamespaceOverride(const std::string& value);

    void setMinRemoteVersion(unsigned value);

    bool doesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool isElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;  

    bool isElementDeprecated(unsigned deprecatedSince) const;

    void setAllInterfacesReferenced();
    void setAllMessagesReferenced();

    bool hasReferencedMessageIdField() const;
    bool hasAnyReferencedMessage() const;    
    bool hasAnyReferencedComponent() const;    

protected:
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl();


private:
    std::unique_ptr<GenSchemaImpl> m_impl;    
};

using SchemaPtr = GenSchema::Ptr;

} // namespace gen

} // namespace commsdsl
