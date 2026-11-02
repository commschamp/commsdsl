//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CInterface;
class CNamespace;
class CGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;

public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenElem = commsdsl::gen::GenElem;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using GenGenerator = commsdsl::gen::GenGenerator;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;

    [[nodiscard]] static const std::string& cFileGeneratedComment();

    [[nodiscard]] static CGenerator& cCast(GenGenerator& obj)
    {
        return static_cast<CGenerator&>(obj);
    }

    [[nodiscard]] static const CGenerator& cCast(const GenGenerator& obj)
    {
        return static_cast<const CGenerator&>(obj);
    }

    [[nodiscard]] std::string cRelHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cAbsHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cRelSourceFor(const GenElem& elem) const;
    [[nodiscard]] std::string cAbsSourceFor(const GenElem& elem) const;
    [[nodiscard]] std::string cRelCommsHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cAbsCommsHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cRelHeaderForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cAbsHeaderForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cRelCommsHeaderForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cAbsCommsHeaderForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cRelSourceForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cAbsSourceForNamespaceMember(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cRelHeaderForInput(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cAbsHeaderForInput(const std::string& name, const CNamespace& parent) const;
    [[nodiscard]] std::string cRelRootHeaderFor(const std::string& name) const;
    [[nodiscard]] std::string cAbsRootHeaderFor(const std::string& name) const;
    [[nodiscard]] std::string cRelRootSourceFor(const std::string& name) const;
    [[nodiscard]] std::string cAbsRootSourceFor(const std::string& name) const;
    [[nodiscard]] std::string cRelRootCommsHeaderFor(const std::string& name) const;
    [[nodiscard]] std::string cAbsRootCommsHeaderFor(const std::string& name) const;
    [[nodiscard]] std::string cRelPathForDoc(const std::string& name) const;
    [[nodiscard]] std::string cAbsPathForDoc(const std::string& name) const;

    [[nodiscard]] std::string cInputRelHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cInputAbsHeaderFor(const GenElem& elem) const;
    [[nodiscard]] std::string cInputRelSourceFor(const GenElem& elem) const;
    [[nodiscard]] std::string cInputAbsSourceFor(const GenElem& elem) const;
    [[nodiscard]] std::string cInputRelPathForDoc(const std::string& name) const;
    [[nodiscard]] std::string cInputAbsPathForDoc(const std::string& name) const;
    [[nodiscard]] std::string cInputRelRootHeaderFor(const std::string& name) const;
    [[nodiscard]] std::string cInputAbsRootHeaderFor(const std::string& name) const;

    [[nodiscard]] std::string cNameFor(const GenElem& elem) const;
    [[nodiscard]] const std::string& cNamesPrefix() const;

    [[nodiscard]] static std::string cScopeToName(const std::string& scope);
    [[nodiscard]] static const std::string& cCppGuardBegin(bool addBool = true);
    [[nodiscard]] static const std::string& cCppGuardEnd();

    [[nodiscard]] const GenStringsList& cProtocolOptions() const;
    [[nodiscard]] const std::string& cInputName() const;

    [[nodiscard]] const CInterface* cForcedInterface() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() override;

    [[nodiscard]] virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenInterfacePtr genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent) override;

    [[nodiscard]] virtual GenFieldPtr genCreateIntFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateSetFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateStringFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateDataFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateListFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateRefFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent) override;

    [[nodiscard]] virtual GenLayerPtr genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent) override;

    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    [[nodiscard]] bool cWriteExtraFilesInternal() const;
    void cSetNamesPrefixInternal(const std::string& value);
    void cSetCommsOptionsInternal(const std::string& value);
    void cSetCommsInputInternal(const std::string& value);
    void cSetCommsInterfaceInternal(const std::string& value);

    [[nodiscard]] bool cPrepareNamesPrefixInternal();
    [[nodiscard]] bool cPrepareCommsOptionsInternal();
    [[nodiscard]] bool cPrepareForcedInterfaceInternal();

    std::string m_namesPrefix;
    std::string m_inputName;
    GenStringsList m_commsOptions;
    const CInterface* m_forcedInterface = nullptr;
};

} // namespace commsdsl2c
