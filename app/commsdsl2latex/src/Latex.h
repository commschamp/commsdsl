//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2latex
{

class LatexGenerator;
class Latex
{
public:
    static bool latexWrite(LatexGenerator& generator);
    
    static std::string latexDocFileBaseName(const LatexGenerator& generator);
    static std::string latexDocTexFileName(const LatexGenerator& generator);
    
private:
    explicit Latex(LatexGenerator& generator) : m_latexGenerator(generator) {}

private:
    bool latexWriteInternal();
    std::string latexDocumentInternal() const;
    std::string latexPackageInternal() const;
    std::string latexMacroInternal() const;
    std::string latexContentsInternal() const;
    std::string latexTitleInternal() const;

    LatexGenerator& m_latexGenerator;
};

} // namespace commsdsl2latex