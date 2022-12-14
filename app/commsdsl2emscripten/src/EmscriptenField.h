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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2emscripten
{

class EmscriptenField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using EmscriptenFieldsList = std::vector<EmscriptenField*>;

    explicit EmscriptenField(commsdsl::gen::Field& field);
    virtual ~EmscriptenField();

    static const EmscriptenField* cast(const commsdsl::gen::Field* field);
    static EmscriptenFieldsList emscriptenTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields);

    commsdsl::gen::Field& field()
    {
        return m_field;
    }

    const commsdsl::gen::Field& field() const
    {
        return m_field;
    }

    std::string emscriptenRelHeaderPath() const;

    bool emscriptenIsVersionOptional() const;
    bool emscriptenWrite() const;

    std::string emscriptenHeaderClass() const;
    std::string emscriptenTemplateScope() const;

protected:
    virtual void emscriptenHeaderMembersAddIncludesImpl(StringsList& incs) const;
    virtual std::string emscriptenHeaderMembersImpl() const;
    virtual std::string emscriptenHeaderValueAccImpl() const;
    virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const;
    virtual std::string emscriptenSourceMembersImpl() const;
    virtual std::string emscriptenSourceValueAccImpl() const;
    virtual std::string emscriptenSourceExtraPublicFuncsImpl() const;
    virtual std::string emscriptenSourceBindValueAccImpl() const;
    virtual std::string emscriptenSourceExtraBindImpl() const;


private:
    bool emscriptenWriteHeaderInternal() const;
    bool emscriptenWriteSrcInternal() const;
    std::string emscriptenHeaderIncludesInternal() const;
    std::string emscriptenHeaderClassInternal() const;
    std::string emscriptenHeaderCommonPublicFuncsInternal() const;    
    std::string emscriptenSourceIncludesInternal() const;
    std::string emscriptenSourceCommonPublicFuncsInternal() const;
    std::string emscriptenSourceBindInternal() const;
    std::string emscriptenSourceBindCommonInternal(bool skipVersionOptCheck = false) const;

    commsdsl::gen::Field& m_field;
};

} // namespace commsdsl2emscripten
