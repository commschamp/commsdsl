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

    [[nodiscard]] static const GenNamespace& genCast(const GenElem& obj)
    {
        return static_cast<const GenNamespace&>(obj);
    }

    [[nodiscard]] bool genCreateAll();
    [[nodiscard]] bool genPrepare();
    [[nodiscard]] bool genWrite() const;

    [[nodiscard]] ParseNamespace genParseObj() const;
    [[nodiscard]] std::string genAdjustedExternalRef() const;

    [[nodiscard]] const GenNamespacesList& genNamespaces() const;
    [[nodiscard]] const GenFieldsList& genFields() const;
    [[nodiscard]] const GenInterfacesList& genInterfaces() const;
    [[nodiscard]] const GenMessagesList& genMessages() const;
    [[nodiscard]] const GenFramesList& genFrames() const;
    [[nodiscard]] bool genHasFramesRecursive() const;
    [[nodiscard]] bool genHasMessagesRecursive() const;

    [[nodiscard]] GenFieldsAccessList genFindMessageIdFields() const;
    [[nodiscard]] const GenField* genFindField(const std::string& externalRef) const;
    [[nodiscard]] const GenMessage* genFindMessage(const std::string& externalRef) const;
    [[nodiscard]] const GenFrame* genFindFrame(const std::string& externalRef) const;
    [[nodiscard]] const GenInterface* genFindInterface(const std::string& externalRef) const;

    [[nodiscard]] GenNamespacesAccessList genGetAllNamespaces() const;
    [[nodiscard]] GenInterfacesAccessList genGetAllInterfaces() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessages() const;
    [[nodiscard]] GenMessagesAccessList genGetAllMessagesIdSorted() const;
    [[nodiscard]] GenFramesAccessList genGetAllFrames() const;
    [[nodiscard]] GenFieldsAccessList genGetAllFields() const;

    [[nodiscard]] GenGenerator& genGenerator();
    [[nodiscard]] const GenGenerator& genGenerator() const;

    [[nodiscard]] GenInterface* genAddDefaultInterface();

    void genSetAllInterfacesReferenced();
    void genSetAllMessagesReferenced();

    [[nodiscard]] bool genHasReferencedMessageIdField() const;
    [[nodiscard]] bool genHasAnyReferencedMessage() const;
    [[nodiscard]] bool genHasAnyReferencedComponent() const;

    [[nodiscard]] const GenInterface* genFindSuitableInterface() const;
    [[nodiscard]] bool genIsSuitableInterface(const GenInterface& iFace) const;

protected:
    [[nodiscard]] virtual GenType genElemTypeImpl() const override final;
    [[nodiscard]] virtual bool genPrepareImpl();
    [[nodiscard]] virtual bool genWriteImpl() const;

private:
    std::unique_ptr<GenNamespaceImpl> m_impl;
};

using GenNamespacePtr = GenNamespace::GenPtr;

} // namespace gen

} // namespace commsdsl
