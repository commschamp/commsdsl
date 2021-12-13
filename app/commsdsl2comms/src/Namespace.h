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

#include <map>
#include <string>
#include <memory>

#include "commsdsl/parse/Namespace.h"
#include "Message.h"
#include "Interface.h"
#include "Field.h"
#include "Frame.h"
#include "common.h"


namespace commsdsl2comms
{

class Generator;
class Namespace
{
public:
    using Ptr = std::unique_ptr<Namespace>;
    using NamespacesList = std::vector<Ptr>;
    using InterfacesList = std::vector<InterfacePtr>;
    using FieldsList = std::vector<FieldPtr>;
    using MessagesAccessList = std::vector<const Message*>;
    using InterfacesAccessList = std::vector<const Interface*>;
    using FramesAccessList = std::vector<const Frame*>;
    using NamespacesScopesList = std::vector<std::string>;

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Namespace(Generator& gen, const commsdsl::parse::Namespace& dslObj)
      : m_generator(gen),
        m_dslObj(dslObj)
    {
    }

    const std::string& name() const;

    bool prepare();

    bool writeInterfaces();
    bool writeMessages();
    bool writeFrames();
    bool writeFields();

    std::string getDefaultOptions(const std::string& base) const;
    std::string getClientOptions(const std::string& base) const;
    std::string getServerOptions(const std::string& base) const;
    std::string getBareMetalDefaultOptions(const std::string& base) const;
    std::string getDataViewDefaultOptions(const std::string& base) const;

    NamespacesScopesList getNamespacesScopes() const;
    MessagesAccessList getAllMessages() const;
    InterfacesAccessList getAllInterfaces() const;
    FramesAccessList getAllFrames() const;

    bool hasInterfaceDefined();

    const Field* findMessageIdField() const;
    const Field* findField(const std::string& externalRef, bool record);
    const Interface* findInterface(const std::string& externalRef) const;
    const Frame* findFrame(const std::string& externalRef) const;

    bool anyInterfaceHasVersion() const;

    common::StringsList pluginCommonSources() const;

    std::string externalRef() const;
    bool addDefaultInterface();

    bool hasFrame() const;

private:

    using MessagesList = std::vector<MessagePtr>;
    using FramesList = std::vector<FramePtr>;
    using AccessedFields = std::map<const Field*, bool>;

    bool prepareNamespaces();
    bool prepareFields();
    bool prepareInterfaces();
    bool prepareMessages();
    bool prepareFrames();
    void recordAccessedField(const Field* field);

    using GetNamespaceOptionsFunc = std::string (Namespace::*)(const std::string& base) const;
    using GetFieldOptionsFunc = std::string (Field::*)(const std::string& base, const std::string& scope) const;
    using GetMessageOptionsFunc = std::string (Message::*)(const std::string& base) const;
    using GetFrameOptionsFunc = std::string (Frame::*)(const std::string& base) const;
    std::string getClientServerOptions(GetMessageOptionsFunc, const std::string& base) const;
    std::string getOptions(
        GetNamespaceOptionsFunc nsFunc,
        GetFieldOptionsFunc fieldFunc,
        GetMessageOptionsFunc messageFunc,
        GetFrameOptionsFunc frameFunc,
        const std::string& base) const;

    Generator& m_generator;
    commsdsl::parse::Namespace m_dslObj;
    NamespacesList m_namespaces;
    FieldsList m_fields;
    InterfacesList m_interfaces;
    MessagesList m_messages;
    FramesList m_frames;
    AccessedFields m_accessedFields;
};

using NamespacePtr = Namespace::Ptr;

inline
NamespacePtr createNamespace(Generator& gen, const commsdsl::parse::Namespace& dslObj)
{
    return NamespacePtr(new Namespace(gen, dslObj));
}

} // namespace commsdsl2comms