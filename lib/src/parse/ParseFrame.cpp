//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseFrame.h"
#include <cassert>

#include "ParseFrameImpl.h"

namespace commsdsl
{

namespace parse
{

ParseFrame::ParseFrame(const ParseFrameImpl* impl)
  : m_pImpl(impl)
{
}

ParseFrame::ParseFrame(const ParseFrame &) = default;

ParseFrame::~ParseFrame() = default;

bool ParseFrame::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseFrame::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseFrame::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

ParseFrame::LayersList ParseFrame::layers() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->layersList();
}

std::string ParseFrame::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

const ParseFrame::AttributesMap& ParseFrame::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseFrame::ElementsList& ParseFrame::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
