//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Interface.h"

#include "Field.h"

#include <memory>

namespace commsdsl2old
{

class Generator;
class Interface
{
public:

    explicit Interface(Generator& gen, const commsdsl::parse::Interface& msg)
      : m_generator(gen),
        m_dslObj(msg)
    {
    }

    bool prepare();

    bool write();

    const std::string& name() const;

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

    bool hasVersion() const;
    bool hasFields() const;
    std::vector<std::string> getVersionFields() const;

private:
    bool writeProtocolDefinitionCommonFile();
    bool writeProtocol();
    bool writePluginHeader();
    bool writePluginSrc();
    std::string getDescription() const;
    std::string getFieldsClassesList() const;
    std::string getFieldsAccessList() const;
    std::string getAliases() const;
    std::string getIncludes() const;
    std::string getFieldsAccessDoc() const;
    std::string getFieldsDef() const;
    std::string getFieldsOpts() const;
    unsigned getHexMsgIdWidth() const;

    Generator& m_generator;
    commsdsl::parse::Interface m_dslObj;
    std::string m_externalRef;
    std::vector<FieldPtr> m_fields;
};

using InterfacePtr = std::unique_ptr<Interface>;

inline
InterfacePtr createInterface(Generator& gen, const commsdsl::parse::Interface& msg)
{
    return InterfacePtr(new Interface(gen, msg));
}

} // namespace commsdsl2old