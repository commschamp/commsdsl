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

#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsDoxygen
{
public:
    [[nodiscard]] static bool commsWrite(CommsGenerator& generator);

private:
    explicit CommsDoxygen(CommsGenerator& generator) : m_commsGenerator(generator) {}

    [[nodiscard]] bool commsWriteInternal() const;

    [[nodiscard]] bool commsWriteConfInternal() const;
    [[nodiscard]] bool commsWriteLayoutInternal() const;
    [[nodiscard]] bool commsWriteNamespacesInternal() const;
    [[nodiscard]] bool commsWriteMainpageInternal() const;
    [[nodiscard]] std::string commsMessagesDocInternal() const;
    [[nodiscard]] std::string commsFieldsDocInternal() const;
    [[nodiscard]] std::string commsInterfaceDocInternal() const;
    [[nodiscard]] std::string commsFrameDocInternal() const;
    [[nodiscard]] std::string commsDispatchDocInternal() const;
    [[nodiscard]] std::string commsCustomizeDocInternal() const;
    [[nodiscard]] std::string commsVersionDocInternal() const;
    [[nodiscard]] std::string commsPlatformsDocInternal() const;

    CommsGenerator& m_commsGenerator;
};

} // namespace commsdsl2comms