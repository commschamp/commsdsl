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

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"
#include "commsdsl/parse/Schema.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class SchemaImpl;
class Schema : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Schema>;
    using NamespacesList = Namespace::NamespacesList;
    using PlatformNamesList = std::vector<std::string>;

    using NamespacesAccessList = Namespace::NamespacesAccessList;
    using InterfacesAccessList = Namespace::InterfacesAccessList;
    using MessagesAccessList = Namespace::MessagesAccessList;
    using FramesAccessList = Namespace::FramesAccessList;

    Schema(Generator& generator, commsdsl::parse::Schema dslObj, Elem* parent = nullptr);
    virtual ~Schema();

    const commsdsl::parse::Schema& dslObj() const;

    const std::string& schemaName() const;
    parse::Endian schemaEndian() const;
    unsigned schemaVersion() const;

    const Field* getMessageIdField() const;
    const Field* findField(const std::string& externalRef) const;
    Field* findField(const std::string& externalRef);
    const Message* findMessage(const std::string& externalRef) const;
    Message* findMessage(const std::string& externalRef);
    const Frame* findFrame(const std::string& externalRef) const;
    const Interface* findInterface(const std::string& externalRef) const;

    const Field* findMessageIdField() const;
    bool anyInterfaceHasVersion() const;
    NamespacesAccessList getAllNamespaces() const;
    InterfacesAccessList getAllInterfaces() const;
    MessagesAccessList getAllMessages() const;
    MessagesAccessList getAllMessagesIdSorted() const;
    FramesAccessList getAllFrames() const;

    bool createAll();
    bool prepare();
    bool write();

    NamespacesList& namespaces();
    const NamespacesList& namespaces() const;
    const PlatformNamesList& platformNames() const;

    bool versionDependentCode() const;
    const std::string& mainNamespace() const;

    Namespace* addDefaultNamespace();
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

protected:
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl();


private:
    std::unique_ptr<SchemaImpl> m_impl;    
};

using SchemaPtr = Schema::Ptr;

} // namespace gen

} // namespace commsdsl
