//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2comms
{

class CommsGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;
public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    using CommsExtraMessageBundle = std::pair<std::string, GenMessagesAccessList>;
    using CommsExtraMessageBundlesList = std::vector<CommsExtraMessageBundle>;

    enum class CommsCustomizationLevel
    {
        Full,
        Limited,
        None,
        NumOfValues
    };

    [[nodiscard]] static const CommsGenerator& commsCast(const commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<const CommsGenerator&>(obj);
    }

    [[nodiscard]] static const std::string& commsFileGeneratedComment();
    [[nodiscard]] CommsCustomizationLevel commsGetCustomizationLevel() const;
    void commsSetCustomizationLevel(const std::string& opt);

    [[nodiscard]] bool commsGetMainNamespaceInOptionsForced() const;
    void commsSetMainNamespaceInOptionsForced(bool value);
    [[nodiscard]] bool commsHasMainNamespaceInOptions() const;

    [[nodiscard]] const std::vector<std::string>& commsGetExtraInputBundles() const;
    void commsSetExtraInputBundles(const std::vector<std::string>& inputBundles);
    [[nodiscard]] const CommsExtraMessageBundlesList& commsExtraMessageBundles() const;

    [[nodiscard]] static const std::string& commsMinCommsVersion();

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;

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

    [[nodiscard]] virtual bool genWriteImpl() override;
    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    [[nodiscard]] bool commsPrepareExtraMessageBundlesInternal();
    [[nodiscard]] bool commsWriteExtraFilesInternal() const;

    static const CommsCustomizationLevel DefaultCustomizationLevel = CommsCustomizationLevel::Limited;
    CommsCustomizationLevel m_customizationLevel = DefaultCustomizationLevel;
    std::vector<std::string> m_extraInputBundles;
    CommsExtraMessageBundlesList m_commsExtraMessageBundles;
    bool m_mainNamespaceInOptionsForced = false;
};

} // namespace commsdsl2comms
