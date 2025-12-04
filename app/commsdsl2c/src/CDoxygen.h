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

#include "CFrame.h"

#include <string>
#include <vector>

namespace commsdsl2c
{

class CGenerator;
class CDoxygen
{
public:
    static bool cWrite(CGenerator& generator);

private:
    explicit CDoxygen(CGenerator& generator);

    void cPrepareInternal();
    bool cWriteInternal() const;

    bool cWriteConfInternal() const;
    bool cWriteLayoutInternal() const;
    bool cWriteMainpageInternal() const;
    std::string cInputDocInternal() const;
    std::string cOutputDocInternal() const;

    CGenerator& m_cGenerator;
    std::vector<const CFrame*> m_frames;
};

} // namespace commsdsl2c