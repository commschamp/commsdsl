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

#include "WiresharkNamespace.h"

#include "WiresharkField.h"
#include "WiresharkFrame.h"
#include "WiresharkGenerator.h"
#include "WiresharkInterface.h"
#include "WiresharkMessage.h"

#include "commsdsl/gen/util.h"

namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkNamespace::WiresharkNamespace(WiresharkGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkNamespace::~WiresharkNamespace() = default;

const WiresharkInterface* WiresharkNamespace::wiresharkInterface() const
{
    auto iFace = genFindSuitableInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    return WiresharkInterface::wiresharkCast(iFace);
}

std::string WiresharkNamespace::wiresharkDissectCode() const
{
    static const std::string Templ =
        "#^#NAMESPACES#$#\n"
        "#^#FIELDS#$#\n"
        "#^#INTERFACES#$#\n"
        "#^#MESSAGES#$#\n"
        "#^#FRAMES#$#\n"
        ;

    util::GenStringsList namespaces;
    for (auto& nsPtr : genNamespaces()) {
        auto str = WiresharkNamespace::wiresharkCast(nsPtr.get())->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        namespaces.push_back(std::move(str));
    }

    util::GenStringsList fields;
    for (auto& fPtr : genFields()) {
        auto str = WiresharkField::wiresharkCast(fPtr.get())->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    util::GenStringsList interfaces;
    for (auto& iPtr : genInterfaces()) {
        auto str = WiresharkInterface::wiresharkCast(iPtr.get())->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        interfaces.push_back(std::move(str));
    }

    util::GenStringsList messages;
    for (auto& mPtr : genMessages()) {
        auto str = WiresharkMessage::wiresharkCast(*mPtr).wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        messages.push_back(std::move(str));
    }

    util::GenStringsList frames;
    for (auto& fPtr : genFrames()) {
        auto str = WiresharkFrame::wiresharkCast(*fPtr).wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        frames.push_back(std::move(str));
    }

    util::GenReplacementMap repl = {
        {"NAMESPACES", util::genStrListToString(namespaces, "\n", "")},
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
        {"INTERFACES", util::genStrListToString(interfaces, "\n", "")},
        {"MESSAGES", util::genStrListToString(messages, "\n", "")},
        {"FRAMES", util::genStrListToString(frames, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkNamespace::wiresharkExtractorsRegCode() const
{
    static const std::string Templ =
        "#^#NAMESPACES#$#\n"
        "#^#FIELDS#$#\n"
        "#^#INTERFACES#$#\n"
        "#^#MESSAGES#$#\n"
        "#^#FRAMES#$#\n"
        ;

    util::GenStringsList namespaces;
    for (auto& nsPtr : genNamespaces()) {
        auto str = WiresharkNamespace::wiresharkCast(nsPtr.get())->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        namespaces.push_back(std::move(str));
    }

    util::GenStringsList fields;
    for (auto& fPtr : genFields()) {
        auto str = WiresharkField::wiresharkCast(fPtr.get())->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    util::GenStringsList interfaces;
    for (auto& iPtr : genInterfaces()) {
        auto str = WiresharkInterface::wiresharkCast(iPtr.get())->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        interfaces.push_back(std::move(str));
    }

    util::GenStringsList messages;
    for (auto& mPtr : genMessages()) {
        auto str = WiresharkMessage::wiresharkCast(*mPtr).wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        messages.push_back(std::move(str));
    }

    util::GenStringsList frames;
    for (auto& fPtr : genFrames()) {
        auto str = WiresharkFrame::wiresharkCast(*fPtr).wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        frames.push_back(std::move(str));
    }

    util::GenReplacementMap repl = {
        {"NAMESPACES", util::genStrListToString(namespaces, "", "")},
        {"FIELDS", util::genStrListToString(fields, "", "")},
        {"INTERFACES", util::genStrListToString(interfaces, "", "")},
        {"MESSAGES", util::genStrListToString(messages, "", "")},
        {"FRAMES", util::genStrListToString(frames, "", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
