//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2swig
{

class SwigGenerator;
class SwigProtocolOptions
{
public:
    using StringsList = commsdsl::gen::util::StringsList;

    static void swigAddCodeIncludes(SwigGenerator& generator, StringsList& list);
    static void swigAddCode(const SwigGenerator& generator, StringsList& list);
    static std::string swigClassName(const SwigGenerator& generator);
    static bool swigIsDefined(const SwigGenerator& generator);
};

} // namespace commsdsl2swig