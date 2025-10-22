//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

    static const CLayer* cCast(const GenLayer* layer);

    commsdsl::gen::GenLayer& cGenLayer()
    {
        return m_genLayer;
    }

    const commsdsl::gen::GenLayer& cGenLayer() const
    {
        return m_genLayer;
    }

    bool cPrepare();
    void cAddHeaderIncludes(GenStringsList& includes) const;
    void cAddSourceIncludes(GenStringsList& includes) const;
    void cAddCommsHeaderIncludes(GenStringsList& includes) const;
    std::string cName() const;
    std::string cCommsTypeName() const;
    std::string cCommsType() const;
    std::string cHeaderCode() const;
    std::string cSourceCode() const;
    std::string cCommsHeaderCode(const CInterface& iFace, bool& hasInputMessages) const;
    bool cIsInterfaceSupported(const CInterface& iFace) const;
    std::string cFrameValueDef() const;

protected:
    virtual std::string cHeaderCodeImpl() const;
    virtual std::string cSourceCodeImpl() const;
    virtual bool cIsInterfaceSupportedImpl(const CInterface& iFace) const;
    virtual bool cHasInputMessagesImpl() const;
    virtual std::string cFrameValueDefImpl() const;

    const CField* cField() const;
    const CFrame* cParentFrame() const;

private:
    commsdsl::gen::GenLayer& m_genLayer;
    const CField* m_cExternalField = nullptr;
    const CField* m_cMemberField = nullptr;
};

} // namespace commsdsl2c
