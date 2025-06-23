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

#include "SwigSetField.h"

#include "SwigGenerator.h"
#include "SwigIntField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

SwigSetField::SwigSetField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigSetField::writeImpl() const
{
    return swigWrite();
}

std::string SwigSetField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = setDslObj();
    util::ReplacementMap repl = {
        {"TYPE", SwigGenerator::cast(generator()).swigConvertIntType(obj.type(), obj.maxLength())}
    };

    return util::processTemplate(Templ, repl);    
}

std::string SwigSetField::swigExtraPublicFuncsDeclImpl() const
{
    auto obj = setDslObj();

    util::StringsList indices;
    util::StringsList accesses;

    for (auto& bitInfo : obj.revBits()) {
        indices.push_back("BitIdx_" + bitInfo.second + " = " + std::to_string(bitInfo.first));

        static const std::string Templ = 
            "bool getBitValue_#^#NAME#$#() const;\n"
            "void setBitValue_#^#NAME#$#(bool val);";

        util::ReplacementMap repl = {
            {"NAME", bitInfo.second}
        };

        accesses.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "enum BitIdx : unsigned\n"
        "{\n"
        "    #^#INDICES#$#\n"
        "    BitIdx_numOfValues\n"
        "};\n\n"
        "bool getBitValue(unsigned bitNum) const;\n"
        "void setBitValue(unsigned bitNum, bool val);\n"        
        "#^#ACCESS_FUNCS#$#\n"
        ;    

    util::ReplacementMap repl = {
        {"INDICES", util::strListToString(indices, ",\n", ",")},
        {"ACCESS_FUNCS", util::strListToString(accesses, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
