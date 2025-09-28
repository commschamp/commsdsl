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

#include "commsdsl/gen/strings.h"

namespace commsdsl
{

namespace gen
{

namespace strings
{

const std::string& genEmptyString()
{
    static const std::string Str;
    return Str;
}

const std::string& genMsgIdEnumNameStr()
{
    static const std::string Str("MsgId");
    return Str;
}

const std::string& genCmakeListsFileStr()
{
    static const std::string Str("CMakeLists.txt");
    return Str;
}

const std::string& genDefaultOptionsStr()
{
    static const std::string Str("DefaultOptions");
    return Str;
}

const std::string& genAllMessagesStr()
{
    static const std::string Str("AllMessages");
    return Str;
}

const std::string& genMessageClassStr()
{
    static const std::string Str("Message");
    return Str;
}

const std::string& genCommonSuffixStr()
{
    static const std::string Str("Common");
    return Str;
}

const std::string& genMembersSuffixStr()
{
    static const std::string Str("Members");
    return Str;
}

const std::string& genCppHeaderSuffixStr()
{
    static const std::string Str(".h");
    return Str;
}

const std::string& genCppSourceSuffixStr()
{
    static const std::string Str(".cpp");
    return Str;
}

const std::string& genLatexSuffixStr()
{
    static const std::string Str(".tex");
    return Str;
}

const std::string& genFieldNamespaceStr()
{
    static const std::string Str("field");
    return Str;
}

const std::string& genMessageNamespaceStr()
{
    static const std::string Str("message");
    return Str;
}

const std::string& genFrameNamespaceStr()
{
    static const std::string Str("frame");
    return Str;
}

const std::string& genLayerNamespaceStr()
{
    static const std::string Str("layer");
    return Str;
}

const std::string& genChecksumNamespaceStr()
{
    static const std::string Str("checksum");
    return Str;
}

const std::string& genOptionsNamespaceStr()
{
    static const std::string Str("options");
    return Str;
}

const std::string& genCispatchNamespaceStr()
{
    static const std::string Str("dispatch");
    return Str;
}

const std::string& genFactoryNamespaceStr()
{
    static const std::string Str("factory");
    return Str;
}

const std::string& genInputNamespaceStr()
{
    static const std::string Str("input");
    return Str;
}

const std::string& genPluginNamespaceStr()
{
    static const std::string Str("plugin");
    return Str;
}

const std::string& genIncludeDirStr()
{
    static const std::string Str("include");
    return Str;
}

const std::string& genSrcDirStr()
{
    static const std::string Str("src");
    return Str;
}

const std::string& genDocDirStr()
{
    static const std::string Str("doc");
    return Str;
}

const std::string& genNameFileSuffixStr()
{
    static const std::string Str(".name");
    return Str;
}

const std::string& genNameBodyFileSuffixStr()
{
    static const std::string Str = genNameFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genValueFileSuffixStr()
{
    static const std::string Str(".value");
    return Str;
}

const std::string& genReadFileSuffixStr()
{
    static const std::string Str(".read");
    return Str;
}

const std::string& genReadBodyFileSuffixStr()
{
    static const std::string Str = genReadFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genRefreshFileSuffixStr()
{
    static const std::string Str(".refresh");
    return Str;
}

const std::string& genRefreshBodyFileSuffixStr()
{
    static const std::string Str = genRefreshFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genWriteFileSuffixStr()
{
    static const std::string Str(".write");
    return Str;
}

const std::string& genWriteBodyFileSuffixStr()
{
    static const std::string Str = genWriteFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genPublicFileSuffixStr()
{
    static const std::string Str(".public");
    return Str;
}

const std::string& genProtectedFileSuffixStr()
{
    static const std::string Str(".protected");
    return Str;
}

const std::string& genPrivateFileSuffixStr()
{
    static const std::string Str(".private");
    return Str;
}

const std::string& genIncFileSuffixStr()
{
    static const std::string Str(".inc");
    return Str;
}

const std::string& genLengthFileSuffixStr()
{
    static const std::string Str(".length");
    return Str;
}

const std::string& genLengthBodyFileSuffixStr()
{
    static const std::string Str = genLengthFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genValidFileSuffixStr()
{
    static const std::string Str(".valid");
    return Str;
}

const std::string& genValidBodyFileSuffixStr()
{
    static const std::string Str = genValidFileSuffixStr() + genBodyFileSuffixStr();
    return Str;
}

const std::string& genReplaceFileSuffixStr()
{
    static const std::string Str(".replace");
    return Str;
}

const std::string& genExtendFileSuffixStr()
{
    static const std::string Str(".extend");
    return Str;
}

const std::string& genAppendFileSuffixStr()
{
    static const std::string Str(".append");
    return Str;
}

const std::string& genPrependFileSuffixStr()
{
    static const std::string Str(".prepend");
    return Str;
}

const std::string& genPrependLangFileSuffixStr()
{
    static const std::string Str(".prepend_lang");
    return Str;
}

const std::string& genBindFileSuffixStr()
{
    static const std::string Str(".bind");
    return Str;
}

const std::string& genConstructFileSuffixStr()
{
    static const std::string Str(".construct");
    return Str;
}

const std::string& genSourcesFileSuffixStr()
{
    static const std::string Str(".sources");
    return Str;
}

const std::string& genForceEmptyDisplayNameStr()
{
    static const std::string Str("_");
    return Str;
}

const std::string& genFieldBaseClassStr()
{
    static const std::string Str("FieldBase");
    return Str;
}

const std::string& genDefaultOptionsClassStr()
{
    static const std::string Str("DefaultOptions");
    return Str;
}

const std::string& genAllMessagesDynMemMsgFactoryDefaultOptionsClassStr()
{
    static const std::string Str("AllMessagesDynMemMsgFactoryDefaultOptions");
    return Str;
}

const std::string& genIndentStr()
{
    static const std::string Str("    ");
    return Str;
}

const std::string& genDoxygenPrefixStr()
{
    static const std::string Str("/// ");
    return Str;
}

const std::string& genVersionOptionalFieldSuffixStr()
{
    static const std::string Str("Field");
    return Str;
}

const std::string& genOrigSuffixStr()
{
    static const std::string Str("Orig");
    return Str;
}

const std::string& genMsgIdPrefixStr()
{
    static const std::string Str("MsgId_");
    return Str;
}

const std::string& genFieldsSuffixStr()
{
    static const std::string Str("Fields");
    return Str;
}

const std::string& genLayersSuffixStr()
{
    static const std::string Str("Layers");
    return Str;
}

const std::string& genBareMetalStr()
{
    static const std::string Str("BareMetal");
    return Str;
}

const std::string& genDataViewStr()
{
    static const std::string Str("DataView");
    return Str;
}

const std::string& genTransportMessageSuffixStr()
{
    static const std::string Str("TransportMessage");
    return Str;
}

const std::string& genUnexpectedValueStr()
{
    static const std::string Str("???");
    return Str;
}

const std::string& genVersionFileNameStr()
{
    static const std::string Str("Version");
    return Str;
}

const std::string& genEnumFirstValueStr()
{
    static const std::string Str("FirstValue");
    return Str;
}

const std::string& genEnumLastValueStr()
{
    static const std::string Str("LastValue");
    return Str;
}

const std::string& genEnumValuesLimitStr()
{
    static const std::string Str("ValuesLimit");
    return Str;
}

const std::string& genTransportFieldAccessPrefixStr()
{
    static const std::string Str("transportField_");
    return Str;
}

const std::string& genTransportFieldTypeAccessPrefixStr()
{
    static const std::string Str("TransportField_");
    return Str;
}

const std::string& genFieldAccessPrefixStr()
{
    static const std::string Str("field_");
    return Str;
}

const std::string& genBodyFileSuffixStr()
{
    static const std::string Str("_body");
    return Str;
}

const std::string& genMsgFactorySuffixStr()
{
    static const std::string Str("MsgFactory");
    return Str;
}

const std::string& genDocumentFileSuffixStr()
{
    static const std::string Str(".document");
    return Str;
}

const std::string& genPackageFileSuffixStr()
{
    static const std::string Str(".package");
    return Str;
}

const std::string& genPackageAppendFileSuffixStr()
{
    static const std::string Str(".package_append");
    return Str;
}

const std::string& genContentFileSuffixStr()
{
    static const std::string Str(".content");
    return Str;
}

const std::string& genContentPrependFileSuffixStr()
{
    static const std::string Str(".content_prepend");
    return Str;
}

const std::string& genContentAppendFileSuffixStr()
{
    static const std::string Str(".content_append");
    return Str;
}

const std::string& genTitleFileSuffixStr()
{
    static const std::string Str(".title");
    return Str;
}

const std::string& genTitleAppendFileSuffixStr()
{
    static const std::string Str(".title_append");
    return Str;
}

const std::string& genMacroFileSuffixStr()
{
    static const std::string Str(".macro");
    return Str;
}

const std::string& genMacroAppendFileSuffixStr()
{
    static const std::string Str(".macro_append");
    return Str;
}

const std::string& genPdfFileSuffixStr()
{
    static const std::string Str(".pdf_replace");
    return Str;
}

const std::string& genPdfAppendFileSuffixStr()
{
    static const std::string Str(".pdf_append");
    return Str;
}

const std::string& genHtmlFileSuffixStr()
{
    static const std::string Str(".html_replace");
    return Str;
}

const std::string& genHtmlAppendFileSuffixStr()
{
    static const std::string Str(".html_append");
    return Str;
}

const std::string& genHtmlCmdAppendFileSuffixStr()
{
    static const std::string Str(".html_cmd_append");
    return Str;
}

const std::string& genYesStr()
{
    static const std::string Str("Yes");
    return Str;
}

const std::string& genNoStr()
{
    static const std::string Str("No");
    return Str;
}

const std::string& genBigStr()
{
    static const std::string Str("Big");
    return Str;
}

const std::string& genLittleStr()
{
    static const std::string Str("Little");
    return Str;
}

const std::string& genDescriptionStr()
{
    static const std::string Str("description");
    return Str;
}

const std::string& genCommsNameSuffixStr()
{
    static const std::string Str("_comms");
    return Str;
}

const std::string& genValueTypeStr()
{
    static const std::string Str("ValueType");
    return Str;
}

char genSiblingRefPrefix()
{
    return '$';
}

char genStringRefPrefix()
{
    return '^';
}

char genSchemaRefPrefix()
{
    return '@';
}

char genInterfaceFieldRefPrefix()
{
    return '%';
}

} // namespace strings

} // namespace gen

} // namespace commsdsl
