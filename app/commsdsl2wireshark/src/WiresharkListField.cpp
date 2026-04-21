//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkListField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

WiresharkListField::WiresharkListField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this)),
    m_wiresharkFields(static_cast<std::size_t>(WiresharkFieldIdx_ValuesLimit), nullptr)
{
}

bool WiresharkListField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    auto castField =
        [](commsdsl::gen::GenField* field) -> WiresharkField*
        {
            if (field == nullptr) {
                return nullptr;
            }

            auto* wiresharkField = WiresharkField::wiresharkCast(field);
            assert(wiresharkField != nullptr);
            return wiresharkField;
        };

    m_wiresharkFields[WiresharkFieldIdx_ExternalElement] = castField(genExternalElementField());
    m_wiresharkFields[WiresharkFieldIdx_MemberElement] = castField(genMemberElementField());
    m_wiresharkFields[WiresharkFieldIdx_ExternalCountPrefix] = castField(genExternalCountPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_MemberCountPrefix] = castField(genMemberCountPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_ExternalLengthPrefix] = castField(genExternalLengthPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_MemberLenthPrefix] = castField(genMemberLengthPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_ExternalElemLengthPrefix] = castField(genExternalElemLengthPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_MemberElemLenthPrefix] = castField(genMemberElemLengthPrefixField());
    m_wiresharkFields[WiresharkFieldIdx_ExternalTermSuffix] = castField(genExternalTermSuffixField());
    m_wiresharkFields[WiresharkFieldIdx_MemberTermSuffix] = castField(genMemberTermSuffixField());

    auto parseObj = genListFieldParseObj();
    if (!parseObj.parseDetachedTermSuffixFieldName().empty()) {
        genGenerator().genLogger().genError(
            "Detached termination suffix is currently not supported, "
            "please contact the developer and request this feature");
        return false;
    }

    if (parseObj.parseIsFailOnInvalid()) {
        genGenerator().genLogger().genError(
            "Using \"failOnInvalid()\" property for the whole <list> field is not supported, "
            "set this property on the element object instead of the whole <list>");
        return false;
    }

    return true;
}

std::string WiresharkListField::wiresharkDissectCodeImpl(const WiresharkField* refField) const
{
    assert(refField == nullptr);

    static const std::string Templ =
        "#^#NAME#$#_size_rec = {}\n"
        "\n"
        "function #^#NAME#$#_size_rec_set(field, value)\n"
        "    #^#NAME#$#_size_rec[field] = value\n"
        "end\n"
        "\n"
        "function #^#NAME#$#_size_rec_get(field)\n"
        "    local value = #^#NAME#$#_size_rec[field]\n"
        "    if value == #^#NIL#$# then\n"
        "        return 0\n"
        "    end\n"
        "\n"
        "    return value\n"
        "end\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName(refField)},
        {"NIL", strings::genNilStr()},
    };

    return util::genProcessTemplate(Templ, repl) + WiresharkBase::wiresharkDissectCodeImpl(refField);
}

std::string WiresharkListField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const WiresharkFieldIdx Members[] = {
        WiresharkFieldIdx_MemberElement,
        WiresharkFieldIdx_MemberCountPrefix,
        WiresharkFieldIdx_MemberLenthPrefix,
        WiresharkFieldIdx_MemberElemLenthPrefix,
        WiresharkFieldIdx_MemberTermSuffix,
    };

    util::GenStringsList elems;
    for (auto m : Members) {
        auto idx = static_cast<unsigned>(m);
        assert(idx < m_wiresharkFields.size());
        auto* memField = m_wiresharkFields[idx];
        if (memField == nullptr) {
            continue;
        }

        auto str = memField->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "", "") + WiresharkBase::wiresharkExtractorsRegCodeImpl(refField);
}

std::string WiresharkListField::wiresharkMembersDissectCodeImpl() const
{
    util::GenStringsList elems;

    auto addDisectCodeFunc =
        [&elems](auto* field)
        {
            if (field != nullptr) {
                elems.push_back(WiresharkField::wiresharkCast(field)->wiresharkDissectCode());
            }
        };

    addDisectCodeFunc(genMemberElementField());
    addDisectCodeFunc(genMemberCountPrefixField());
    addDisectCodeFunc(genMemberLengthPrefixField());
    addDisectCodeFunc(genMemberElemLengthPrefixField());
    addDisectCodeFunc(genMemberTermSuffixField());

    genGenerator().genLogger().genDebug("There are " + std::to_string(elems.size()) + " member elements of the " + genParseObj().parseInnerRef() + " list field");
    if (elems.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(elems, "\n", "");
}

std::string WiresharkListField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);

    static const std::string Templ =
        "local count = 0\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add(#^#FIELD#$#, #^#TVB#$#(#^#NEXT_OFFSET#$#, 0))\n"
        "#^#READ#$#\n"
        "#^#NAME#$#_size_rec_set(#^#FIELD#$#, count)\n"
        "#^#SUBTREE#$#:set_len(#^#NEXT_OFFSET#$# - #^#OFFSET#$#)\n"
        ;

    util::GenReplacementMap repl {
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"FIELD", wiresharkFieldStr()},
        {"TVB", wiresharkTvbStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"NAME", wiresharkFieldObjName()},
        {"TREE", wiresharkTreeStr()},
    };

    auto parseObj = genListFieldParseObj();
    do {
        if (parseObj.parseFixedCount() != 0) {
            repl["READ"] = wiresharkFixedCountDissectInternal();
            break;
        }

        if (parseObj.parseHasCountPrefixField()) {
            repl["READ"] = wiresharkCountPrefixDissectInternal();
            break;
        }

        repl["READ"] = "-- TODO: implement";
    } while (false);

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkListField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        if ((f == nullptr) || (f->wiresharkHasTrivialValid())) {
            continue;
        }

        static const std::string FieldTempl =
            "if not #^#VALID_FUNC#$#(#^#FIELD#$#) then\n"
            "    return false\n"
            "end\n"
            ;

        util::GenReplacementMap fieldRepl = {
            {"VALID_FUNC", f->wiresharkValidFuncName()},
            {"FIELD", f->wiresharkFieldObjName()},
        };

        elems.push_back(util::genProcessTemplate(FieldTempl, fieldRepl));
    }

    elems.push_back("return true");
    return util::genStrListToString(elems, "\n", "");
}

std::string WiresharkListField::wiresharkSizeAccessStrImpl(const std::string& accStr, [[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    if (!accStr.empty()) {
        auto modifiedMembers = m_wiresharkFields;
        modifiedMembers.erase(
            std::remove(modifiedMembers.begin(), modifiedMembers.end(), nullptr),
            modifiedMembers.end());

        auto memInfo = wiresharkSplitMemberAccStr(accStr, modifiedMembers);
        if (memInfo.first == nullptr) {
            genGenerator().genLogger().genError("BUG: Unexpected access string \"" + accStr + "\" for " + genParseObj().parseInnerRef());
            return strings::genUnexpectedValueStr();
        }

        return memInfo.first->wiresharkSizeAccessStr(memInfo.second);
    }

    static const std::string Templ =
        "#^#NAME#$#_size_rec_get(#^#NAME#$#)"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName()}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkListField::wiresharkHasTrivialValidImpl() const
{
    return
        std::all_of(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [](auto* f)
            {
                return (f == nullptr) || f->wiresharkHasTrivialValid();
            });
}

std::string WiresharkListField::wiresharkDissectElemCodeInternal() const
{
    assert(WiresharkFieldIdx_ValuesLimit <= m_wiresharkFields.size());

    auto* elemField = m_wiresharkFields[WiresharkFieldIdx_ExternalElement];
    if (elemField == nullptr) {
        elemField = m_wiresharkFields[WiresharkFieldIdx_MemberElement];
    }

    assert(elemField != nullptr);

    static const std::string Templ =
        "#^#ELEM_LIMIT#$#\n"
        "#^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#NEXT_OFFSET#$#, elem_limit, #^#FIELD#$#)\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#, #^#OFFSET#$#\n"
        "end\n"
        "count = count + 1\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"DISSECT", elemField->wiresharkDissectName()},
        {"TVB", wiresharkTvbStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"FIELD", elemField->wiresharkFieldObjName()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"OFFSET", wiresharkOffsetStr()},
        {"ELEM_LIMIT", wiresharkElemLimitCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkListField::wiresharkFixedCountDissectInternal() const
{
    auto parseObj = genListFieldParseObj();
    auto cnt = parseObj.parseFixedCount();
    assert(cnt != 0U);

    static const std::string Templ =
        "for i = 1, #^#CNT#$#, 1\n"
        "do\n"
        "    #^#READ_ELEM#$#\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"CNT", std::to_string(cnt)},
        {"READ_ELEM", wiresharkDissectElemCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkListField::wiresharkCountPrefixDissectInternal() const
{
    auto* prefixField = m_wiresharkFields[WiresharkFieldIdx_ExternalCountPrefix];
    if (prefixField == nullptr) {
        prefixField = m_wiresharkFields[WiresharkFieldIdx_MemberCountPrefix];
    }

    assert(prefixField != nullptr);

    static const std::string Templ =
        "#^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#NEXT_OFFSET#$#, #^#LIMIT#$#, #^#FIELD#$#)\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#, #^#OFFSET#$#\n"
        "end\n"
        "\n"
        "for i = 1, #^#VALUE_FUNC#$#(#^#FIELD#$#), 1\n"
        "do\n"
        "    #^#READ_ELEM#$#\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"DISSECT", prefixField->wiresharkDissectName()},
        {"TVB", wiresharkTvbStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"FIELD", prefixField->wiresharkFieldObjName()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"OFFSET", wiresharkOffsetStr()},
        {"VALUE_FUNC", wiresharkValueFuncName()},
        {"READ_ELEM", wiresharkDissectElemCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkListField::wiresharkElemLimitCodeInternal() const
{
    auto parseObj = genListFieldParseObj();
    if (!parseObj.parseHasElemLengthPrefixField()) {
        return "local elem_limit = " + wiresharkOffsetLimitStr();
    }

    auto* lenField = m_wiresharkFields[WiresharkFieldIdx_ExternalElemLengthPrefix];
    if (lenField == nullptr) {
        lenField = m_wiresharkFields[WiresharkFieldIdx_MemberElemLenthPrefix];
    }

    assert(lenField != nullptr);

    static const std::string ReadTempl =
        "#^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#NEXT_OFFSET#$#, #^#LIMIT#$#, #^#FIELD#$#)"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#, #^#OFFSET#$#\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap readRepl = {
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"DISSECT", lenField->wiresharkDissectName()},
        {"TVB", wiresharkTvbStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"FIELD", lenField->wiresharkFieldObjName()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"OFFSET", wiresharkOffsetStr()},
    };

    static const std::string ValueTempl =
        "local elem_limit = #^#VALUE_FUNC#$#(#^#FIELD#$#)"
        ;

    util::GenReplacementMap valueRepl = {
        {"VALUE_FUNC", lenField->wiresharkValueFuncName()},
        {"FIELD", lenField->wiresharkFieldObjName()},
    };

    if (!parseObj.parseElemFixedLength()) {
        return util::genProcessTemplate(ReadTempl, readRepl) + '\n' + util::genProcessTemplate(ValueTempl, valueRepl);
    }

    if ((parseObj.parseFixedCount()) ||
        (parseObj.parseHasCountPrefixField())) {
        static const std::string FixedCountTempl =
            "if cnt == 1 then\n"
            "   #^#READ#$#\n"
            "end\n"
            "#^#VALUE#$#\n"
            ;

        util::GenReplacementMap fixedCountRepl = {
            {"READ", util::genProcessTemplate(ReadTempl, readRepl)},
            {"VALUE", util::genProcessTemplate(ValueTempl, valueRepl)},
        };

        return util::genProcessTemplate(FixedCountTempl, fixedCountRepl);
    }

    // TODO:
    return "-- TODO: not implemented";
}

} // namespace commsdsl2wireshark
