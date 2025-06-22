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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseBundleField.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class BundleFieldImpl;
class COMMSDSL_API BundleField : public Field
{
    using Base = Field;
public:

    BundleField(Generator& generator, commsdsl::parse::ParseField dslObj, Elem* parent = nullptr);
    virtual ~BundleField();

    const FieldsList& members() const;

protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override final;
    
    commsdsl::parse::ParseBundleField bundleDslObj() const;

private:
    std::unique_ptr<BundleFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
