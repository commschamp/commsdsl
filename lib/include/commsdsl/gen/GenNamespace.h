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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/parse/ParseNamespace.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenNamespaceImpl;
class COMMSDSL_API GenNamespace : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenNamespace>;
    using NamespacesList = std::vector<Ptr>;
    using FieldsList = GenField::FieldsList;
    using InterfacesList = std::vector<GenInterfacePtr>;
    using MessagesList = std::vector<GenMessagePtr>;
    using FramesList = std::vector<FramePtr>;
    using NamespacesAccessList = std::vector<const GenNamespace*>;
    using InterfacesAccessList = std::vector<const GenInterface*>;
    using MessagesAccessList = std::vector<const GenMessage*>;
    using FramesAccessList = std::vector<const GenFrame*>;
    using FieldsAccessList = std::vector<const GenField*>;
    using ParseNamespace = commsdsl::parse::ParseNamespace;

    explicit GenNamespace(GenGenerator& generator, ParseNamespace parseObj, GenElem* parent = nullptr);
    virtual ~GenNamespace();

    bool genCreateAll();
    bool genPrepare();
    bool genWrite() const;

    ParseNamespace genParseObj() const;
    std::string genAdjustedExternalRef() const;

    const NamespacesList& genNamespaces() const;
    const FieldsList& genFields() const;
    const InterfacesList& genInterfaces() const;
    const MessagesList& genMessages() const;
    const FramesList& genFrames() const;
    bool genHasFramesRecursive() const;
    bool genHasMessagesRecursive() const;

    FieldsAccessList genFindMessageIdFields() const;
    const GenField* genFindField(const std::string& externalRef) const;
    const GenMessage* genGindMessage(const std::string& externalRef) const;
    const GenFrame* genFindFrame(const std::string& externalRef) const;
    const GenInterface* genFindInterface(const std::string& externalRef) const;

    NamespacesAccessList genGetAllNamespaces() const;
    InterfacesAccessList genGetAllInterfaces() const;
    MessagesAccessList genGetAllMessages() const;
    MessagesAccessList genGetAllMessagesIdSorted() const;
    FramesAccessList genGetAllFrames() const;
    FieldsAccessList genGetAllFields() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

    GenInterface* genAddDefaultInterface();

    void genSetAllInterfacesReferenced();
    void genSetAllMessagesReferenced();

    bool genHasReferencedMessageIdField() const;
    bool genHasAnyReferencedMessage() const;
    bool genHasAnyReferencedComponent() const;

protected:    
    virtual Type genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;

private:
    std::unique_ptr<GenNamespaceImpl> m_impl;
};

using GenNamespacePtr = GenNamespace::Ptr;

} // namespace gen

} // namespace commsdsl
