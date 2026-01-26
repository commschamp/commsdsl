//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class Wireshark
{
public:
    static bool wiresharkWrite(WiresharkGenerator& generator);
    static std::string wiresharkFileName(const WiresharkGenerator& generator);
    static const std::string& wiresharkProtocolObjName(const WiresharkGenerator& generator);

private:
    explicit Wireshark(WiresharkGenerator& generator) : m_wiresharkGenerator(generator) {}

private:
    bool wiresharkWriteInternal() const;
    std::string wiresharkProtocolDefInternal() const;
    std::string wiresharkDissectFuncInternal() const;

    WiresharkGenerator& m_wiresharkGenerator;
};

} // namespace commsdsl2wireshark