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

#include "commsdsl/Frame.h"
#include <cassert>

#include "FrameImpl.h"

namespace commsdsl
{

Frame::Frame(const FrameImpl* impl)
  : m_pImpl(impl)
{
}

Frame::Frame(const Frame &) = default;

Frame::~Frame() = default;

bool Frame::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Frame::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Frame::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Frame::LayersList Frame::layers() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->layersList();
}

std::string Frame::externalRef() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef();
}

const Frame::AttributesMap& Frame::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Frame::ElementsList& Frame::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace commsdsl
