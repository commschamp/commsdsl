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
    using ParseSchema = commsdsl::parse::ParseSchema;

    using GenPtr = std::unique_ptr<GenSchema>;
    using GenNamespacesList = GenNamespace::GenNamespacesList;
    using GenPlatformNamesList = std::vector<std::string>;
    using GenNamespacesAccessList = GenNamespace::GenNamespacesAccessList;
    using GenInterfacesAccessList = GenNamespace::GenInterfacesAccessList;
    using GenMessagesAccessList = GenNamespace::GenMessagesAccessList;
    using GenFramesAccessList = GenNamespace::GenFramesAccessList;
    using GenFieldsAccessList = GenNamespace::GenFieldsAccessList;

    GenSchema(GenGenerator& generator, ParseSchema parseObj, GenElem* parent = nullptr);
    virtual ~GenSchema();

    static const GenSchema& genCast(const GenElem& obj)
    {
        return static_cast<const GenSchema&>(obj);
    }

    const ParseSchema& genParseObj() const;

    const std::string& genSchemaName() const;
    const std::string& genDisplayName() const;
    parse::ParseEndian genSchemaEndian() const;
    unsigned genSchemaVersion() const;

    GenFieldsAccessList genGetAllMessageIdFields() const;
    const GenField* genFindField(const std::string& externalRef) const;
    GenField* genFindField(const std::string& externalRef);
    const GenMessage* genGindMessage(const std::string& externalRef) const;
    GenMessage* genGindMessage(const std::string& externalRef);
    const GenFrame* genFindFrame(const std::string& externalRef) const;
    const GenInterface* genFindInterface(const std::string& externalRef) const;

    bool genAnyInterfaceHasVersion() const;
    GenNamespacesAccessList genGetAllNamespaces() const;
    GenInterfacesAccessList genGetAllInterfaces() const;
    GenMessagesAccessList genGetAllMessages() const;
    GenMessagesAccessList genGetAllMessagesIdSorted() const;
    GenFramesAccessList genGetAllFrames() const;
    GenFieldsAccessList genGetAllFields() const;

    bool genCreateAll();
    bool genPrepare();
    bool genWrite();

    GenNamespacesList& genNamespaces();
    const GenNamespacesList& genNamespaces() const;
    const GenPlatformNamesList& platformNames() const;

    bool genVersionDependentCode() const;
    const std::string& genMainNamespace() const;
    const std::string& genOrigNamespace() const;

    GenNamespace* genAddDefaultNamespace();
    void genForceSchemaVersion(unsigned value);
    void genSetVersionIndependentCodeForced(bool value);
    void genSetMainNamespaceOverride(const std::string& value);

    void genSetMinRemoteVersion(unsigned value);

    bool genDoesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool genIsElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool genIsElementDeprecated(unsigned deprecatedSince) const;

    void genSetAllInterfacesReferenced();
    void genSetAllMessagesReferenced();

    bool genHasReferencedMessageIdField() const;
    bool genHasAnyReferencedMessage() const;
    bool genHasAnyReferencedComponent() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

protected:
    virtual GenType genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl();

private:
    std::unique_ptr<GenSchemaImpl> m_impl;
};

using GenSchemaPtr = GenSchema::GenPtr;

} // namespace gen

} // namespace commsdsl
