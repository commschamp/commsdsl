//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsDispatch.h"
#include "CommsField.h"
#include "CommsFrame.h"
#include "CommsInputMessages.h"
#include "CommsMessage.h"
#include "CommsMsgFactory.h"
#include "CommsMsgId.h"

#include "commsdsl/gen/GenNamespace.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsNamespace final: public commsdsl::gen::GenNamespace
{
    using Base = commsdsl::gen::GenNamespace;

public:
    explicit CommsNamespace(CommsGenerator& generator, commsdsl::parse::ParseNamespace dslObj, commsdsl::gen::GenElem* parent);
    virtual ~CommsNamespace();

    static const CommsNamespace* cast(const commsdsl::gen::GenNamespace* ptr)
    {
        return static_cast<const CommsNamespace*>(ptr);
    }

    std::string commsDefaultOptions() const;
    std::string commsClientDefaultOptions() const;
    std::string commsServerDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;
    std::string commsMsgFactoryDefaultOptions() const;

    bool commsHasReferencedMsgId() const;
    bool commsHasAnyGeneratedCode() const;
    bool commsHasAnyField() const;

    const CommsField* findValidInterfaceReferencedField(const std::string& refStr) const;

    std::string commsMsgFactoryAliasType() const;
    std::string commsMsgFactoryAliasDef(const std::string& namePrefix, const std::string& typeSuffix) const;
    std::string commsRelHeaderPath(const std::string& namePrefix) const;

    bool commsHasMsgId() const;

protected:
    virtual bool genPrepareImpl() override;    
    virtual bool genWriteImpl() const override;

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
    CommsDispatch m_dispatch;
    CommsMsgFactory m_factory;
    CommsInputMessages m_input;
    CommsMsgId m_msgId;
};

} // namespace commsdsl2comms
