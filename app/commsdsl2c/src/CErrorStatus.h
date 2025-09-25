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

#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CGenerator;
class CErrorStatus
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    static std::string cName(const CGenerator& generator);
    static std::string cRelHeaderPath(const CGenerator& generator);
    static std::string cRelSourcePath(const CGenerator& generator);
    static void cAddSourceFiles(const CGenerator& generator, GenStringsList& sources);

    static bool cWrite(CGenerator& generator);

private:
    explicit CErrorStatus(CGenerator& generator);

    bool cWriteHeaderInternal();
    bool cWriteSourceInternal();

    CGenerator& m_cGenerator;
};

} // namespace commsdsl2c