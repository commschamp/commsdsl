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

#include "CField.h"
#include "CInterface.h"

#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2c
{

class CFrame;
class CLayer
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenLayer = commsdsl::gen::GenLayer;

    explicit CLayer(GenLayer& layer);
    virtual ~CLayer();

    [[nodiscard]] static const CLayer* cCast(const GenLayer* layer);

    [[nodiscard]] commsdsl::gen::GenLayer& cGenLayer()
    {
        return m_genLayer;
    }

    [[nodiscard]] const commsdsl::gen::GenLayer& cGenLayer() const
    {
        return m_genLayer;
    }

    [[nodiscard]] bool cPrepare();
    void cAddHeaderIncludes(GenStringsList& includes) const;
    void cAddSourceIncludes(GenStringsList& includes) const;
    void cAddCommsHeaderIncludes(GenStringsList& includes) const;
    [[nodiscard]] std::string cName() const;
    [[nodiscard]] std::string cCommsTypeName() const;
    [[nodiscard]] std::string cCommsType() const;
    [[nodiscard]] std::string cHeaderCode() const;
    [[nodiscard]] std::string cSourceCode() const;
    [[nodiscard]] std::string cCommsHeaderCode(const CInterface& iFace) const;
    [[nodiscard]] bool cIsInterfaceSupported(const CInterface& iFace) const;
    [[nodiscard]] std::string cFrameValueDef() const;
    [[nodiscard]] std::string cFrameValueAssign(
        const std::string& valuesPtrName,
        const std::string& commsBundleName,
        unsigned layerIdx) const;

protected:
    [[nodiscard]] virtual std::string cHeaderCodeImpl() const;
    [[nodiscard]] virtual std::string cSourceCodeImpl() const;
    [[nodiscard]] virtual bool cIsInterfaceSupportedImpl(const CInterface& iFace) const;
    [[nodiscard]] virtual bool cHasInputMessagesImpl() const;
    [[nodiscard]] virtual std::string cFrameValueDefImpl() const;
    [[nodiscard]] virtual std::string cFrameValueAssignImpl(
        const std::string& valuesPtrName,
        const std::string& commsBundleName,
        unsigned layerIdx) const;

    [[nodiscard]] const CField* cField() const;
    [[nodiscard]] const CFrame* cParentFrame() const;

private:
    commsdsl::gen::GenLayer& m_genLayer;
    const CField* m_cExternalField = nullptr;
    const CField* m_cMemberField = nullptr;
};

} // namespace commsdsl2c
