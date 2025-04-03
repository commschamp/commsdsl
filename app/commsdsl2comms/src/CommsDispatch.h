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

#include "commsdsl/gen/Message.h"

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
    using CheckMsgFunc = std::function<bool (const commsdsl::gen::Message& msg)>;
    using MessagesList = std::vector<const commsdsl::gen::Message*>;
    using MessagesMap = std::map<std::uintmax_t, MessagesList>;

    bool commsWriteDispatchInternal() const;
    bool commsWriteClientDispatchInternal() const;
    bool commsWriteServerDispatchInternal() const;
    bool commsWritePlatformDispatchInternal() const;
    bool commsWriteExtraDispatchInternal() const;

    std::string commsIncludesInternal(const std::string& inputPrefix) const;
    std::string commsDispatchCodeInternal(const std::string& name, CheckMsgFunc&& func) const;
    std::string commsCasesCodeInternal(const MessagesMap& map) const;
    std::string commsMsgIdStringInternal(std::uintmax_t value) const;
    std::string commsMsgDispatcherCodeInternal(const std::string& inputPrefix) const;

    CommsGenerator& m_generator;
    const CommsNamespace& m_parent;
};

} // namespace commsdsl2comms