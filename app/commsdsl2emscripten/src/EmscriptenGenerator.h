//
// Copyright 2022 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseIntField.h"

#include <string>

namespace commsdsl2emscripten
{

class EmscriptenInterface;
class EmscriptenNamespace;

class EmscriptenGenerator final : public commsdsl::gen::GenGenerator
{
    using GenBase = commsdsl::gen::GenGenerator;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    EmscriptenGenerator();

    [[nodiscard]] static const std::string& emscriptenFileGeneratedComment();

    [[nodiscard]] static EmscriptenGenerator& emscriptenCast(commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<EmscriptenGenerator&>(obj);
    }

    [[nodiscard]] static const EmscriptenGenerator& emscriptenCast(const commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<const EmscriptenGenerator&>(obj);
    }

    [[nodiscard]] std::string emscriptenClassName(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenScopeNameForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenScopeNameForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenProtocolClassNameForRoot(const std::string& name) const;

    [[nodiscard]] std::string emscriptenRelHeaderForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenAbsHeaderForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenRelHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenAbsHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenRelHeaderForInput(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenAbsHeaderForInput(const std::string& name, const EmscriptenNamespace& parent) const;

    [[nodiscard]] std::string emscriptenRelSourceForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenAbsSourceForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenRelSourceForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenAbsSourceForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;

    [[nodiscard]] std::string emscriptenProtocolRelHeaderForRoot(const std::string& name) const;
    [[nodiscard]] std::string emscriptenProtocolRelHeaderForNamespaceMember(const std::string& name, const EmscriptenNamespace& parent) const;
    [[nodiscard]] std::string emscriptenSchemaRelSourceForRoot(unsigned schemaIdx, const std::string& name) const;

    [[nodiscard]] std::string emscriptenRelHeaderFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenRelSourceFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenAbsSourceFor(const commsdsl::gen::GenElem& elem) const;

    [[nodiscard]] std::string emscriptenInputRelHeaderFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenInputAbsHeaderFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenInputRelSourceFor(const commsdsl::gen::GenElem& elem) const;
    [[nodiscard]] std::string emscriptenInputAbsSourceFor(const commsdsl::gen::GenElem& elem) const;

    [[nodiscard]] static std::string emscriptenScopeToName(const std::string& scope);

    void emscriptenSetMainNamespaceInNamesForced(bool value);
    void emscriptenSetHasCodeVersion(bool value);

    [[nodiscard]] bool emscriptenHasCodeVersion() const;

protected:
    [[nodiscard]] virtual bool genWriteImpl() override;

    [[nodiscard]] virtual GenSchemaPtr genCreateSchemaImpl(commsdsl::parse::ParseSchema parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenNamespacePtr genCreateNamespaceImpl(commsdsl::parse::ParseNamespace parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenInterfacePtr genCreateInterfaceImpl(commsdsl::parse::ParseInterface parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenMessagePtr genCreateMessageImpl(commsdsl::parse::ParseMessage parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFramePtr genCreateFrameImpl(commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent) override;

    [[nodiscard]] virtual GenFieldPtr genCreateIntFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateEnumFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateSetFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateFloatFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateBitfieldFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateBundleFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateStringFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateDataFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateListFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateRefFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateOptionalFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFieldPtr genCreateVariantFieldImpl(commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent) override;

    [[nodiscard]] virtual GenLayerPtr genCreateCustomLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSyncLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSizeLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateIdLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateValueLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreatePayloadLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateChecksumLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;

    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    [[nodiscard]] bool emscriptenWriteExtraFilesInternal() const;

    bool m_mainNamespaceInNamesForced = false;
    bool m_hasCodeVersion = false;
};

} // namespace commsdsl2emscripten
