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

namespace commsdsl2comms
{

class CommsGenerator;
class CommsVersion
{
public:
    static bool commsWrite(CommsGenerator& generator);

private:
    explicit CommsVersion(CommsGenerator& generator) : m_commsGenerator(generator) {}

    bool commsWriteInternal() const;
    std::string commsCodeVersionDefineInternal() const;
    std::string commsCodeVersionFuncsInternal() const;

    CommsGenerator& m_commsGenerator;
};

} // namespace commsdsl2comms