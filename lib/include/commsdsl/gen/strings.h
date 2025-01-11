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

const std::string& emptyString();
const std::string& msgIdEnumNameStr();
const std::string& cmakeListsFileStr();
const std::string& defaultOptionsStr();
const std::string& allMessagesStr();
const std::string& messageClassStr();
const std::string& commonSuffixStr();
const std::string& membersSuffixStr();
const std::string& cppHeaderSuffixStr();
const std::string& cppSourceSuffixStr();
const std::string& fieldNamespaceStr();
const std::string& messageNamespaceStr();
const std::string& frameNamespaceStr();
const std::string& layerNamespaceStr();
const std::string& checksumNamespaceStr();
const std::string& optionsNamespaceStr();
const std::string& dispatchNamespaceStr();
const std::string& factoryNamespaceStr();
const std::string& inputNamespaceStr();
const std::string& pluginNamespaceStr();
const std::string& includeDirStr();
const std::string& srcDirStr();
const std::string& docDirStr();
const std::string& nameFileSuffixStr();
const std::string& nameBodyFileSuffixStr();
const std::string& valueFileSuffixStr();
const std::string& readFileSuffixStr();
const std::string& readBodyFileSuffixStr();
const std::string& refreshFileSuffixStr();
const std::string& refreshBodyFileSuffixStr();
const std::string& writeFileSuffixStr();
const std::string& writeBodyFileSuffixStr();
const std::string& publicFileSuffixStr();
const std::string& protectedFileSuffixStr();
const std::string& privateFileSuffixStr();
const std::string& incFileSuffixStr();
const std::string& lengthFileSuffixStr();
const std::string& lengthBodyFileSuffixStr();
const std::string& validFileSuffixStr();
const std::string& validBodyFileSuffixStr();
const std::string& replaceFileSuffixStr();
const std::string& extendFileSuffixStr();
const std::string& appendFileSuffixStr();
const std::string& prependFileSuffixStr();
const std::string& prependLangFileSuffixStr();
const std::string& bindFileSuffixStr();
const std::string& constructFileSuffixStr();
const std::string& sourcesFileSuffixStr();
const std::string& forceEmptyDisplayNameStr();
const std::string& fieldBaseClassStr();
const std::string& defaultOptionsClassStr();
const std::string& allMessagesDynMemMsgFactoryDefaultOptionsClassStr();
const std::string& indentStr();
const std::string& doxygenPrefixStr();
const std::string& versionOptionalFieldSuffixStr();
const std::string& origSuffixStr();
const std::string& msgIdPrefixStr();
const std::string& fieldsSuffixStr();
const std::string& layersSuffixStr();
const std::string& bareMetalStr();
const std::string& dataViewStr();
const std::string& transportMessageSuffixStr();
const std::string& unexpectedValueStr();
const std::string& versionFileNameStr();
const std::string& enumFirstValueStr();
const std::string& enumLastValueStr();
const std::string& enumValuesLimitStr();
const std::string& transportFieldAccessPrefixStr();
const std::string& transportFieldTypeAccessPrefixStr();
const std::string& fieldAccessPrefixStr();
const std::string& bodyFileSuffixStr();

char siblingRefPrefix();
char stringRefPrefix();
char schemaRefPrefix();
char interfaceFieldRefPrefix();

} // namespace strings

} // namespace gen

} // namespace commsdsl
