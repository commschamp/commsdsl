//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/BitfieldField.h"

#include <cassert>

#include "BitfieldFieldImpl.h"

namespace commsdsl
{

namespace
{

const BitfieldFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const BitfieldFieldImpl*>(ptr);
}

} // namespace

BitfieldField::BitfieldField(const BitfieldFieldImpl* impl)
  : Base(impl)
{
}

BitfieldField::BitfieldField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Bitfield);
}

Endian BitfieldField::endian() const
{
    return cast(m_pImpl)->endian();
}

BitfieldField::Members BitfieldField::members() const
{
    return cast(m_pImpl)->membersList();
}

} // namespace commsdsl
