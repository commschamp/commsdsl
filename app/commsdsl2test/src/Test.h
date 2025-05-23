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

namespace commsdsl2test
{

class TestGenerator;
class Test
{
public:
    static bool write(TestGenerator& generator);

private:
    explicit Test(TestGenerator& generator) : m_generator(generator) {}

    bool writeInputTest() const;
private:
    TestGenerator& m_generator;
};

} // namespace commsdsl2test