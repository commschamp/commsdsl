//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Message.h"

#include "common.h"

namespace commsdsl2old
{

class Generator;
class Dispatch
{
public:
    static bool write(Generator& generator);

private:
    using DslMessagesList = std::vector<commsdsl::parse::Message>;

    explicit Dispatch(Generator& generator) : m_generator(generator) {}

    bool writeProtocolDefinition() const;
    std::string getDispatchFunc(
        const std::string& funcName,
        const DslMessagesList& messages) const;
    std::string getMsgDispatcher(
        const std::string& fileName) const;
    std::string getIdString(std::uintmax_t value) const;

    Generator& m_generator;
};

} // namespace commsdsl2old