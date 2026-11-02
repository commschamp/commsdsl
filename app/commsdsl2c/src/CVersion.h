//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2c
{

class CGenerator;
class CVersion
{
public:
    [[nodiscard]] static bool cWrite(CGenerator& generator);
    [[nodiscard]] static std::string cRelCommsHeader(const CGenerator& generator);

private:
    explicit CVersion(CGenerator& generator) : m_cGenerator(generator) {}

    [[nodiscard]] bool cWriteHeaderInternal() const;
    [[nodiscard]] bool cWriteCommsHeaderInternal() const;
    [[nodiscard]] std::string cCodeVersionDefineInternal() const;
    [[nodiscard]] std::string cCodeVersionCommsHeaderInternal() const;

    CGenerator& m_cGenerator;
};

} // namespace commsdsl2c