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

#include "CLayer.h"
#include "CInterface.h"
#include "CMsgHandler.h"

#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2c
{

class CGenerator;
class CNamespace;

class CFrame final: public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using ParseFrame = commsdsl::parse::ParseFrame;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenFrame = commsdsl::gen::GenFrame;

    explicit CFrame(CGenerator& generator, ParseFrame parseObj, commsdsl::gen::GenElem* parent);
    virtual ~CFrame();

    [[nodiscard]] static const CFrame* cCast(const GenFrame* obj)
    {
        return static_cast<const CFrame*>(obj);
    }

    [[nodiscard]] std::string cRelHeader() const;
    [[nodiscard]] std::string cRelCommsHeader() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    [[nodiscard]] std::string cCommsType(bool appendOptions = true) const;
    [[nodiscard]] std::string cName() const;
    [[nodiscard]] std::string cCommsTypeName() const;
    [[nodiscard]] bool cIsValidFrame() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using CLayersList = std::vector<CLayer*>;

    [[nodiscard]] bool cWriteHeaderInternal() const;
    [[nodiscard]] bool cWriteSourceInternal() const;
    [[nodiscard]] bool cWriteCommsHeaderInternal() const;
    [[nodiscard]] std::string cHeaderIncludesInternal() const;
    [[nodiscard]] std::string cHeaderLayersCodeInternal() const;
    [[nodiscard]] std::string cHeaderFrameCodeInternal() const;
    [[nodiscard]] std::string cHeaderFrameFieldsCodeInternal() const;
    [[nodiscard]] std::string cSourceIncludesInternal() const;
    [[nodiscard]] std::string cSourceLayersCodeInternal() const;
    [[nodiscard]] std::string cSourceFrameCodeInternal() const;
    [[nodiscard]] std::string cCommsHeaderIncludesInternal() const;
    [[nodiscard]] std::string cCommsHeaderLayersCodeInternal() const;
    [[nodiscard]] std::string cCommsHeaderFrameCodeInternal() const;
    [[nodiscard]] const CInterface* cInterfaceInternal() const;
    [[nodiscard]] const CMsgHandler* cMsgHandlerInternal() const;
    [[nodiscard]] std::string cMsgIdTypeInternal() const;

    CLayersList m_cLayers;
    bool m_validFrame = false;
};

} // namespace commsdsl2c
