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

#include <string>

namespace commsdsl
{

namespace gen
{

class GenGenerator;
class GenElem
{
public:
    enum Type
    {
        Type_Invalid,
        Type_Namespace,
        Type_Message,
        Type_Field,
        Type_Interface,
        Type_Frame,
        Type_Layer,
        Type_Schema,
        Type_NumOfValues
    };
    virtual ~GenElem();

    void genSetParent(GenElem* parent);
    GenElem* genGetParent();
    const GenElem* genGetParent() const;

    Type genElemType() const;

    const std::string& genName() const;

protected:
    explicit GenElem(GenElem* parent = nullptr);

    virtual Type genElemTypeImpl() const = 0;

private:
    GenElem* m_parent = nullptr;
};

} // namespace gen

} // namespace commsdsl
