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

    [[nodiscard]] static const GenSchema& genCast(const GenElem& obj)
    {
        return static_cast<const GenSchema&>(obj);
    }

    [[nodiscard]] const ParseSchema& genParseObj() const;

    [[nodiscard]] const std::string& genSchemaName() const;
    [[nodiscard]] const std::string& genDisplayName() const;
    [[nodiscard]] parse::ParseEndian genSchemaEndian() const;
    [[nodiscard]] unsigned genSchemaVersion() const;

    [[nodiscard]] GenFieldsAccessList genGetAllMessageIdFields() const;
    [[nodiscard]] const GenField* genFindField(const std::string& externalRef) const;
    [[nodiscard]] GenField* genFindField(const std::string& externalRef);
    [[nodiscard]] const GenMessage* genFindMessage(const std::string& externalRef) const;
    [[nodiscard]] GenMessage* genFindMessage(const std::string& externalRef);
    [[nodiscard]] const GenFrame* genFindFrame(const std::string& externalRef) const;
    [[nodiscard]] const GenInterface* genFindInterface(const std::string& externalRef) const;

    [[nodiscard]] bool genAnyInterfaceHasVersion() const;
    [[nodiscard]] GenNamespacesAccessList genGetAllNamespaces() const;
    [[nodiscard]] GenInterfacesAccessList genGetAllInterfaces() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessages() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessagesIdSorted() const;
    [[nodiscard]] GenFramesAccessList genGetAllFrames() const;
    [[nodiscard]] GenFieldsAccessList genGetAllFields() const;

    [[nodiscard]] bool genCreateAll();
    [[nodiscard]] bool genPrepare();
    [[nodiscard]] bool genWrite();

    [[nodiscard]] GenNamespacesList& genNamespaces();
    [[nodiscard]] const GenNamespacesList& genNamespaces() const;
    [[nodiscard]] const GenPlatformNamesList& platformNames() const;

    [[nodiscard]] bool genVersionDependentCode() const;
    [[nodiscard]] const std::string& genMainNamespace() const;
    [[nodiscard]] const std::string& genOrigNamespace() const;

    [[nodiscard]] GenNamespace* genAddDefaultNamespace();
    void genForceSchemaVersion(unsigned value);
    void genSetVersionIndependentCodeForced(bool value);
    void genSetMainNamespaceOverride(const std::string& value);

    void genSetMinRemoteVersion(unsigned value);

    [[nodiscard]] bool genDoesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    [[nodiscard]] bool genIsElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    [[nodiscard]] bool genIsElementDeprecated(unsigned deprecatedSince) const;

    void genSetAllInterfacesReferenced();
    void genSetAllMessagesReferenced();

    [[nodiscard]] bool genHasReferencedMessageIdField() const;
    [[nodiscard]] bool genHasAnyReferencedMessage() const;
    [[nodiscard]] bool genHasAnyReferencedComponent() const;

    [[nodiscard]] GenGenerator& genGenerator();
    [[nodiscard]] const GenGenerator& genGenerator() const;

protected:
    [[nodiscard]] virtual GenType genElemTypeImpl() const override final;
    [[nodiscard]] virtual bool genPrepareImpl();
    [[nodiscard]] virtual bool genWriteImpl();

private:
    std::unique_ptr<GenSchemaImpl> m_impl;
};

using GenSchemaPtr = GenSchema::GenPtr;

} // namespace gen

} // namespace commsdsl
