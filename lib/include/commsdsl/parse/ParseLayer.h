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

#pragma once

#include <string>

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseInterface.h"

namespace commsdsl
{

namespace parse
{

class ParseLayerImpl;
class COMMSDSL_API ParseLayer
{
public:
    using AttributesMap = ParseField::AttributesMap;
    using ElementsList = ParseField::ElementsList;

    enum class Kind
    {
        Custom,
        Sync,
        Size,
        Id,
        Value,
        Payload,
        Checksum,
        NumOfValues
    };

    explicit ParseLayer(const ParseLayerImpl* impl);
    ParseLayer(const ParseLayer& other);
    ~ParseLayer();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    Kind kind() const;
    bool hasField() const;
    ParseField field() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const ParseLayerImpl* m_pImpl;
};

class ParseCustomLayerImpl;
class COMMSDSL_API ParseCustomLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    explicit ParseCustomLayer(const ParseCustomLayerImpl* impl);
    explicit ParseCustomLayer(ParseLayer layer);

    Kind semanticLayerType() const;
    const std::string& checksumFromLayer() const;
    const std::string& checksumUntilLayer() const;
};

class ParsePayloadLayerImpl;
class COMMSDSL_API ParsePayloadLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    explicit ParsePayloadLayer(const ParsePayloadLayerImpl* impl);
    explicit ParsePayloadLayer(ParseLayer layer);
};

class ParseIdLayerImpl;
class COMMSDSL_API ParseIdLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    explicit ParseIdLayer(const ParseIdLayerImpl* impl);
    explicit ParseIdLayer(ParseLayer layer);
};

class ParseSizeLayerImpl;
class COMMSDSL_API ParseSizeLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    explicit ParseSizeLayer(const ParseSizeLayerImpl* impl);
    explicit ParseSizeLayer(ParseLayer layer);
};

class ParseSyncLayerImpl;
class COMMSDSL_API ParseSyncLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    explicit ParseSyncLayer(const ParseSyncLayerImpl* impl);
    explicit ParseSyncLayer(ParseLayer layer);
};

class ParseChecksumLayerImpl;
class COMMSDSL_API ParseChecksumLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    enum class Alg
    {
        Custom,
        Sum,
        Crc_CCITT,
        Crc_16,
        Crc_32,
        Xor,
        NumOfValues
    };

    explicit ParseChecksumLayer(const ParseChecksumLayerImpl* impl);
    explicit ParseChecksumLayer(ParseLayer layer);

    Alg alg() const;
    const std::string& customAlgName() const;
    const std::string& fromLayer() const;
    const std::string& untilLayer() const;
    bool verifyBeforeRead() const;
};

class ParseValueLayerImpl;
class COMMSDSL_API ParseValueLayer : public ParseLayer
{
    using Base = ParseLayer;
public:
    using Interfaces = std::vector<ParseInterface>;

    explicit ParseValueLayer(const ParseValueLayerImpl* impl);
    explicit ParseValueLayer(ParseLayer layer);

    Interfaces interfaces() const;
    const std::string& fieldName() const;
    std::size_t fieldIdx() const;
    bool pseudo() const;
};


} // namespace parse

} // namespace commsdsl
