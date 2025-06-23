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
#include "commsdsl/parse/ParseNamespace.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenMessage.h"

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
    using InterfacesList = std::vector<InterfacePtr>;
    using MessagesList = std::vector<MessagePtr>;
    using FramesList = std::vector<FramePtr>;
    using NamespacesAccessList = std::vector<const GenNamespace*>;
    using InterfacesAccessList = std::vector<const GenInterface*>;
    using MessagesAccessList = std::vector<const GenMessage*>;
    using FramesAccessList = std::vector<const GenFrame*>;
    using FieldsAccessList = std::vector<const GenField*>;

    explicit GenNamespace(GenGenerator& generator, commsdsl::parse::ParseNamespace dslObj, GenElem* parent = nullptr);
    virtual ~GenNamespace();

    bool createAll();
    bool prepare();
    bool write() const;

    commsdsl::parse::ParseNamespace dslObj() const;
    std::string adjustedExternalRef() const;

    const NamespacesList& namespaces() const;
    const FieldsList& fields() const;
    const InterfacesList& interfaces() const;
    const MessagesList& messages() const;
    const FramesList& frames() const;
    bool hasFramesRecursive() const;
    bool hasMessagesRecursive() const;

    FieldsAccessList findMessageIdFields() const;
    const GenField* findField(const std::string& externalRef) const;
    const GenMessage* findMessage(const std::string& externalRef) const;
    const GenFrame* findFrame(const std::string& externalRef) const;
    const GenInterface* findInterface(const std::string& externalRef) const;

    NamespacesAccessList getAllNamespaces() const;
    InterfacesAccessList getAllInterfaces() const;
    MessagesAccessList getAllMessages() const;
    MessagesAccessList getAllMessagesIdSorted() const;
    FramesAccessList getAllFrames() const;
    FieldsAccessList getAllFields() const;

    GenGenerator& generator();
    const GenGenerator& generator() const;

    GenInterface* addDefaultInterface();

    void setAllInterfacesReferenced();
    void setAllMessagesReferenced();

    bool hasReferencedMessageIdField() const;
    bool hasAnyReferencedMessage() const;
    bool hasAnyReferencedComponent() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<GenNamespaceImpl> m_impl;
};

using NamespacePtr = GenNamespace::Ptr;

} // namespace gen

} // namespace commsdsl
