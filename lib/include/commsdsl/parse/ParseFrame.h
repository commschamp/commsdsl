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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseLayer.h"

#include <cstdint>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseFrameImpl;
class COMMSDSL_API ParseFrame
{
public:
    using LayersList = std::vector<ParseLayer>;
    using AttributesMap = ParseLayer::AttributesMap;
    using ElementsList = ParseLayer::ElementsList;

    explicit ParseFrame(const ParseFrameImpl* impl);
    ParseFrame(const ParseFrame& other);
    ~ParseFrame();

    bool parseValid() const;
    const std::string& parseName() const;
    const std::string& parseDescription() const;
    LayersList parseLayers() const;
    std::string parseExternalRef(bool schemaRef = true) const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;

protected:
    const ParseFrameImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
