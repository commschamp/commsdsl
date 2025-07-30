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

#include "LatexEnumField.h"

#include "LatexGenerator.h"

namespace commsdsl2latex
{

LatexEnumField::LatexEnumField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this)) 
{
}   

bool LatexEnumField::genWriteImpl() const
{
    return latexWrite();
}


} // namespace commsdsl2latex
