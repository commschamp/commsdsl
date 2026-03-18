//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ParseLayerImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseSyncLayerImpl final : public ParseLayerImpl
{
    using Base = ParseLayerImpl;
public:
    ParseSyncLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

    bool parseHasEscField() const
    {
        return
            (m_extEscField != nullptr) ||
            static_cast<bool>(m_escField);
    }

    ParseField parseEscField() const
    {
        if (m_extEscField != nullptr) {
            return ParseField(m_extEscField);
        }

        return ParseField(m_escField.get());
    }

    bool parseSeekField() const
    {
        return m_seekField;
    }

    bool parseVerifyBeforeRead() const
    {
        return m_verifyBeforeRead;
    }

    const std::string& parseFrom() const
    {
        return *m_from;
    }

    bool parseIsAfterPayload() const
    {
        return m_afterPayload;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool parseVerifyImpl(const ParseLayersList& layers) override;

private:
    bool parseUpdateSeekFieldInternal();
    bool parseUpdateEscFieldInternal();
    bool parseUpdateVerifyBeforeReadInternal();
    bool parseUpdateFromInternal();
    bool parseCheckEscFieldFromRefInternal();
    bool parseCheckEscFieldAsChildInternal();

    const ParseFieldImpl* m_extEscField = nullptr;
    ParseFieldImplPtr m_escField;
    const std::string* m_from = nullptr;
    bool m_seekField = false;
    bool m_verifyBeforeRead = false;
    bool m_afterPayload = false;
};

} // namespace parse

} // namespace commsdsl
