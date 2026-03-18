//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "CommsSyncLayer.h"

#include "CommsField.h"
#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsSyncLayer::CommsSyncLayer(CommsGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsSyncLayer::genPrepareImpl()
{
    bool result = GenBase::genPrepareImpl() && CommsBase::commsPrepare();
    if (!result) {
        return false;
    }

    auto* extEscField = genExternalEscField();
    if (extEscField != nullptr) {
        extEscField->genSetReferenced();
    }

    commsSetForcedFailOnInvalidField();
    return true;
}

CommsSyncLayer::CommsIncludesList CommsSyncLayer::commsCommonIncludesImpl() const
{
    auto* memEscField = genMemberEscField();
    if (memEscField != nullptr) {
        return dynamic_cast<const CommsField*>(memEscField)->commsCommonIncludes();
    }

    return CommsIncludesList();
}

CommsSyncLayer::CommsIncludesList CommsSyncLayer::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/frame/SyncPrefixLayer.h"
    };

    auto parseObj = genSyncLayerDslObj();

    if (parseObj.parseIsAfterPayload()) {
        result.push_back("comms/frame/SyncSuffixLayer.h");
    }
    else {
        result.push_back("comms/frame/SyncPrefixLayer.h");
    }

    auto* extEscField = genExternalEscField();
    if (extEscField != nullptr) {
        result.push_back(comms::genRelHeaderPathFor(*extEscField, genGenerator()));
    }

    auto* memEscField = genMemberEscField();
    if (memEscField != nullptr) {
        auto extraIncs = dynamic_cast<const CommsField*>(memEscField)->commsDefIncludes();
        result.insert(result.end(), extraIncs.begin(), extraIncs.end());
    }

    if (parseObj.parseSeekField()) {
        result.push_back("comms/options.h");
    }

    return result;
}

std::string CommsSyncLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ =
        "comms::frame::Sync#^#TYPE#$#Layer<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    #^#PREV_LAYER#$##^#COMMA#$#\n"
        "    #^#OPTS#$#\n"
        ">";

    auto parseObj = genSyncLayerDslObj();
    std::string type = "Prefix";
    if (parseObj.parseIsAfterPayload()) {
        type = "Suffix";
    }

    util::GenStringsList opts;
    if (parseObj.parseSeekField()) {
        std::string escFieldOpt;
        do {
            auto* extEscField = genExternalEscField();
            if (extEscField != nullptr) {
                escFieldOpt = comms::genScopeFor(*extEscField, genGenerator()) + "<TOpt>";
                break;
            }

            auto* memEscField = genMemberEscField();
            if (memEscField != nullptr) {
                escFieldOpt = "typename " +
                comms::genClassName(parseObj.parseName()) + strings::genMembersSuffixStr() +
                "::" + comms::genClassName(memEscField->genParseObj().parseName());
                break;
            }

        } while (false);

        opts.push_back("comms::option::def::FrameLayerSeekField<" + escFieldOpt + ">");
    }

    if (parseObj.parseVerifyBeforeRead()) {
        opts.push_back("comms::option::def::FrameLayerVerifyBeforeRead");
    }

    util::GenReplacementMap repl = {
        {"FIELD_TYPE", commsDefFieldType()},
        {"PREV_LAYER", prevName},
        {"TYPE", std::move(type)},
        {"OPTS", util::genStrListToString(opts, ",\n", "")},
    };

    if (!opts.empty()) {
        repl["COMMA"] = ",";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsSyncLayer::commsExtraMemberFieldsDefsImpl() const
{
    auto* memEscField = dynamic_cast<const CommsField*>(genMemberEscField());
    if (memEscField == nullptr) {
        return strings::genEmptyString();
    }

    return memEscField->commsDefCode();
}

std::string CommsSyncLayer::commsExtraMemberFieldsCommonCodeImpl() const
{
    auto* memEscField = dynamic_cast<const CommsField*>(genMemberEscField());
    if (memEscField == nullptr) {
        return strings::genEmptyString();
    }

    return memEscField->commsCommonCode();
}

} // namespace commsdsl2comms
