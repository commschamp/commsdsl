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

#include "commsdsl/gen/GenMessage.h"

#include <cstdint>
#include <functional>
#include <map>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsNamespace;

class CommsDispatch
{
public:
    CommsDispatch(CommsGenerator& generator, const CommsNamespace& parent);
    bool commsWrite() const;

private:
    using GenMessage = commsdsl::gen::GenMessage;
    using GenMessagesList = std::vector<const GenMessage*>;

    using CommsCheckMsgFunc = std::function<bool (const GenMessage& msg)>;
    using CommsMessagesMap = std::map<std::uintmax_t, GenMessagesList>;

    [[nodiscard]] bool commsWriteDispatchInternal() const;
    [[nodiscard]] bool commsWriteClientDispatchInternal() const;
    [[nodiscard]] bool commsWriteServerDispatchInternal() const;
    [[nodiscard]] bool commsWritePlatformDispatchInternal() const;
    [[nodiscard]] bool commsWriteExtraDispatchInternal() const;

    [[nodiscard]] std::string commsIncludesInternal(const std::string& inputPrefix) const;
    [[nodiscard]] std::string commsDispatchCodeInternal(const std::string& name, CommsCheckMsgFunc&& func) const;
    [[nodiscard]] std::string commsCasesCodeInternal(const CommsMessagesMap& map) const;
    [[nodiscard]] std::string commsMsgDispatcherCodeInternal(const std::string& inputPrefix) const;

    CommsGenerator& m_commsGenerator;
    const CommsNamespace& m_commsParent;
};

} // namespace commsdsl2comms