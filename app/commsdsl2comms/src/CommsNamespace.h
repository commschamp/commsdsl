//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsField.h"
#include "CommsFrame.h"
#include "CommsMessage.h"

#include "commsdsl/gen/Namespace.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsNamespace final: public commsdsl::gen::Namespace
{
    using Base = commsdsl::gen::Namespace;

public:
    explicit CommsNamespace(CommsGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent);
    virtual ~CommsNamespace();

    std::string commsDefaultOptions() const;
    std::string commsClientDefaultOptions() const;
    std::string commsServerDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;

    bool commsHasReferencedMsgId() const;
    bool commsHasAnyGeneratedCode() const;
    bool commsHasAnyField() const;

protected:
    virtual bool prepareImpl() override;    

private:
    using NamespaceOptsFunc = std::string (CommsNamespace::*)() const;
    using FieldOptsFunc = std::string (CommsField::*)() const;
    using MessageOptsFunc = std::string (CommsMessage::*)() const;
    using FrameOptsFunc = std::string (CommsFrame::*)() const;
    using CommsFieldsList = CommsField::CommsFieldsList;

    std::string commsOptionsInternal(
        NamespaceOptsFunc nsOptsFunc,
        FieldOptsFunc fieldOptsFunc,
        MessageOptsFunc messageOptsFunc,
        FrameOptsFunc frameOptsFunc,
        bool hasBase) const;

    CommsFieldsList m_commsFields;        
};

} // namespace commsdsl2comms
