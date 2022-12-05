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

#include "EmscriptenField.h"

#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenMessage final: public commsdsl::gen::Message
{
    using Base = commsdsl::gen::Message;

public:
    using StringsList = commsdsl::gen::util::StringsList;
    
    explicit EmscriptenMessage(EmscriptenGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent);
    virtual ~EmscriptenMessage();

    static const EmscriptenMessage* cast(const commsdsl::gen::Message* i)
    {
        return static_cast<const EmscriptenMessage*>(i);
    }

protected:
    virtual bool prepareImpl() override;    
    virtual bool writeImpl() const override;

private:
    using EmscriptenFieldsList = EmscriptenField::EmscriptenFieldsList;

    EmscriptenFieldsList m_emscriptenFields;        
};

} // namespace commsdsl2emscripten
