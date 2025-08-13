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

#pragma once

#include <string>

namespace commsdsl  
{

namespace gen
{  

namespace strings
{

const std::string& genEmptyString();
const std::string& genMsgIdEnumNameStr();
const std::string& genCmakeListsFileStr();
const std::string& genDefaultOptionsStr();
const std::string& genAllMessagesStr();
const std::string& genMessageClassStr();
const std::string& genCommonSuffixStr();
const std::string& genMembersSuffixStr();
const std::string& genCppHeaderSuffixStr();
const std::string& genCppSourceSuffixStr();
const std::string& genLatexSuffixStr();
const std::string& genFieldNamespaceStr();
const std::string& genMessageNamespaceStr();
const std::string& genFrameNamespaceStr();
const std::string& genLayerNamespaceStr();
const std::string& genChecksumNamespaceStr();
const std::string& genOptionsNamespaceStr();
const std::string& genCispatchNamespaceStr();
const std::string& genFactoryNamespaceStr();
const std::string& genInputNamespaceStr();
const std::string& genPluginNamespaceStr();
const std::string& genIncludeDirStr();
const std::string& genSrcDirStr();
const std::string& genDocDirStr();
const std::string& genNameFileSuffixStr();
const std::string& genNameBodyFileSuffixStr();
const std::string& genValueFileSuffixStr();
const std::string& genReadFileSuffixStr();
const std::string& genReadBodyFileSuffixStr();
const std::string& genRefreshFileSuffixStr();
const std::string& genRefreshBodyFileSuffixStr();
const std::string& genWriteFileSuffixStr();
const std::string& genWriteBodyFileSuffixStr();
const std::string& genPublicFileSuffixStr();
const std::string& genProtectedFileSuffixStr();
const std::string& genPrivateFileSuffixStr();
const std::string& genIncFileSuffixStr();
const std::string& genLengthFileSuffixStr();
const std::string& genLengthBodyFileSuffixStr();
const std::string& genValidFileSuffixStr();
const std::string& genValidBodyFileSuffixStr();
const std::string& genReplaceFileSuffixStr();
const std::string& genExtendFileSuffixStr();
const std::string& genAppendFileSuffixStr();
const std::string& genPrependFileSuffixStr();
const std::string& genPrependLangFileSuffixStr();
const std::string& genBindFileSuffixStr();
const std::string& genConstructFileSuffixStr();
const std::string& genSourcesFileSuffixStr();
const std::string& genForceEmptyDisplayNameStr();
const std::string& genFieldBaseClassStr();
const std::string& genDefaultOptionsClassStr();
const std::string& genAllMessagesDynMemMsgFactoryDefaultOptionsClassStr();
const std::string& genIndentStr();
const std::string& genDoxygenPrefixStr();
const std::string& genVersionOptionalFieldSuffixStr();
const std::string& genOrigSuffixStr();
const std::string& genMsgIdPrefixStr();
const std::string& genFieldsSuffixStr();
const std::string& genLayersSuffixStr();
const std::string& genBareMetalStr();
const std::string& genDataViewStr();
const std::string& genTransportMessageSuffixStr();
const std::string& genUnexpectedValueStr();
const std::string& genVersionFileNameStr();
const std::string& genEnumFirstValueStr();
const std::string& genEnumLastValueStr();
const std::string& genEnumValuesLimitStr();
const std::string& genTransportFieldAccessPrefixStr();
const std::string& genTransportFieldTypeAccessPrefixStr();
const std::string& genFieldAccessPrefixStr();
const std::string& genBodyFileSuffixStr();
const std::string& genMsgFactorySuffixStr();
const std::string& genDocumentFileSuffixStr();
const std::string& genPackageFileSuffixStr();
const std::string& genPackageAppendFileSuffixStr();
const std::string& genContentFileSuffixStr();
const std::string& genContentPrependFileSuffixStr();
const std::string& genContentAppendFileSuffixStr();
const std::string& genTitleFileSuffixStr();
const std::string& genTitleAppendFileSuffixStr();
const std::string& genMacroFileSuffixStr();
const std::string& genMacroAppendFileSuffixStr();
const std::string& genPdfFileSuffixStr();
const std::string& genPdfAppendFileSuffixStr();
const std::string& genHtmlFileSuffixStr();
const std::string& genHtmlAppendFileSuffixStr();
const std::string& genHtmlCmdAppendFileSuffixStr();
const std::string& genYesStr();
const std::string& genNoStr();
const std::string& genBigStr();
const std::string& genLittleStr();


char genSiblingRefPrefix();
char genStringRefPrefix();
char genSchemaRefPrefix();
char genInterfaceFieldRefPrefix();

} // namespace strings

} // namespace gen

} // namespace commsdsl
