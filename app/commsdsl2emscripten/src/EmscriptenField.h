//
// Copyright 2022 - 2026 (C). Alex Robenko. All rights reserved.
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

    [[nodiscard]] static const EmscriptenField* emscriptenCast(const commsdsl::gen::GenField* field);
    [[nodiscard]] static EmscriptenField* emscriptenCast(commsdsl::gen::GenField* field);
    [[nodiscard]] static EmscriptenFieldsList emscriptenTransformFieldsList(const GenFieldsList& fields);

    [[nodiscard]] commsdsl::gen::GenField& emscriptenGenField()
    {
        return m_genField;
    }

    [[nodiscard]] const commsdsl::gen::GenField& emscriptenGenField() const
    {
        return m_genField;
    }

    [[nodiscard]] std::string emscriptenRelHeaderPath() const;

    [[nodiscard]] bool emscriptenIsVersionOptional() const;
    [[nodiscard]] bool emscriptenWrite() const;

    [[nodiscard]] std::string emscriptenHeaderClass() const;
    [[nodiscard]] std::string emscriptenTemplateScope() const;
    [[nodiscard]] std::string emscriptenSourceCode() const;

    void emscriptenHeaderAddExtraIncludes(GenStringsList& incs) const;
    void emscriptenAddSourceFiles(GenStringsList& sources) const;

    void emscriptenSetListElement()
    {
        m_listElement = true;
    }

protected:
    virtual void emscriptenHeaderAddExtraIncludesImpl(GenStringsList& incs) const;
    [[nodiscard]] virtual std::string emscriptenHeaderExtraCodePrefixImpl() const;
    [[nodiscard]] virtual std::string emscriptenHeaderValueAccImpl() const;
    [[nodiscard]] virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceExtraCodeImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceBindValueAccImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceBindFuncsImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceBindExtraImpl() const;

    void emscriptenAssignMembers(const commsdsl::gen::GenField::GenFieldsList& fields);
    void emscriptenAddMember(commsdsl::gen::GenField* field);
    [[nodiscard]] std::string emscriptenHeaderValueAccByRef() const;
    [[nodiscard]] std::string emscriptenHeaderValueAccByValue() const;
    [[nodiscard]] std::string emscriptenHeaderValueAccLengthField() const;
    [[nodiscard]] std::string emscriptenHeaderValueAccByPointer() const;
    [[nodiscard]] std::string emscriptenHeaderValueStorageAccByPointer() const;
    [[nodiscard]] std::string emscriptenSourceBindValueAcc() const;
    [[nodiscard]] std::string emscriptenSourceBindValueAccByPointer() const;
    [[nodiscard]] std::string emscriptenSourceBindValueStorageAccByPointer() const;

    const EmscriptenFieldsList& emscriptenMembers() const
    {
        return m_emscriptenMembers;
    }

    [[nodiscard]] std::string emscriptenBindClassName(bool checkVersionOptional = true) const;
    [[nodiscard]] std::string emscriptenMembersAccessFuncs() const;
    [[nodiscard]] std::string emscriptenMembersBindFuncs() const;

private:
    [[nodiscard]] bool emscriptenWriteHeaderInternal() const;
    [[nodiscard]] bool emscriptenWriteSrcInternal() const;
    [[nodiscard]] std::string emscriptenHeaderIncludesInternal() const;
    [[nodiscard]] std::string emscriptenHeaderClassInternal() const;
    [[nodiscard]] std::string emscriptenHeaderCommonPublicFuncsInternal() const;
    [[nodiscard]] std::string emscriptenSourceIncludesInternal() const;
    [[nodiscard]] std::string emscriptenSourceBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceBindCommonInternal(bool skipVersionOptCheck = false) const;
    [[nodiscard]] std::string emscriptenHeaderMembersInternal() const;
    [[nodiscard]] std::string emscriptenSourceMembersInternal() const;
    [[nodiscard]] std::string emscriptenSourceRegisterVectorInternal() const;
    [[nodiscard]] std::string emscriptenSourceExtraVectorFuncsInternal() const;
    [[nodiscard]] std::string emscriptenSourceExtraCodeInternal() const;

    commsdsl::gen::GenField& m_genField;
    EmscriptenFieldsList m_emscriptenMembers;
    bool m_listElement = false;
};

} // namespace commsdsl2emscripten
