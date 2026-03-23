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
    enum class StatusCode {
        Success,
        NotEnoughData,
        MalformedPacket,
        InvalidMsgId,
        InvalidMsgData,
        CodegenError,
        ValuesLimit // Must be last
    };

    static bool wiresharkWrite(const WiresharkGenerator& generator);
    static std::string wiresharkFileName(const WiresharkGenerator& generator);
    static const std::string& wiresharkProtocolObjName(const WiresharkGenerator& generator);
    static std::string wiresharkCreateFieldFuncName(const WiresharkGenerator& generator);
    static std::string wiresharkCreateExtractorFuncName(const WiresharkGenerator& generator);
    static std::string wiresharkFieldsListName(const WiresharkGenerator& generator);
    static std::string wiresharkExtractorsMapName(const WiresharkGenerator& generator);
    static std::string wiresharkStatusCodeStr(const WiresharkGenerator& generator, StatusCode code);

private:
    explicit Wireshark(const WiresharkGenerator& generator) : m_wiresharkGenerator(generator) {}

private:
    bool wiresharkWriteInternal() const;
    std::string wiresharkProtocolDefInternal() const;
    std::string wiresharkDissectFuncInternal() const;
    std::string wiresharkFieldsRegistrationInternal() const;
    std::string wiresharkCodeInternal() const;
    std::string wiresharkDissectFuncBodyInternal() const;
    std::string wiresharkStatusCodeNameInternal() const;
    std::string wiresharkStatusCodeDefInternal() const;
    std::string wiresharkExtractorsDeclInternal() const;
    std::string wiresharkExtractorsRegCodeInternal() const;
    static const std::string& wiresharkStatusCodeStrInternal(StatusCode code);

    const WiresharkGenerator& m_wiresharkGenerator;
};

} // namespace commsdsl2wireshark