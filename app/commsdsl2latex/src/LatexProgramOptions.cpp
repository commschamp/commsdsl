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

#include "LatexProgramOptions.h"

namespace commsdsl2latex
{

namespace
{

const std::string LatexCodeInjectCommentsStr("code-inject-comments");

} // namespace

LatexProgramOptions::LatexProgramOptions()
{
    genAddCommonOptions()
    (LatexCodeInjectCommentsStr, 
        "Show code injection comments in the generated TEX files. "
        "They serve as guidance of what can be injected and where")    
    ;
}

bool LatexProgramOptions::latexCodeInjectCommentsRequested() const
{
    return genIsOptUsed(LatexCodeInjectCommentsStr);
}

} // namespace commsdsl2latex
