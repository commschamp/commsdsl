//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "Field.h"

namespace commsdsl
{

namespace parse
{

class BitfieldFieldImpl;
class COMMSDSL_API BitfieldField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BitfieldField(const BitfieldFieldImpl* impl);
    explicit BitfieldField(Field field);

    Endian endian() const;
    Members members() const;
};

} // namespace parse

} // namespace commsdsl
