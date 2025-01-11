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

#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigComms
{
public:
    using StringsList = commsdsl::gen::util::StringsList;

    static bool swigWrite(SwigGenerator& generator);

    static const std::string& swigRelHeader();

    static void swigAddDef(StringsList& list);
    static void swigAddCode(const SwigGenerator& generator, StringsList& list);

    static std::string swigErrorStatusClassName(const SwigGenerator& generator);
    static std::string swigOptionalModeClassName(const SwigGenerator& generator);

private:
    explicit SwigComms(SwigGenerator& generator) : m_generator(generator) {}

    bool swigWriteInternal() const;

    SwigGenerator& m_generator;
};

} // namespace commsdsl2swig
