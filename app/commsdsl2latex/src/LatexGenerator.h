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

#include "commsdsl/gen/GenGenerator.h"

namespace commsdsl2latex 
{

class LatexGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;

public:
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    static const std::string& latexFileGeneratedComment();
    static const std::string& latexCodeInjectCommentPrefix();

    std::string latexInputCodePathForFile(const std::string& name) const;    

    bool latexHasCodeInjectionComments() const
    {
        return m_hasCodeInjectionComments;
    }

protected:
    virtual bool genWriteImpl() override;    

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    bool latexWriteExtraFilesInternal() const;

    bool m_hasCodeInjectionComments = false;
};

} // namespace commsdsl2latex
