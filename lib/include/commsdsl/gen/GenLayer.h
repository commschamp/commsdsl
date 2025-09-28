//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/parse/ParseLayer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenLayerImpl;
class COMMSDSL_API GenLayer : public GenElem
{
    using Base = GenElem;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;

    using GenPtr = std::unique_ptr<GenLayer>;
    using GenLayersAccessList = std::vector<const GenLayer*>;

    virtual ~GenLayer();

    static GenPtr genCreate(GenGenerator& generator, ParseLayer parseObj, GenElem* parent = nullptr);

    bool genPrepare();
    bool genWrite() const;

    ParseLayer genParseObj() const;

    GenField* genExternalField();
    const GenField* genExternalField() const;
    GenField* genMemberField();
    const GenField* genMemberField() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

    // return true if re-order happened, false otherwise
    bool genForceCommsOrder(GenLayersAccessList& layers, bool& success) const;

    std::string genTemplateScopeOfComms(const std::string& iFaceStr, const std::string& allMessagesStr, const std::string& protOptionsStr) const;

protected:
    GenLayer(GenGenerator& generator, const ParseLayer& parseObj, GenElem* parent = nullptr);

    virtual GenType genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;
    virtual bool genForceCommsOrderImpl(GenLayersAccessList& layers, bool& success) const;

private:
    std::unique_ptr<GenLayerImpl> m_impl;
};

using GenLayerPtr = GenLayer::GenPtr;

} // namespace gen

} // namespace commsdsl
