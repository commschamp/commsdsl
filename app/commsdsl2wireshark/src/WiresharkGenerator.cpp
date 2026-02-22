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

#include "WiresharkGenerator.h"

#include "Wireshark.h"
#include "WiresharkBitfieldField.h"
#include "WiresharkBundleField.h"
#include "WiresharkChecksumLayer.h"
#include "WiresharkCustomLayer.h"
#include "WiresharkDataField.h"
#include "WiresharkEnumField.h"
#include "WiresharkFloatField.h"
#include "WiresharkFrame.h"
#include "WiresharkIdLayer.h"
#include "WiresharkInterface.h"
#include "WiresharkIntField.h"
#include "WiresharkListField.h"
#include "WiresharkMessage.h"
#include "WiresharkNamespace.h"
#include "WiresharkOptionalField.h"
#include "WiresharkPayloadLayer.h"
#include "WiresharkRefField.h"
#include "WiresharkSchema.h"
#include "WiresharkSetField.h"
#include "WiresharkSizeLayer.h"
#include "WiresharkStringField.h"
#include "WiresharkSyncLayer.h"
#include "WiresharkValueLayer.h"
#include "WiresharkVariantField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkGenerator::WiresharkGenerator() = default;

WiresharkGenerator::GenSchemaPtr WiresharkGenerator::genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkSchema>(*this, parseObj, parent);
}

WiresharkGenerator::GenNamespacePtr WiresharkGenerator::genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkNamespace>(*this, parseObj, parent);
}

WiresharkGenerator::GenInterfacePtr WiresharkGenerator::genCreateInterfaceImpl(ParseInterface parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkInterface>(*this, parseObj, parent);
}

WiresharkGenerator::GenFramePtr WiresharkGenerator::genCreateFrameImpl(ParseFrame parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkFrame>(*this, parseObj, parent);
}

WiresharkGenerator::GenMessagePtr WiresharkGenerator::genCreateMessageImpl(ParseMessage parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkMessage>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateCustomLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkCustomLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateSyncLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkSyncLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateSizeLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkSizeLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateIdLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkIdLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateValueLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkValueLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreatePayloadLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkPayloadLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenLayerPtr WiresharkGenerator::genCreateChecksumLayerImpl(ParseLayer parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkChecksumLayer>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateIntFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkIntField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkEnumField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateSetFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkSetField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkFloatField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkBitfieldField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkBundleField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateStringFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkStringField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateDataFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkDataField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateListFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkListField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateRefFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkRefField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkOptionalField>(*this, parseObj, parent);
}

WiresharkGenerator::GenFieldPtr WiresharkGenerator::genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent)
{
    return std::make_unique<WiresharkVariantField>(*this, parseObj, parent);
}

bool WiresharkGenerator::genWriteImpl()
{
    assert(&genCurrentSchema() == &genProtocolSchema());
    return
        Wireshark::wiresharkWrite(*this) &&
        wiresharkWriteExtraFilesInternal();
}

const std::string& WiresharkGenerator::wiresharkFileGeneratedComment()
{
    static const std::string Str =
        "-- Generated by commsdsl2wireshark " + genVersionStr() + '\n';
    return Str;
}

std::string WiresharkGenerator::wiresharkScopeToName(const std::string& scope) const
{
    return util::genStrReplace(scope, "::", "_");
}

std::string WiresharkGenerator::wiresharkFuncNameFor(const GenElem& elem, const std::string& suffix) const
{
    auto scope = comms::genScopeFor(elem, *this, false);
    auto protName = Wireshark::wiresharkProtocolObjName(*this);
    return protName + '_' + wiresharkScopeToName(scope) + suffix;
}

std::string WiresharkGenerator::wiresharkDissectNameFor(const GenElem& elem) const
{
    return wiresharkFuncNameFor(elem, strings::genReadSuffixStr());
}

std::string WiresharkGenerator::wiresharkInputRelPathPrefix() const
{
    return Wireshark::wiresharkFileName(*this) + '-';
}

std::string WiresharkGenerator::wiresharkInputDissectRelPathFor(const GenElem& elem) const
{
    return wiresharkInputRelPathPrefix() + wiresharkDissectNameFor(elem);
}

std::string WiresharkGenerator::wiresharkInputDissectAbsPathFor(const GenElem& elem) const
{
    return genGetCodeDir() + '/' + wiresharkInputDissectRelPathFor(elem);
}

std::string WiresharkGenerator::wiresharkInputDissectRelPathFor(const std::string& name) const
{
    return wiresharkInputRelPathPrefix() + name;
}

std::string WiresharkGenerator::wiresharkInputDissectAbsPathFor(const std::string& name) const
{
    return genGetCodeDir() + '/' + wiresharkInputDissectRelPathFor(name);
}

std::string WiresharkGenerator::wiresharkInputRelPathFor(const GenElem& elem, const std::string& suffix) const
{
    return wiresharkInputRelPathPrefix() + wiresharkFuncNameFor(elem, suffix);
}

std::string WiresharkGenerator::wiresharkInputAbsPathFor(const GenElem& elem, const std::string& suffix) const
{
    return genGetCodeDir() + '/' + wiresharkInputRelPathFor(elem, suffix);
}

const std::string& WiresharkGenerator::genCommentPrefixImpl() const
{
    static const std::string Str("-- ");
    return Str;
}

bool WiresharkGenerator::wiresharkWriteExtraFilesInternal() const
{
    const std::vector<std::string> ReservedExt = {
        strings::genAppendFileSuffixStr(),
        strings::genExtendFileSuffixStr(),
        strings::genPrependFileSuffixStr(),
        strings::genReplaceFileSuffixStr(),
    };

    return genCopyExtraSourceFiles(ReservedExt);
}

} // namespace commsdsl2wireshark
