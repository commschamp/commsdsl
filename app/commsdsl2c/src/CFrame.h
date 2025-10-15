//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CLayer.h"

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

    static const CFrame* cCast(const GenFrame* i)
    {
        return static_cast<const CFrame*>(i);
    }

    std::string cRelHeader() const;
    std::string cRelCommsHeader() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    std::string cCommsType() const;
    std::string cName() const;
    std::string cCommsTypeName() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using CLayersList = std::vector<CLayer*>;

    bool cWriteHeaderInternal() const;
    bool cWriteSourceInternal() const;

    CLayersList m_cLayers;
    bool m_validFrame = true;
};

} // namespace commsdsl2c
