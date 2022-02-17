//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsDataField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2new
{

CommsDataField::CommsDataField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsDataField::prepareImpl()
{
    bool result = Base::prepareImpl() && commsPrepare();
    if (result) {
        auto* externalPrefix = externalPrefixField();
        m_commsExternalPrefixField = dynamic_cast<CommsField*>(externalPrefix);
        assert((m_commsExternalPrefixField != nullptr) || (externalPrefix == nullptr)); // Make sure dynamic cast is successful
        if (m_commsExternalPrefixField != nullptr) {
            m_commsExternalPrefixField->setReferenced();
        }

        auto* memberPrefix = memberPrefixField();
        m_commsMemberPrefixField = dynamic_cast<CommsField*>(memberPrefix);
        assert((m_commsMemberPrefixField != nullptr) || (memberPrefix == nullptr)); // Make sure dynamic cast is successful
    }
    return result;
}

bool CommsDataField::writeImpl() const
{
    return commsWrite();
}

CommsDataField::IncludesList CommsDataField::commsCommonIncludesImpl() const 
{
    if (m_commsMemberPrefixField == nullptr) {
        return IncludesList();
    }

    return m_commsMemberPrefixField->commsCommonIncludes();
}

std::string CommsDataField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsDataField::commsCommonMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::emptyString();
    }

    return m_commsMemberPrefixField->commsCommonCode();
}

CommsDataField::IncludesList CommsDataField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/ArrayList.h",
        "<cstdint>"
    };

    do {
        auto obj = dataDslObj();
        if (!obj.defaultValue().empty()) {
            result.insert(result.end(), {
                "comms/util/assign.h",
                "<iterator>"                
            });            
        }   

        if (m_commsMemberPrefixField != nullptr) {
            auto extraIncs = m_commsMemberPrefixField->commsDefIncludes();
            result.reserve(result.size() + extraIncs.size());
            std::move(extraIncs.begin(), extraIncs.end(), std::back_inserter(result));
            break;
        }

        if (m_commsExternalPrefixField != nullptr) {
            result.push_back(comms::relHeaderPathFor(m_commsExternalPrefixField->field(), generator()));
        }

        auto& detachedPrefixName = obj.detachedPrefixFieldName();
        if (!detachedPrefixName.empty()) {
            result.insert(result.end(), {
                "<algorithm>",
                "<limits>"
            });
        }
    } while (false);
    return result;
}

std::string CommsDataField::commsDefMembersCodeImpl() const
{
    if (m_commsMemberPrefixField == nullptr) {
        return strings::emptyString();
    }

    return m_commsMemberPrefixField->commsDefCode();
}

std::string CommsDataField::commsBaseClassDefImpl() const
{
    static const std::string Templ = 
    "comms::field::ArrayList<\n"
    "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "    std::uint8_t#^#COMMA#$#\n"
    "    #^#FIELD_OPTS#$#\n"
    ">";    

    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", generator().mainNamespace()},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsDataField::commsDefPublicCodeImpl() const
{
    auto obj = dataDslObj();
    auto& defaultValue = obj.defaultValue();
    if (defaultValue.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "/// @brief Default constructor\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$#()\n"
        "{\n"
        "    static const std::uint8_t Data[] = {\n"
        "        #^#BYTES#$#\n"
        "    };\n"
        "    comms::util::assign(Base::value(), std::begin(Data), std::end(Data));\n"
        "}\n";

    util::StringsList bytes;
    bytes.reserve(defaultValue.size());
    for (auto& b : defaultValue) {
        std::stringstream stream;
        stream << std::hex << "0x" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(b);
        bytes.push_back(stream.str());
    }

    std::string bytesStr = util::strListToString(bytes, ", ", "");
    bytesStr = util::strMakeMultiline(bytesStr);

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(obj.name())},
        {"BYTES", std::move(bytesStr)}
    };

    if (util::isFileReadable(comms::inputCodePathFor(*this, generator()) + strings::extendFileSuffixStr())) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }
    return util::processTemplate(Templ, repl);
}

std::string CommsDataField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    auto obj = dataDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return strings::emptyString();
    }

    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&detachedPrefixName](auto* f)
            {
                return f->field().dslObj().name() == detachedPrefixName;
            });

    if (iter == siblings.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return strings::emptyString();
    }

    bool lenVersionOptional = (*iter)->commsIsVersionOptional();

    static const std::string Templ = 
        "auto expectedLength = static_cast<std::size_t>(field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value());\n"
        "auto realLength = field_#^#NAME#$#()#^#STR_ACC#$#.value().size();\n"
        "if (expectedLength == realLength) {\n"
        "    return false;\n"
        "}\n\n"
        "using LenValueType = typename std::decay<decltype(field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value())>::type;\n"
        "static const auto MaxLenValue = static_cast<std::size_t>(std::numeric_limits<LenValueType>::max());\n"
        "auto maxAllowedLen = std::min(MaxLenValue, realLength);\n"
        "field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value() = static_cast<LenValueType>(maxAllowedLen);\n"
        "if (maxAllowedLen < realLength) {\n"
        "    field_#^#NAME#$#()#^#STR_ACC#$#.value().resize(maxAllowedLen);\n"
        "}\n"
        "return true;";

    util::ReplacementMap repl = {
        {"NAME", comms::className(dslObj().name())},
        {"LEN_NAME", comms::accessName(detachedPrefixName)}
    };

    if (commsIsVersionOptional()) {
        repl["STR_ACC"] = ".field()";
    }

    if (lenVersionOptional) {
        repl["LEN_ACC"] = ".field()";
    }
    
    return util::processTemplate(Templ, repl);
}

bool CommsDataField::commsIsLimitedCustomizableImpl() const
{
    return true;
}

bool CommsDataField::commsDoesRequireGeneratedReadRefreshImpl() const
{
    return !dataDslObj().detachedPrefixFieldName().empty();
}

std::string CommsDataField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddFixedLengthOptInternal(opts);
    commsAddLengthPrefixOptInternal(opts);
    commsAddLengthForcingOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

void CommsDataField::commsAddFixedLengthOptInternal(StringsList& opts) const
{
    auto obj = dataDslObj();
    auto fixedLen = obj.fixedLength();
    if (fixedLen == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        util::numToString(static_cast<std::uintmax_t>(fixedLen)) +
        ">";
    opts.push_back(std::move(str));
}

void CommsDataField::commsAddLengthPrefixOptInternal(StringsList& opts) const
{
    if ((m_commsExternalPrefixField == nullptr) && (m_commsMemberPrefixField == nullptr)) {
        return;
    }

    std::string prefixName;
    if (m_commsMemberPrefixField != nullptr) {
        prefixName = "typename " + comms::className(name()) + strings::membersSuffixStr();
        if (comms::isGlobalField(*this)) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + comms::className(m_commsMemberPrefixField->field().name());
    }
    else {
        assert(m_commsExternalPrefixField != nullptr);
        prefixName = comms::scopeFor(m_commsExternalPrefixField->field(), generator(), true, true);
        prefixName += "<TOpt> ";
    }

    opts.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void CommsDataField::commsAddLengthForcingOptInternal(StringsList& opts) const
{
    auto obj = dataDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return;
    }

    opts.push_back("comms::option::def::SequenceLengthForcingEnabled");
}

} // namespace commsdsl2new
