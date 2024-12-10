//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <utility>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlerror.h>

#include "commsdsl/parse/ErrorLevel.h"
#include "commsdsl/parse/Protocol.h"
#include "commsdsl/parse/Schema.h"
#include "Logger.h"
#include "NamespaceImpl.h"
#include "SchemaImpl.h"

namespace commsdsl
{

namespace parse
{

class ProtocolImpl
{
public:
    using ErrorReportFunction = Protocol::ErrorReportFunction;
    using ExtraPrefixes = std::vector<std::string>;
    using SchemasList = std::vector<SchemaImplPtr>;
    using SchemasAccessList = Protocol::SchemasList;

    ProtocolImpl();
    bool parse(const std::string& input);
    bool validate();

    SchemasAccessList schemas() const;

    SchemasList& schemaImpls()
    {
        return m_schemas;
    }

    const SchemasList& schemaImpls() const
    {
        return m_schemas;
    }

    SchemaImpl& currSchema();
    const SchemaImpl& currSchema() const;

    void setErrorReportCallback(ErrorReportFunction&& cb)
    {
        m_errorReportCb = std::move(cb);
    }

    Logger& logger() const
    {
        return m_logger;
    }

    const FieldImpl* findField(const std::string& ref, bool checkRef = true) const;

    const MessageImpl* findMessage(const std::string& ref, bool checkRef = true) const;

    const InterfaceImpl* findInterface(const std::string& ref, bool checkRef = true) const;

    bool strToEnumValue(const std::string& ref, std::intmax_t& val, bool checkRef = true) const;

    bool strToNumeric(const std::string& ref, bool checkRef, std::intmax_t& val, bool& isBigUnsigned) const;
    bool strToFp(const std::string& ref, bool checkRef, double& val) const;
    bool strToBool(const std::string& ref, bool checkRef, bool& val) const;
    bool strToString(const std::string& ref, bool checkRef, std::string& val) const;
    bool strToData(const std::string& ref, bool checkRef, std::vector<std::uint8_t>& val) const;

    bool strToStringValue(const std::string& str, std::string& val) const;

    void addExpectedExtraPrefix(const std::string& value)
    {
        m_extraPrefixes.push_back(value);
    }

    const ExtraPrefixes& extraElementPrefixes() const
    {
        return m_extraPrefixes;
    }

    bool isFeatureSupported(unsigned minDslVersion) const;
    bool isFeatureDeprecated(unsigned deprecatedVersion) const;
    bool isPropertySupported(const std::string& name) const;    
    bool isPropertyDeprecated(const std::string& name) const;    
    bool isFieldValueReferenceSupported() const;
    bool isSemanticTypeLengthSupported() const;
    bool isSemanticTypeRefInheritanceSupported() const;
    bool isNonIntSemanticTypeLengthSupported() const;
    bool isNonUniqueSpecialsAllowedSupported() const;
    bool isFieldAliasSupported() const;
    bool isCopyFieldsFromBundleSupported() const;
    bool isOverrideTypeSupported() const;
    bool isMemberReplaceSupported() const;
    bool isMultiSchemaSupported() const;
    bool isInterfaceFieldReferenceSupported() const;
    bool isFailOnInvalidInMessageSupported() const;
    bool isSizeCompInConditionalsSupported() const;
    bool isExistsCheckInConditionalsSupported() const;
    bool isValidValueInStringAndDataSupported() const;
    bool isValidateMinLengthForFieldsSupported() const;
    bool isMessageReuseSupported() const;

    void setMultipleSchemasEnabled(bool value)
    {
        m_multipleSchemasEnabled = value;
    }

    bool getMultipleSchemasEnabled() const
    {
        return m_multipleSchemasEnabled;
    }

private:
    struct XmlDocFree
    {
        void operator()(::xmlDocPtr p) const
        {
            ::xmlFreeDoc(p);
        }
    };

    using XmlDocPtr = std::unique_ptr<::xmlDoc, XmlDocFree>;
    using DocsList = std::vector<XmlDocPtr>;
    using StrToValueConvertFunc = std::function<bool (const NamespaceImpl& ns, const std::string& ref)>;

    static void cbXmlErrorFunc(void* userData, const xmlError* err);
    static void cbXmlErrorFunc(void* userData, xmlErrorPtr err);
    void handleXmlError(const xmlError* err);
    bool validateDoc(::xmlDocPtr doc);
    bool validateSchema(::xmlNodePtr node);
    bool validatePlatforms(::xmlNodePtr root);
    bool validateSinglePlatform(::xmlNodePtr node);
    bool validateNamespaces(::xmlNodePtr root);
    bool validateAllMessages();
    bool validateMessageIds();
    // unsigned countMessageIds() const;
    bool strToValue(const std::string& ref, bool checkRef, StrToValueConvertFunc&& func) const;
    std::pair<const SchemaImpl*, std::string> parseExternalRef(const std::string& externalRef) const;

    LogWrapper logError() const;
    LogWrapper logWarning() const;

    ErrorReportFunction m_errorReportCb;
    DocsList m_docs;
    ErrorLevel m_minLevel = ErrorLevel_Info;
    mutable Logger m_logger;
    SchemasList m_schemas;
    SchemaImpl* m_currSchema = nullptr;
    ExtraPrefixes m_extraPrefixes;
    bool m_validated = false;
    bool m_multipleSchemasEnabled = false;
};

} // namespace parse

} // namespace commsdsl
