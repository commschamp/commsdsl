//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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
    [[nodiscard]] static bool latexWrite(LatexGenerator& generator);

    [[nodiscard]] static std::string latexDocFileBaseName(const LatexGenerator& generator);
    [[nodiscard]] static std::string latexDocTexFileName(const LatexGenerator& generator);
    [[nodiscard]] static std::string latexDocCfgFileName(const LatexGenerator& generator);

private:
    explicit Latex(LatexGenerator& generator) : m_latexGenerator(generator) {}

private:
    [[nodiscard]] bool latexWriteInternal();
    [[nodiscard]] bool latexWriteCfgInternal();
    [[nodiscard]] std::string latexDocumentInternal() const;
    [[nodiscard]] std::string latexPackageInternal() const;
    [[nodiscard]] std::string latexMacroInternal() const;
    [[nodiscard]] std::string latexContentsInternal() const;
    [[nodiscard]] std::string latexTitleInternal() const;

    LatexGenerator& m_latexGenerator;
};

} // namespace commsdsl2latex