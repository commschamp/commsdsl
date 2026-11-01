//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
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
    [[nodiscard]] bool parse(const std::string& input);
    [[nodiscard]] bool parseValidate();

    [[nodiscard]] ParseSchemasAccessList parseSchemas() const;

    [[nodiscard]] ParseSchemasList& parseSchemaImpls()
    {
        return m_schemas;
    }

    [[nodiscard]] const ParseSchemasList& parseSchemaImpls() const
    {
        return m_schemas;
    }

    [[nodiscard]] ParseSchemaImpl& parseCurrSchema();
    [[nodiscard]] const ParseSchemaImpl& parseCurrSchema() const;

    void parseSetErrorReportCallback(ParseErrorReportFunction&& cb)
    {
        m_errorReportCb = std::move(cb);
    }

    [[nodiscard]] ParseLogger& parseLogger() const
    {
        return m_logger;
    }

    [[nodiscard]] const ParseFieldImpl* parseFindField(const std::string& ref, bool checkRef = true) const;

    [[nodiscard]] const ParseMessageImpl* parseFindMessage(const std::string& ref, bool checkRef = true) const;

    [[nodiscard]] const ParseInterfaceImpl* parseFindInterface(const std::string& ref, bool checkRef = true) const;

    [[nodiscard]] bool parseStrToEnumValue(const std::string& ref, std::intmax_t& val, bool checkRef = true) const;

    [[nodiscard]] bool parseStrToNumeric(const std::string& ref, bool checkRef, std::intmax_t& val, bool& isBigUnsigned) const;
    [[nodiscard]] bool parseStrToFp(const std::string& ref, bool checkRef, double& val) const;
    [[nodiscard]] bool parseStrToBool(const std::string& ref, bool checkRef, bool& val) const;
    [[nodiscard]] bool parseStrToString(const std::string& ref, bool checkRef, std::string& val) const;
    [[nodiscard]] bool parseStrToData(const std::string& ref, bool checkRef, std::vector<std::uint8_t>& val) const;

    [[nodiscard]] bool parseStrToStringValue(const std::string& str, std::string& val) const;

    void parseAddExpectedExtraPrefix(const std::string& value)
    {
        m_extraPrefixes.push_back(value);
    }

    [[nodiscard]] const ParseExtraPrefixes& parseExtraElementPrefixes() const
    {
        return m_extraPrefixes;
    }

    [[nodiscard]] bool parseIsFeatureSupported(unsigned minDslVersion) const;
    [[nodiscard]] bool parseIsFeatureDeprecated(unsigned deprecatedVersion) const;
    [[nodiscard]] bool parseIsPropertySupported(const std::string& name) const;
    [[nodiscard]] bool parseIsPropertyDeprecated(const std::string& name) const;
    [[nodiscard]] bool parseIsFieldValueReferenceSupported() const;
    [[nodiscard]] bool parseIsSemanticTypeLengthSupported() const;
    [[nodiscard]] bool parseIsSemanticTypeRefInheritanceSupported() const;
    [[nodiscard]] bool parseIsNonIntSemanticTypeLengthSupported() const;
    [[nodiscard]] bool parseIsNonUniqueSpecialsAllowedSupported() const;
    [[nodiscard]] bool parseIsFieldAliasSupported() const;
    [[nodiscard]] bool parseIsCopyFieldsFromBundleSupported() const;
    [[nodiscard]] bool parseIsOverrideTypeSupported() const;
    [[nodiscard]] bool parseIsMemberReplaceSupported() const;
    [[nodiscard]] bool parseIsMultiSchemaSupported() const;
    [[nodiscard]] bool parseIsInterfaceFieldReferenceSupported() const;
    [[nodiscard]] bool parseIsFailOnInvalidInMessageSupported() const;
    [[nodiscard]] bool parseIsSizeCompInConditionalsSupported() const;
    [[nodiscard]] bool parseIsExistsCheckInConditionalsSupported() const;
    [[nodiscard]] bool parseIsValidValueInStringAndDataSupported() const;
    [[nodiscard]] bool parseIsValidateMinLengthForFieldsSupported() const;
    [[nodiscard]] bool parseIsMessageReuseSupported() const;
    [[nodiscard]] bool parseIsInterfaceReuseSupported() const;
    [[nodiscard]] bool parseIsValidCondSupportedInCompositeFields() const;
    [[nodiscard]] bool parseIsNamespaceDisplayNameSupported() const;
    [[nodiscard]] bool parseIsFrameDisplayNameSupported() const;
    [[nodiscard]] bool parseIsLayerDisplayNameSupported() const;
    [[nodiscard]] bool parseIsSyncSuffixLayerSupported() const;
    [[nodiscard]] bool parseIsSetMaskSupported() const;

    void parseSetMultipleSchemasEnabled(bool value)
    {
        m_multipleSchemasEnabled = value;
    }

    [[nodiscard]] bool parseGetMultipleSchemasEnabled() const
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
    [[nodiscard]] bool parseValidateDoc(::xmlDocPtr doc);
    [[nodiscard]] bool parseValidateSchema(::xmlNodePtr node);
    [[nodiscard]] bool parseValidatePlatforms(::xmlNodePtr root);
    [[nodiscard]] bool parseValidateSinglePlatform(::xmlNodePtr node);
    [[nodiscard]] bool parseValidateNamespaces(::xmlNodePtr root);
    [[nodiscard]] bool parseValidateAllMessages();
    [[nodiscard]] bool parseStrToValue(const std::string& ref, bool checkRef, ParseStrToValueConvertFunc&& func) const;
    [[nodiscard]] std::pair<const ParseSchemaImpl*, std::string> parseExternalRef(const std::string& externalRef) const;
    [[nodiscard]] bool parseCanRefSchema(const ParseSchemaImpl* schema, const std::string& externalRef) const;

    [[nodiscard]] ParseLogWrapper parseLogError() const;
    [[nodiscard]] ParseLogWrapper parseLogWarning() const;

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
