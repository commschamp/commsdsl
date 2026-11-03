//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenGenerator.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator final : public commsdsl::gen::GenGenerator
{
    using GenBase = commsdsl::gen::GenGenerator;

public:
    using ParseNamespace = commsdsl::parse::ParseNamespace;
    using ParseSchema = commsdsl::parse::ParseSchema;
    using ParseFrame = commsdsl::parse::ParseFrame;

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
    using ParseMessage = commsdsl::parse::ParseMessage;
    using ParseLayer = commsdsl::parse::ParseLayer;

    WiresharkGenerator();

    [[nodiscard]] static WiresharkGenerator& wiresharkCast(commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<WiresharkGenerator&>(obj);
    }

    [[nodiscard]] static const WiresharkGenerator& wiresharkCast(const commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<const WiresharkGenerator&>(obj);
    }

    [[nodiscard]] static const std::string& wiresharkFileGeneratedComment();

    [[nodiscard]] std::string wiresharkScopeToName(const std::string& scope) const;
    [[nodiscard]] std::string wiresharkFuncNameFor(const GenElem& elem, const std::string& suffix) const;
    [[nodiscard]] std::string wiresharkDissectNameFor(const GenElem& elem) const;

    [[nodiscard]] std::string wiresharkInputRelPathPrefix() const;
    [[nodiscard]] std::string wiresharkInputDissectRelPathFor(const GenElem& elem) const;
    [[nodiscard]] std::string wiresharkInputDissectAbsPathFor(const GenElem& elem) const;
    [[nodiscard]] std::string wiresharkInputDissectRelPathFor(const std::string& name) const;
    [[nodiscard]] std::string wiresharkInputDissectAbsPathFor(const std::string& name) const;
    [[nodiscard]] std::string wiresharkInputRelPathFor(const GenElem& elem, const std::string& suffix) const;
    [[nodiscard]] std::string wiresharkInputAbsPathFor(const GenElem& elem, const std::string& suffix) const;

    [[nodiscard]] unsigned wiresharkDefaultPort() const;

protected:
    [[nodiscard]] virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenInterfacePtr genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent) override;

    [[nodiscard]] virtual GenLayerPtr genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent) override;

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

    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() override;
    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;
    [[nodiscard]] virtual const std::string& genCommentPrefixImpl() const override;

private:
    [[nodiscard]] bool wiresharkWriteExtraFilesInternal() const;

    unsigned m_defaultPort = 0U;
};

} // namespace commsdsl2wireshark
