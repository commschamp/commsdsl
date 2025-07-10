//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2emscripten
{

class EmscriptenField
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenFieldsList = commsdsl::gen::GenField::GenFieldsList;

    using EmscriptenIncludesList = GenStringsList;
    using EmscriptenFieldsList = std::vector<EmscriptenField*>;

    explicit EmscriptenField(commsdsl::gen::GenField& field);
    virtual ~EmscriptenField();

    static const EmscriptenField* emscriptenCast(const commsdsl::gen::GenField* field);
    static EmscriptenField* emscriptenCast(commsdsl::gen::GenField* field);
    static EmscriptenFieldsList emscriptenTransformFieldsList(const GenFieldsList& fields);

    commsdsl::gen::GenField& emscriptenGenField()
    {
        return m_genField;
    }

    const commsdsl::gen::GenField& emscriptenGenField() const
    {
        return m_genField;
    }

    std::string emscriptenRelHeaderPath() const;

    bool emscriptenIsVersionOptional() const;
    bool emscriptenWrite() const;

    std::string emscriptenHeaderClass() const;
    std::string emscriptenTemplateScope() const;
    std::string emscriptenSourceCode() const;

    void emscriptenHeaderAddExtraIncludes(GenStringsList& incs) const;
    void emscriptenAddSourceFiles(GenStringsList& sources) const;

    void emscriptenSetListElement()
    {
        m_listElement = true;
    }    

protected:
    virtual void emscriptenHeaderAddExtraIncludesImpl(GenStringsList& incs) const;
    virtual std::string emscriptenHeaderExtraCodePrefixImpl() const;
    virtual std::string emscriptenHeaderValueAccImpl() const;
    virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const;
    virtual std::string emscriptenSourceExtraCodeImpl() const;
    virtual std::string emscriptenSourceBindValueAccImpl() const;
    virtual std::string emscriptenSourceBindFuncsImpl() const;
    virtual std::string emscriptenSourceBindExtraImpl() const;

    void emscriptenAssignMembers(const commsdsl::gen::GenField::GenFieldsList& fields);
    void emscriptenAddMember(commsdsl::gen::GenField* field);
    std::string emscriptenHeaderValueAccByRef() const;
    std::string emscriptenHeaderValueAccByValue() const;
    std::string emscriptenHeaderValueAccLengthField() const;
    std::string emscriptenHeaderValueAccByPointer() const;
    std::string emscriptenHeaderValueStorageAccByPointer() const;
    std::string emscriptenSourceBindValueAcc() const;
    std::string emscriptenSourceBindValueAccByPointer() const;
    std::string emscriptenSourceBindValueStorageAccByPointer() const;

    const EmscriptenFieldsList& emscriptenMembers() const
    {
        return m_emscriptenMembers;
    }

    std::string emscriptenBindClassName(bool checkVersionOptional = true) const;
    std::string emscriptenMembersAccessFuncs() const;
    std::string emscriptenMembersBindFuncs() const;

private:
    bool emscriptenWriteHeaderInternal() const;
    bool emscriptenWriteSrcInternal() const;
    std::string emscriptenHeaderIncludesInternal() const;
    std::string emscriptenHeaderClassInternal() const;
    std::string emscriptenHeaderCommonPublicFuncsInternal() const;    
    std::string emscriptenSourceIncludesInternal() const;
    std::string emscriptenSourceBindInternal() const;
    std::string emscriptenSourceBindCommonInternal(bool skipVersionOptCheck = false) const;
    std::string emscriptenHeaderMembersInternal() const;
    std::string emscriptenSourceMembersInternal() const;
    std::string emscriptenSourceRegisterVectorInternal() const;
    std::string emscriptenSourceExtraVectorFuncsInternal() const;
    std::string emscriptenSourceExtraCodeInternal() const;

    commsdsl::gen::GenField& m_genField;
    EmscriptenFieldsList m_emscriptenMembers;
    bool m_listElement = false;
};

} // namespace commsdsl2emscripten
