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

const std::string& emptyString()
{
    static const std::string Str;
    return Str;
}    

const std::string& msgIdEnumNameStr()
{
    static const std::string Str("MsgId");
    return Str;
}

const std::string& cmakeListsFileStr()
{
    static const std::string Str("CMakeLists.txt");
    return Str;
}

const std::string& defaultOptionsStr()
{
    static const std::string Str("DefaultOptions");
    return Str;
}

const std::string& allMessagesStr()
{
    static const std::string Str("AllMessages");
    return Str;    
}

const std::string& messageClassStr()
{
    static const std::string Str("Message");
    return Str;    
}

const std::string& commonSuffixStr()
{
    static const std::string Str("Common");
    return Str;     
}

const std::string& membersSuffixStr()
{
    static const std::string Str("Members");
    return Str;     
}

const std::string& cppHeaderSuffixStr()
{
    static const std::string Str(".h");
    return Str;     
}

const std::string& cppSourceSuffixStr()
{
    static const std::string Str(".cpp");
    return Str;     
}

const std::string& fieldNamespaceStr()
{
    static const std::string Str("field");
    return Str;   
}

const std::string& messageNamespaceStr()
{
    static const std::string Str("message");
    return Str;   
}

const std::string& frameNamespaceStr()
{
    static const std::string Str("frame");
    return Str;   
}

const std::string& layerNamespaceStr()
{
    static const std::string Str("layer");
    return Str;   
}

const std::string& checksumNamespaceStr()
{
    static const std::string Str("checksum");
    return Str;   
}

const std::string& optionsNamespaceStr()
{
    static const std::string Str("options");
    return Str;   
}

const std::string& dispatchNamespaceStr()
{
    static const std::string Str("dispatch");
    return Str;   
}

const std::string& factoryNamespaceStr()
{
    static const std::string Str("factory");
    return Str;   
}

const std::string& inputNamespaceStr()
{
    static const std::string Str("input");
    return Str;   
}

const std::string& pluginNamespaceStr()
{
    static const std::string Str("plugin");
    return Str;   
}

const std::string& includeDirStr()
{
    static const std::string Str("include");
    return Str;   
}

const std::string& srcDirStr()
{
    static const std::string Str("src");
    return Str;   
}

const std::string& docDirStr()
{
    static const std::string Str("doc");
    return Str;   
}

const std::string& nameFileSuffixStr()
{
    static const std::string Str(".name");
    return Str;     
}

const std::string& nameBodyFileSuffixStr()
{
    static const std::string Str = nameFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& valueFileSuffixStr()
{
    static const std::string Str(".value");
    return Str;     
}

const std::string& readFileSuffixStr()
{
    static const std::string Str(".read");
    return Str;     
}

const std::string& readBodyFileSuffixStr()
{
    static const std::string Str = readFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& refreshFileSuffixStr()
{
    static const std::string Str(".refresh");
    return Str;     
}

const std::string& refreshBodyFileSuffixStr()
{
    static const std::string Str = refreshFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& writeFileSuffixStr()
{
    static const std::string Str(".write");
    return Str;     
}

const std::string& writeBodyFileSuffixStr()
{
    static const std::string Str = writeFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& publicFileSuffixStr()
{
    static const std::string Str(".public");
    return Str;     
}

const std::string& protectedFileSuffixStr()
{
    static const std::string Str(".protected");
    return Str;     
}

const std::string& privateFileSuffixStr()
{
    static const std::string Str(".private");
    return Str;     
}

const std::string& incFileSuffixStr()
{
    static const std::string Str(".inc");
    return Str;     
}

const std::string& lengthFileSuffixStr()
{
    static const std::string Str(".length");
    return Str;     
}

const std::string& lengthBodyFileSuffixStr()
{
    static const std::string Str = lengthFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& validFileSuffixStr()
{
    static const std::string Str(".valid");
    return Str;     
}

const std::string& validBodyFileSuffixStr()
{
    static const std::string Str = validFileSuffixStr() + bodyFileSuffixStr();
    return Str;     
}

const std::string& replaceFileSuffixStr()
{
    static const std::string Str(".replace");
    return Str;     
}

const std::string& extendFileSuffixStr()
{
    static const std::string Str(".extend");
    return Str;     
}

const std::string& appendFileSuffixStr()
{
    static const std::string Str(".append");
    return Str;     
}

const std::string& prependFileSuffixStr()
{
    static const std::string Str(".prepend");
    return Str;     
}

const std::string& prependLangFileSuffixStr()
{
    static const std::string Str(".prepend_lang");
    return Str;     
}

const std::string& bindFileSuffixStr()
{
    static const std::string Str(".bind");
    return Str;     
}

const std::string& constructFileSuffixStr()
{
    static const std::string Str(".construct");
    return Str;     
}

const std::string& sourcesFileSuffixStr()
{
    static const std::string Str(".sources");
    return Str;     
}

const std::string& forceEmptyDisplayNameStr()
{
    static const std::string Str("_");
    return Str;
}

const std::string& fieldBaseClassStr()
{
    static const std::string Str("FieldBase");
    return Str;    
}

const std::string& defaultOptionsClassStr()
{
    static const std::string Str("DefaultOptions");
    return Str;    
}

const std::string& allMessagesDynMemMsgFactoryDefaultOptionsClassStr()
{
    static const std::string Str("AllMessagesDynMemMsgFactoryDefaultOptions");
    return Str;      
}

const std::string& indentStr()
{
    static const std::string Str("    ");
    return Str;    
}

const std::string& doxygenPrefixStr()
{
    static const std::string Str("/// ");
    return Str;      
}

const std::string& versionOptionalFieldSuffixStr()
{
    static const std::string Str("Field");
    return Str;      
}

const std::string& origSuffixStr()
{
    static const std::string Str("Orig");
    return Str;      
}

const std::string& msgIdPrefixStr()
{
    static const std::string Str("MsgId_");
    return Str;    
}

const std::string& fieldsSuffixStr()
{
    static const std::string Str("Fields");
    return Str;    
}

const std::string& layersSuffixStr()
{
    static const std::string Str("Layers");
    return Str;    
}

const std::string& bareMetalStr()
{
    static const std::string Str("BareMetal");
    return Str;    
}

const std::string& dataViewStr()
{
    static const std::string Str("DataView");
    return Str;    
}

const std::string& transportMessageSuffixStr()
{
    static const std::string Str("TransportMessage");
    return Str;     
}

const std::string& unexpectedValueStr()
{
    static const std::string Str("???");
    return Str;     
}

const std::string& versionFileNameStr()
{
    static const std::string Str("Version");
    return Str;      
}

const std::string& enumFirstValueStr()
{
    static const std::string Str("FirstValue");
    return Str;
}

const std::string& enumLastValueStr()
{
    static const std::string Str("LastValue");
    return Str;
}

const std::string& enumValuesLimitStr()
{
    static const std::string Str("ValuesLimit");
    return Str;
}

const std::string& transportFieldAccessPrefixStr()
{
    static const std::string Str("transportField_");
    return Str;    
}

const std::string& transportFieldTypeAccessPrefixStr()
{
    static const std::string Str("TransportField_");
    return Str;    
}

const std::string& fieldAccessPrefixStr()
{
    static const std::string Str("field_");
    return Str;    
}

const std::string& bodyFileSuffixStr()
{
    static const std::string Str("_body");
    return Str;     
}

const std::string& msgFactorySuffixStr()
{
    static const std::string Str("MsgFactory");
    return Str;     
}

char siblingRefPrefix()
{
    return '$';
}

char stringRefPrefix()
{
    return '^';
}

char schemaRefPrefix()
{
    return '@';
}

char interfaceFieldRefPrefix()
{
    return '%';
}

} // namespace strings

} // namespace gen

} // namespace commsdsl
