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
    enum class WiresharkStatusCode {
        Success,
        NotEnoughData,
        MalformedPacket,
        InvalidMsgId,
        InvalidMsgData,
        ChecksumError,
        CodegenError,
        InvalidFrame,
        ValuesLimit // Must be last
    };

    enum class WiresharkOptMode {
        Tentative,
        Exists,
        Missing,
        ValuesLimit // Must be last
    };

    [[nodiscard]] static bool wiresharkWrite(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkFileName(const WiresharkGenerator& generator, const std::string& suffix = std::string());
    [[nodiscard]] static const std::string& wiresharkProtocolObjName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkCreateFieldFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkCreateExtractorFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkFieldsListName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkExtractorsMapName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkStatusCodeStr(const WiresharkGenerator& generator, WiresharkStatusCode code);
    [[nodiscard]] static std::string wiresharkOptModeStr(const WiresharkGenerator& generator, WiresharkOptMode code);
    [[nodiscard]] static std::string wiresharkOptModeValsName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkFieldValueFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkLocalNamespaceName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkProtVersionGetFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkProtVersionSetFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkPinfoName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkPacketIdFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkCreateCrcFuncName(const WiresharkGenerator& generator);
    [[nodiscard]] static std::string wiresharkBitLibAccFuncName(const WiresharkGenerator& generator);

private:
    explicit Wireshark(const WiresharkGenerator& generator) : m_wiresharkGenerator(generator) {}

private:
    [[nodiscard]] bool wiresharkWriteInternal() const;
    [[nodiscard]] bool wiresharkWriteMainInternal() const;
    [[nodiscard]] bool wiresharkWriteTcpInternal() const;
    [[nodiscard]] std::string wiresharkProtocolDefInternal() const;
    [[nodiscard]] std::string wiresharkLocalInternal() const;
    [[nodiscard]] std::string wiresharkDissectFuncInternal() const;
    [[nodiscard]] std::string wiresharkFieldsRegistrationInternal() const;
    [[nodiscard]] std::string wiresharkCodeInternal() const;
    [[nodiscard]] std::string wiresharkDissectFuncBodyInternal() const;
    [[nodiscard]] std::string wiresharkStatusCodeNameInternal() const;
    [[nodiscard]] std::string wiresharkStatusCodeDefInternal() const;
    [[nodiscard]] std::string wiresharkOptModeNameInternal() const;
    [[nodiscard]] std::string wiresharkOptionalModeDefInternal() const;
    [[nodiscard]] std::string wiresharkExtractorsDeclInternal() const;
    [[nodiscard]] std::string wiresharkExtractorsRegCodeInternal() const;
    [[nodiscard]] std::string wiresharkFieldValueFuncInternal() const;
    [[nodiscard]] std::string wiresharkProtocolVersionDefInternal() const;
    [[nodiscard]] std::string wiresharkBitLibAccessFuncInternal() const;
    [[nodiscard]] std::string wiresharkPinfoDefInternal() const;
    [[nodiscard]] std::string wiresharkCrcCodeDefInternal() const;

    [[nodiscard]] static const std::string& wiresharkStatusCodeStrInternal(WiresharkStatusCode code);
    [[nodiscard]] static const std::string& wiresharkOptModeStrInternal(WiresharkOptMode code);

    const WiresharkGenerator& m_wiresharkGenerator;
};

} // namespace commsdsl2wireshark