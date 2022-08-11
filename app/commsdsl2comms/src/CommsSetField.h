//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/SetField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsSetField final : public commsdsl::gen::SetField, public CommsField
{
    using Base = commsdsl::gen::SetField;
    using CommsBase = CommsField;
public:
    CommsSetField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;
    virtual std::size_t commsMinLengthImpl() const override;
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;

private:
    std::string commsCommonBitNameFuncCodeInternal() const;
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefBitsAccessCodeInternal() const;
    std::string commsDefBitNameFuncCodeInternal() const;

    void commsAddLengthOptInternal(commsdsl::gen::util::StringsList& opts) const;
    void commsAddDefaultValueOptInternal(commsdsl::gen::util::StringsList& opts) const;
    void commsAddReservedBitsOptInternal(commsdsl::gen::util::StringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(StringsList& opts) const;
};

} // namespace commsdsl2comms
