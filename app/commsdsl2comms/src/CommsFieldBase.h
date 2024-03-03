//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2comms
{

class CommsGenerator;
class CommsFieldBase
{
public:
    static bool write(CommsGenerator& generator);

private:
    explicit CommsFieldBase(CommsGenerator& generator) : m_generator(generator) {}

    bool commsWriteInternal() const;
    
private:
    CommsGenerator& m_generator;
};

} // namespace commsdsl2comms