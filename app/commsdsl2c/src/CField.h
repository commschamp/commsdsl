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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2c
{

class CField
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenField = commsdsl::gen::GenField;
    using GenFieldsList = GenField::GenFieldsList;

    using CIncludesList = GenStringsList;
    using CFieldsList = std::vector<CField*>;

    explicit CField(GenField& field);
    virtual ~CField();

    static CFieldsList cTransformFieldsList(const GenFieldsList& fields);

    bool cWrite() const;

    const GenField& cGenField() const
    {
        return m_genField;
    }

protected:

private:
    bool cWriteHeaderInternal() const;
    bool cWriteSrcInternal() const;
    
    GenField& m_genField;
};

} // namespace commsdsl2c
