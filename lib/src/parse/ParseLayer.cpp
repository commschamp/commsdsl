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

#include "commsdsl/parse/ParseLayer.h"
#include <cassert>

#include "ParseLayerImpl.h"
#include "ParsePayloadLayerImpl.h"
#include "ParseIdLayerImpl.h"
#include "ParseSizeLayerImpl.h"
#include "ParseSyncLayerImpl.h"
#include "ParseChecksumLayerImpl.h"
#include "ParseValueLayerImpl.h"
#include "ParseCustomLayerImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseChecksumLayerImpl* asChecksum(const ParseLayerImpl* layer)
{
    assert(layer != nullptr);
    return static_cast<const ParseChecksumLayerImpl*>(layer);
}

const ParseValueLayerImpl* asValue(const ParseLayerImpl* layer)
{
    assert(layer != nullptr);
    return static_cast<const ParseValueLayerImpl*>(layer);
}

const ParseCustomLayerImpl* asCustom(const ParseLayerImpl* layer)
{
    assert(layer != nullptr);
    return static_cast<const ParseCustomLayerImpl*>(layer);
}

} // namespace

ParseLayer::ParseLayer(const ParseLayerImpl* impl) :
    m_pImpl(impl)
{
}

ParseLayer::ParseLayer(const ParseLayer &) = default;

ParseLayer::~ParseLayer() = default;

bool ParseLayer::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseLayer::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseLayer::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

ParseLayer::Kind ParseLayer::kind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->kind();
}

bool ParseLayer::hasField() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->hasField();
}

ParseField ParseLayer::field() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->field();
}

const ParseLayer::AttributesMap& ParseLayer::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseLayer::ElementsList& ParseLayer::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

ParseCustomLayer::ParseCustomLayer(const ParseCustomLayerImpl* impl) :
    Base(impl)
{
}

ParseCustomLayer::ParseCustomLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Custom);
}

ParseLayer::Kind ParseCustomLayer::semanticLayerType() const
{
    assert(valid());
    return asCustom(m_pImpl)->semanticLayerType();
}

const std::string& ParseCustomLayer::checksumFromLayer() const
{
    assert(valid());
    return asCustom(m_pImpl)->checksumFromLayer();
}

const std::string& ParseCustomLayer::checksumUntilLayer() const
{
    assert(valid());
    return asCustom(m_pImpl)->checksumUntilLayer();
}

ParsePayloadLayer::ParsePayloadLayer(const ParsePayloadLayerImpl* impl) :
    Base(impl)
{
}

ParsePayloadLayer::ParsePayloadLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Payload);
}

ParseIdLayer::ParseIdLayer(const ParseIdLayerImpl* impl) :
    Base(impl)
{
}

ParseIdLayer::ParseIdLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Id);
}

ParseSizeLayer::ParseSizeLayer(const ParseSizeLayerImpl* impl) :
    Base(impl)
{
}

ParseSizeLayer::ParseSizeLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Size);
}

ParseSyncLayer::ParseSyncLayer(const ParseSyncLayerImpl* impl) :
    Base(impl)
{
}

ParseSyncLayer::ParseSyncLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Sync);
}

ParseChecksumLayer::ParseChecksumLayer(const ParseChecksumLayerImpl* impl) :
    Base(impl)
{
}

ParseChecksumLayer::ParseChecksumLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Checksum);
}

ParseChecksumLayer::Alg ParseChecksumLayer::alg() const
{
    assert(valid());
    return asChecksum(m_pImpl)->alg();
}

const std::string& ParseChecksumLayer::customAlgName() const
{
    assert(valid());
    return asChecksum(m_pImpl)->algName();
}

const std::string& ParseChecksumLayer::fromLayer() const
{
    assert(valid());
    return asChecksum(m_pImpl)->from();
}

const std::string& ParseChecksumLayer::untilLayer() const
{
    assert(valid());
    return asChecksum(m_pImpl)->until();
}

bool ParseChecksumLayer::verifyBeforeRead() const
{
    assert(valid());
    return asChecksum(m_pImpl)->verifyBeforeRead();
}

ParseValueLayer::ParseValueLayer(const ParseValueLayerImpl* impl) :
    Base(impl)
{
}

ParseValueLayer::ParseValueLayer(ParseLayer layer) :
    Base(layer)
{
    assert(kind() == Kind::Value);
}

ParseValueLayer::Interfaces ParseValueLayer::interfaces() const
{
    assert(valid());
    return asValue(m_pImpl)->interfacesList();
}

const std::string& ParseValueLayer::fieldName() const
{
    assert(valid());
    return asValue(m_pImpl)->fieldName();
}

std::size_t ParseValueLayer::fieldIdx() const
{
    assert(valid());
    return asValue(m_pImpl)->fieldIdx();
}

bool ParseValueLayer::pseudo() const
{
    assert(valid());
    return asValue(m_pImpl)->pseudo();
}

} // namespace parse

} // namespace commsdsl
