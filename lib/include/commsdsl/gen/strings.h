//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include <string>

namespace commsdsl
{

namespace gen
{

namespace strings
{

[[nodiscard]] const std::string& genEmptyString();
[[nodiscard]] const std::string& genMsgIdEnumNameStr();
[[nodiscard]] const std::string& genCmakeListsFileStr();
[[nodiscard]] const std::string& genDefaultOptionsStr();
[[nodiscard]] const std::string& genAllMessagesStr();
[[nodiscard]] const std::string& genMessageClassStr();
[[nodiscard]] const std::string& genCommonSuffixStr();
[[nodiscard]] const std::string& genMembersSuffixStr();
[[nodiscard]] const std::string& genCppHeaderSuffixStr();
[[nodiscard]] const std::string& genCppSourceSuffixStr();
[[nodiscard]] const std::string& genLatexSuffixStr();
[[nodiscard]] const std::string& genFieldNamespaceStr();
[[nodiscard]] const std::string& genMessageNamespaceStr();
[[nodiscard]] const std::string& genFrameNamespaceStr();
[[nodiscard]] const std::string& genLayerNamespaceStr();
[[nodiscard]] const std::string& genChecksumNamespaceStr();
[[nodiscard]] const std::string& genOptionsNamespaceStr();
[[nodiscard]] const std::string& genCispatchNamespaceStr();
[[nodiscard]] const std::string& genFactoryNamespaceStr();
[[nodiscard]] const std::string& genInputNamespaceStr();
[[nodiscard]] const std::string& genPluginNamespaceStr();
[[nodiscard]] const std::string& genIncludeDirStr();
[[nodiscard]] const std::string& genSrcDirStr();
[[nodiscard]] const std::string& genDocDirStr();
[[nodiscard]] const std::string& genNameFileSuffixStr();
[[nodiscard]] const std::string& genNameBodyFileSuffixStr();
[[nodiscard]] const std::string& genValueFileSuffixStr();
[[nodiscard]] const std::string& genReadFileSuffixStr();
[[nodiscard]] const std::string& genReadBodyFileSuffixStr();
[[nodiscard]] const std::string& genRefreshFileSuffixStr();
[[nodiscard]] const std::string& genRefreshBodyFileSuffixStr();
[[nodiscard]] const std::string& genWriteFileSuffixStr();
[[nodiscard]] const std::string& genWriteBodyFileSuffixStr();
[[nodiscard]] const std::string& genPublicFileSuffixStr();
[[nodiscard]] const std::string& genProtectedFileSuffixStr();
[[nodiscard]] const std::string& genPrivateFileSuffixStr();
[[nodiscard]] const std::string& genIncFileSuffixStr();
[[nodiscard]] const std::string& genLengthFileSuffixStr();
[[nodiscard]] const std::string& genLengthBodyFileSuffixStr();
[[nodiscard]] const std::string& genValidFileSuffixStr();
[[nodiscard]] const std::string& genValidBodyFileSuffixStr();
[[nodiscard]] const std::string& genReplaceFileSuffixStr();
[[nodiscard]] const std::string& genExtendFileSuffixStr();
[[nodiscard]] const std::string& genAppendFileSuffixStr();
[[nodiscard]] const std::string& genPrependFileSuffixStr();
[[nodiscard]] const std::string& genPrependLangFileSuffixStr();
[[nodiscard]] const std::string& genBindFileSuffixStr();
[[nodiscard]] const std::string& genConstructFileSuffixStr();
[[nodiscard]] const std::string& genConstructBodyFileSuffixStr();
[[nodiscard]] const std::string& genSourcesFileSuffixStr();
[[nodiscard]] const std::string& genForceEmptyDisplayNameStr();
[[nodiscard]] const std::string& genFieldBaseClassStr();
[[nodiscard]] const std::string& genDefaultOptionsClassStr();
[[nodiscard]] const std::string& genAllMessagesDynMemMsgFactoryDefaultOptionsClassStr();
[[nodiscard]] const std::string& genIndentStr();
[[nodiscard]] const std::string& genDoxygenPrefixStr();
[[nodiscard]] const std::string& genVersionOptionalFieldSuffixStr();
[[nodiscard]] const std::string& genOrigSuffixStr();
[[nodiscard]] const std::string& genMsgIdPrefixStr();
[[nodiscard]] const std::string& genFieldsSuffixStr();
[[nodiscard]] const std::string& genLayersSuffixStr();
[[nodiscard]] const std::string& genBareMetalStr();
[[nodiscard]] const std::string& genDataViewStr();
[[nodiscard]] const std::string& genTransportMessageSuffixStr();
[[nodiscard]] const std::string& genUnexpectedValueStr();
[[nodiscard]] const std::string& genVersionFileNameStr();
[[nodiscard]] const std::string& genEnumFirstValueStr();
[[nodiscard]] const std::string& genEnumLastValueStr();
[[nodiscard]] const std::string& genEnumValuesLimitStr();
[[nodiscard]] const std::string& genTransportFieldAccessPrefixStr();
[[nodiscard]] const std::string& genTransportFieldTypeAccessPrefixStr();
[[nodiscard]] const std::string& genFieldAccessPrefixStr();
[[nodiscard]] const std::string& genBodyFileSuffixStr();
[[nodiscard]] const std::string& genMsgFactorySuffixStr();
[[nodiscard]] const std::string& genDocumentFileSuffixStr();
[[nodiscard]] const std::string& genPackageFileSuffixStr();
[[nodiscard]] const std::string& genPackageAppendFileSuffixStr();
[[nodiscard]] const std::string& genContentFileSuffixStr();
[[nodiscard]] const std::string& genContentPrependFileSuffixStr();
[[nodiscard]] const std::string& genContentAppendFileSuffixStr();
[[nodiscard]] const std::string& genTitleFileSuffixStr();
[[nodiscard]] const std::string& genTitleAppendFileSuffixStr();
[[nodiscard]] const std::string& genMacroFileSuffixStr();
[[nodiscard]] const std::string& genMacroAppendFileSuffixStr();
[[nodiscard]] const std::string& genPdfFileSuffixStr();
[[nodiscard]] const std::string& genPdfAppendFileSuffixStr();
[[nodiscard]] const std::string& genHtmlFileSuffixStr();
[[nodiscard]] const std::string& genHtmlAppendFileSuffixStr();
[[nodiscard]] const std::string& genHtmlCmdAppendFileSuffixStr();
[[nodiscard]] const std::string& genYesStr();
[[nodiscard]] const std::string& genNoStr();
[[nodiscard]] const std::string& genBigStr();
[[nodiscard]] const std::string& genLittleStr();
[[nodiscard]] const std::string& genSystemStr();
[[nodiscard]] const std::string& genDescriptionStr();
[[nodiscard]] const std::string& genCommsNameSuffixStr();
[[nodiscard]] const std::string& genValueTypeStr();
[[nodiscard]] const std::string& genValsSuffixStr();
[[nodiscard]] const std::string& genReadSuffixStr();
[[nodiscard]] const std::string& genValidSuffixStr();
[[nodiscard]] const std::string& genValueSuffixStr();
[[nodiscard]] const std::string& genNameSuffixStr();
[[nodiscard]] const std::string& genNilStr();
[[nodiscard]] const std::string& genLittleEndianSuffixStr();
[[nodiscard]] const std::string& genLittleEndianPrefixStr();

[[nodiscard]] char genSiblingRefPrefix();
[[nodiscard]] char genStringRefPrefix();
[[nodiscard]] char genSchemaRefPrefix();
[[nodiscard]] char genInterfaceFieldRefPrefix();

} // namespace strings

} // namespace gen

} // namespace commsdsl
