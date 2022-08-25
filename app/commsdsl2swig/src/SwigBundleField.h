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

#include "SwigField.h"

#include "commsdsl/gen/BundleField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigBundleField final : public commsdsl::gen::BundleField, public SwigField
{
    using Base = commsdsl::gen::BundleField;
    using SwigBase = SwigField;
public:
    SwigBundleField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    const SwigFieldsList& swigMembers() const
    {
        return m_members;
    }

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // SwigBase overrides

private:
    bool prepareInternal();
    
    SwigFieldsList m_members;
};

} // namespace commsdsl2swig
