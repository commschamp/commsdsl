//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

class Generator;
class Doxygen
{
public:
    static bool write(Generator& generator);

private:
    explicit Doxygen(Generator& generator) : m_generator(generator) {}

    bool writeConf() const;
    bool writeLayout() const;
    bool writeNamespaces() const;
    bool writeMainpage() const;

    std::string getMessagesDoc() const;
    std::string getFieldsDoc() const;
    std::string getInterfacesDoc() const;
    std::string getFramesDoc() const;
    std::string getDispatchDoc() const;
    std::string getPlatformsDoc() const;
    std::string getCustomizeDoc() const;
    std::string getVersionDoc() const;

    Generator& m_generator;
};

} // namespace commsdsl2comms
