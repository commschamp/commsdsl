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
    using ParseNamespace = commsdsl::parse::ParseNamespace;

    using GenPtr = std::unique_ptr<GenNamespace>;
    using GenNamespacesList = std::vector<GenPtr>;
    using GenFieldsList = GenField::GenFieldsList;
    using GenInterfacesList = std::vector<GenInterfacePtr>;
    using GenMessagesList = std::vector<GenMessagePtr>;
    using GenFramesList = std::vector<GenFramePtr>;
    using GenNamespacesAccessList = std::vector<const GenNamespace*>;
    using GenInterfacesAccessList = std::vector<const GenInterface*>;
    using GenMessagesAccessList = std::vector<const GenMessage*>;
    using GenFramesAccessList = std::vector<const GenFrame*>;
    using GenFieldsAccessList = std::vector<const GenField*>;

    explicit GenNamespace(GenGenerator& generator, ParseNamespace parseObj, GenElem* parent = nullptr);
    virtual ~GenNamespace();

    static const GenNamespace& genCast(const GenElem& obj)
    {
        return static_cast<const GenNamespace&>(obj);
    }

    bool genCreateAll();
    bool genPrepare();
    bool genWrite() const;

    ParseNamespace genParseObj() const;
    std::string genAdjustedExternalRef() const;

    const GenNamespacesList& genNamespaces() const;
    const GenFieldsList& genFields() const;
    const GenInterfacesList& genInterfaces() const;
    const GenMessagesList& genMessages() const;
    const GenFramesList& genFrames() const;
    bool genHasFramesRecursive() const;
    bool genHasMessagesRecursive() const;

    GenFieldsAccessList genFindMessageIdFields() const;
    const GenField* genFindField(const std::string& externalRef) const;
    const GenMessage* genGindMessage(const std::string& externalRef) const;
    const GenFrame* genFindFrame(const std::string& externalRef) const;
    const GenInterface* genFindInterface(const std::string& externalRef) const;

    GenNamespacesAccessList genGetAllNamespaces() const;
    GenInterfacesAccessList genGetAllInterfaces() const;
    GenMessagesAccessList genGetAllMessages() const;
    GenMessagesAccessList genGetAllMessagesIdSorted() const;
    GenFramesAccessList genGetAllFrames() const;
    GenFieldsAccessList genGetAllFields() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

    GenInterface* genAddDefaultInterface();

    void genSetAllInterfacesReferenced();
    void genSetAllMessagesReferenced();

    bool genHasReferencedMessageIdField() const;
    bool genHasAnyReferencedMessage() const;
    bool genHasAnyReferencedComponent() const;

    const GenInterface* genFindSuitableInterface() const;
    bool genIsSuitableInterface(const GenInterface& iFace) const;

protected:
    virtual GenType genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;

private:
    std::unique_ptr<GenNamespaceImpl> m_impl;
};

using GenNamespacePtr = GenNamespace::GenPtr;

} // namespace gen

} // namespace commsdsl
