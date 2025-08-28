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
class CommsDefaultOptions
{
public:
    static bool commsWrite(CommsGenerator& generator);

private:
    explicit CommsDefaultOptions(CommsGenerator& generator) : m_commsGenerator(generator) {}

    bool commsWriteInternal() const;
    bool commsWriteDefaultOptionsInternal() const;
    bool commsWriteClientDefaultOptionsInternal() const;
    bool commsWriteServerDefaultOptionsInternal() const;
    bool commsWriteDataViewDefaultOptionsInternal() const;
    bool commsWriteBareMetalDefaultOptionsInternal() const;
    bool commsWriteMsgFactoryDefaultOptionsInternal() const;
    bool commsWriteAllMessagesDynMemMsgFactoryOptionsInternal() const;
    bool commsWriteClientInputMessagesDynMemMsgFactoryOptionsInternal() const;
    bool commsWriteServerInputMessagesDynMemMsgFactoryOptionsInternal() const;
    bool commsWritePlatformSpecificDynMemMsgFactoryOptionsInternal() const;
    bool commsWriteExtraBundlesDynMemMsgFactoryOptionsInternal() const;
    bool commsWriteSingleMsgFactoryDefaultOptionsInternal(
        const std::string& prefix, 
        const std::string& messagesDesc,
        const std::string& allocDesc) const;

    CommsGenerator& m_commsGenerator;
};

} // namespace commsdsl2comms