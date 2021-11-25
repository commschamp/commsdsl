//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "LayerImpl.h"

namespace commsdsl
{

class ChecksumLayerImpl final : public LayerImpl
{
    using Base = LayerImpl;
public:
    using Alg = ChecksumLayer::Alg;
    ChecksumLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    Alg alg() const
    {
        return m_alg;
    }

    const std::string& algName() const
    {
        return *m_algName;
    }

    const std::string& from() const
    {
        return *m_from;
    }

    const std::string& until() const
    {
        return *m_until;
    }

    bool verifyBeforeRead() const
    {
        return m_verifyBeforeRead;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool verifyImpl(const LayersList& layers) override;

private:
    bool updateAlg();
    bool updateFrom();
    bool updateUntil();
    bool updateVerifyBeforeRead();

    Alg m_alg = Alg::NumOfValues;
    const std::string* m_algName = nullptr;
    const std::string* m_from = nullptr;
    const std::string* m_until = nullptr;
    bool m_verifyBeforeRead = false;
};

} // namespace commsdsl
