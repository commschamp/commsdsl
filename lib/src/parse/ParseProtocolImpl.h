//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseErrorLevel.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "commsdsl/parse/ParseSchema.h"
#include "ParseLogger.h"
#include "ParseNamespaceImpl.h"
#include "ParseSchemaImpl.h"

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <utility>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl
{
public:
    using ParseErrorReportFunction = ParseProtocol::ParseErrorReportFunction;
    using ParseExtraPrefixes = std::vector<std::string>;
    using ParseSchemasList = std::vector<ParseSchemaImplPtr>;
    using ParseSchemasAccessList = ParseProtocol::ParseSchemasList;

    ParseProtocolImpl();
    bool parse(const std::string& input);
    bool parseValidate();

    ParseSchemasAccessList parseSchemas() const;

    ParseSchemasList& parseSchemaImpls()
    {
        return m_schemas;
    }

    const ParseSchemasList& parseSchemaImpls() const
    {
        return m_schemas;
    }

    ParseSchemaImpl& parseCurrSchema();
    const ParseSchemaImpl& parseCurrSchema() const;

    void parseSetErrorReportCallback(ParseErrorReportFunction&& cb)
    {
        m_errorReportCb = std::move(cb);
    }

    ParseLogger& parseLogger() const
    {
        return m_logger;
    }

    const ParseFieldImpl* parseFindField(const std::string& ref, bool checkRef = true) const;

    const ParseMessageImpl* parseFindMessage(const std::string& ref, bool checkRef = true) const;

    const ParseInterfaceImpl* parseFindInterface(const std::string& ref, bool checkRef = true) const;

    bool parseStrToEnumValue(const std::string& ref, std::intmax_t& val, bool checkRef = true) const;

    bool parseStrToNumeric(const std::string& ref, bool checkRef, std::intmax_t& val, bool& isBigUnsigned) const;
    bool parseStrToFp(const std::string& ref, bool checkRef, double& val) const;
    bool parseStrToBool(const std::string& ref, bool checkRef, bool& val) const;
    bool parseStrToString(const std::string& ref, bool checkRef, std::string& val) const;
    bool parseStrToData(const std::string& ref, bool checkRef, std::vector<std::uint8_t>& val) const;

    bool parseStrToStringValue(const std::string& str, std::string& val) const;

    void parseAddExpectedExtraPrefix(const std::string& value)
    {
        m_extraPrefixes.push_back(value);
    }

    const ParseExtraPrefixes& parseExtraElementPrefixes() const
    {
        return m_extraPrefixes;
    }

    bool parseIsFeatureSupported(unsigned minDslVersion) const;
    bool parseIsFeatureDeprecated(unsigned deprecatedVersion) const;
    bool parseIsPropertySupported(const std::string& name) const;    
    bool parseIsPropertyDeprecated(const std::string& name) const;    
    bool parseIsFieldValueReferenceSupported() const;
    bool parseIsSemanticTypeLengthSupported() const;
    bool parseIsSemanticTypeRefInheritanceSupported() const;
    bool parseIsNonIntSemanticTypeLengthSupported() const;
    bool parseIsNonUniqueSpecialsAllowedSupported() const;
    bool parseIsFieldAliasSupported() const;
    bool parseIsCopyFieldsFromBundleSupported() const;
    bool parseIsOverrideTypeSupported() const;
    bool parseIsMemberReplaceSupported() const;
    bool parseIsMultiSchemaSupported() const;
    bool parseIsInterfaceFieldReferenceSupported() const;
    bool parseIsFailOnInvalidInMessageSupported() const;
    bool parseIsSizeCompInConditionalsSupported() const;
    bool parseIsExistsCheckInConditionalsSupported() const;
    bool parseIsValidValueInStringAndDataSupported() const;
    bool parseIsValidateMinLengthForFieldsSupported() const;
    bool parseIsMessageReuseSupported() const;
    bool parseIsInterfaceReuseSupported() const;
    bool parseIsValidCondSupportedInCompositeFields() const;
    bool parseIsNamespaceDisplayNameSupported() const;
    bool parseIsFrameDisplayNameSupported() const;

    void parseSetMultipleSchemasEnabled(bool value)
    {
        m_multipleSchemasEnabled = value;
    }

    bool parseGetMultipleSchemasEnabled() const
    {
        return m_multipleSchemasEnabled;
    }

private:
    struct ParseXmlDocFree
    {
        void operator()(::xmlDocPtr p) const
        {
            ::xmlFreeDoc(p);
        }
    };

    using ParseXmlDocPtr = std::unique_ptr<::xmlDoc, ParseXmlDocFree>;
    using ParseDocsList = std::vector<ParseXmlDocPtr>;
    using ParseStrToValueConvertFunc = std::function<bool (const ParseNamespaceImpl& ns, const std::string& ref)>;

    static void parseCbXmlErrorFunc(void* userData, const xmlError* err);
    static void parseCbXmlErrorFunc(void* userData, xmlErrorPtr err);
    void parseHandleXmlError(const xmlError* err);
    bool parseValidateDoc(::xmlDocPtr doc);
    bool parseValidateSchema(::xmlNodePtr node);
    bool parseValidatePlatforms(::xmlNodePtr root);
    bool parseValidateSinglePlatform(::xmlNodePtr node);
    bool parseValidateNamespaces(::xmlNodePtr root);
    bool parseValidateAllMessages();
    bool parseStrToValue(const std::string& ref, bool checkRef, ParseStrToValueConvertFunc&& func) const;
    std::pair<const ParseSchemaImpl*, std::string> parseExternalRef(const std::string& externalRef) const;

    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;

    ParseErrorReportFunction m_errorReportCb;
    ParseDocsList m_docs;
    ParseErrorLevel m_minLevel = ParseErrorLevel_Info;
    mutable ParseLogger m_logger;
    ParseSchemasList m_schemas;
    ParseSchemaImpl* m_currSchema = nullptr;
    ParseExtraPrefixes m_extraPrefixes;
    bool m_validated = false;
    bool m_multipleSchemasEnabled = false;
};

} // namespace parse

} // namespace commsdsl
