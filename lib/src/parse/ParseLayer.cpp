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

bool ParseLayer::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseLayer::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseLayer::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

ParseLayer::Kind ParseLayer::parseKind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseKind();
}

bool ParseLayer::parseHasField() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseHasField();
}

ParseField ParseLayer::parseField() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseField();
}

const ParseLayer::AttributesMap& ParseLayer::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseLayer::ElementsList& ParseLayer::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}

ParseCustomLayer::ParseCustomLayer(const ParseCustomLayerImpl* impl) :
    Base(impl)
{
}

ParseCustomLayer::ParseCustomLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Custom);
}

ParseLayer::Kind ParseCustomLayer::parseSemanticLayerType() const
{
    assert(parseValid());
    return asCustom(m_pImpl)->parseSemanticLayerType();
}

const std::string& ParseCustomLayer::parseChecksumFromLayer() const
{
    assert(parseValid());
    return asCustom(m_pImpl)->parseChecksumFromLayer();
}

const std::string& ParseCustomLayer::parseChecksumUntilLayer() const
{
    assert(parseValid());
    return asCustom(m_pImpl)->parseChecksumUntilLayer();
}

ParsePayloadLayer::ParsePayloadLayer(const ParsePayloadLayerImpl* impl) :
    Base(impl)
{
}

ParsePayloadLayer::ParsePayloadLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Payload);
}

ParseIdLayer::ParseIdLayer(const ParseIdLayerImpl* impl) :
    Base(impl)
{
}

ParseIdLayer::ParseIdLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Id);
}

ParseSizeLayer::ParseSizeLayer(const ParseSizeLayerImpl* impl) :
    Base(impl)
{
}

ParseSizeLayer::ParseSizeLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Size);
}

ParseSyncLayer::ParseSyncLayer(const ParseSyncLayerImpl* impl) :
    Base(impl)
{
}

ParseSyncLayer::ParseSyncLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Sync);
}

ParseChecksumLayer::ParseChecksumLayer(const ParseChecksumLayerImpl* impl) :
    Base(impl)
{
}

ParseChecksumLayer::ParseChecksumLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Checksum);
}

ParseChecksumLayer::Alg ParseChecksumLayer::parseAlg() const
{
    assert(parseValid());
    return asChecksum(m_pImpl)->parseAlg();
}

const std::string& ParseChecksumLayer::parseCustomAlgName() const
{
    assert(parseValid());
    return asChecksum(m_pImpl)->parseAlgName();
}

const std::string& ParseChecksumLayer::parseFromLayer() const
{
    assert(parseValid());
    return asChecksum(m_pImpl)->parseFrom();
}

const std::string& ParseChecksumLayer::parseUntilLayer() const
{
    assert(parseValid());
    return asChecksum(m_pImpl)->parseUntil();
}

bool ParseChecksumLayer::parseVerifyBeforeRead() const
{
    assert(parseValid());
    return asChecksum(m_pImpl)->parseVerifyBeforeRead();
}

ParseValueLayer::ParseValueLayer(const ParseValueLayerImpl* impl) :
    Base(impl)
{
}

ParseValueLayer::ParseValueLayer(ParseLayer layer) :
    Base(layer)
{
    assert(parseKind() == Kind::Value);
}

ParseValueLayer::Interfaces ParseValueLayer::parseInterfaces() const
{
    assert(parseValid());
    return asValue(m_pImpl)->parseInterfacesList();
}

const std::string& ParseValueLayer::parseFieldName() const
{
    assert(parseValid());
    return asValue(m_pImpl)->parseFieldName();
}

std::size_t ParseValueLayer::parseFieldIdx() const
{
    assert(parseValid());
    return asValue(m_pImpl)->parseFieldIdx();
}

bool ParseValueLayer::parsePseudo() const
{
    assert(parseValid());
    return asValue(m_pImpl)->parsePseudo();
}

} // namespace parse

} // namespace commsdsl
