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
#include "Field.h"
#include "Interface.h"

namespace commsdsl
{

namespace parse
{

class LayerImpl;
class COMMSDSL_API Layer
{
public:

    using AttributesMap = Field::AttributesMap;
    using ElementsList = Field::ElementsList;

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

    explicit Layer(const LayerImpl* impl);
    Layer(const Layer& other);
    ~Layer();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    Kind kind() const;
    bool hasField() const;
    Field field() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const LayerImpl* m_pImpl;
};

class CustomLayerImpl;
class COMMSDSL_API CustomLayer : public Layer
{
    using Base = Layer;
public:
    explicit CustomLayer(const CustomLayerImpl* impl);
    explicit CustomLayer(Layer layer);

    Kind semanticLayerType() const;
    const std::string& checksumFromLayer() const;
    const std::string& checksumUntilLayer() const;
};

class PayloadLayerImpl;
class COMMSDSL_API PayloadLayer : public Layer
{
    using Base = Layer;
public:
    explicit PayloadLayer(const PayloadLayerImpl* impl);
    explicit PayloadLayer(Layer layer);
};

class IdLayerImpl;
class COMMSDSL_API IdLayer : public Layer
{
    using Base = Layer;
public:
    explicit IdLayer(const IdLayerImpl* impl);
    explicit IdLayer(Layer layer);
};

class SizeLayerImpl;
class COMMSDSL_API SizeLayer : public Layer
{
    using Base = Layer;
public:
    explicit SizeLayer(const SizeLayerImpl* impl);
    explicit SizeLayer(Layer layer);
};

class SyncLayerImpl;
class COMMSDSL_API SyncLayer : public Layer
{
    using Base = Layer;
public:
    explicit SyncLayer(const SyncLayerImpl* impl);
    explicit SyncLayer(Layer layer);
};

class ChecksumLayerImpl;
class COMMSDSL_API ChecksumLayer : public Layer
{
    using Base = Layer;
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

    explicit ChecksumLayer(const ChecksumLayerImpl* impl);
    explicit ChecksumLayer(Layer layer);

    Alg alg() const;
    const std::string& customAlgName() const;
    const std::string& fromLayer() const;
    const std::string& untilLayer() const;
    bool verifyBeforeRead() const;
};

class ValueLayerImpl;
class COMMSDSL_API ValueLayer : public Layer
{
    using Base = Layer;
public:
    using Interfaces = std::vector<Interface>;

    explicit ValueLayer(const ValueLayerImpl* impl);
    explicit ValueLayer(Layer layer);

    Interfaces interfaces() const;
    const std::string& fieldName() const;
    std::size_t fieldIdx() const;
    bool pseudo() const;
};


} // namespace parse

} // namespace commsdsl
