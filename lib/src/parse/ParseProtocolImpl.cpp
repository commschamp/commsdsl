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

#include "ParseProtocolImpl.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <type_traits>

#include "ParseEnumFieldImpl.h"
#include "ParseFieldImpl.h"
#include "ParseXmlWrap.h"

namespace commsdsl
{

namespace parse
{

ParseProtocolImpl::ParseProtocolImpl() :
    m_logger(
        [this](ParseErrorLevel level, const std::string& msg)
        {
            if (m_errorReportCb) {
                m_errorReportCb(level, msg);
                return;
            }

            static const std::string Prefix[] = {
                "[DEBUG]: ",
                "[INFO]: ",
                "[WARNING]: ",
                "[ERROR]: "
            };

            static const std::size_t PrefixCount = std::extent<decltype(Prefix)>::value;
            static_assert(PrefixCount == ParseErrorLevel_NumOfValues, "Invalid map");

            std::cerr << Prefix[level] << msg << std::endl;
        }
    )
{
    xmlSetStructuredErrorFunc(this, static_cast<xmlStructuredErrorFunc>(&ParseProtocolImpl::parseCbXmlErrorFunc));
    m_logger.parseSetMinLevel(m_minLevel);
}

bool ParseProtocolImpl::parse(const std::string& input)
{
    if (m_validated) {
        parseLogError() << "Parsing extra files after validation is not allowed";
        return false;
    }

    XmlDocPtr doc(xmlParseFile(input.c_str()));
    if (!doc) {
        std::cerr << "ERROR: Failed to parse" << input << std::endl;
        return false;
    }

    m_docs.push_back(std::move(doc));
    return true;
}

bool ParseProtocolImpl::parseValidate()
{
    if (m_validated) {
        return true;
    }

    if (m_docs.empty()) {
        parseLogError() << "Cannot validate without any schema files";
        return false;
    }

    for (auto& d : m_docs) {
        if (!parseValidateDoc(d.get())) {
            return false;
        }
    }

    if (!parseValidateAllMessages()) {
        return false;
    }

    m_validated = true;
    return true;
}

ParseProtocolImpl::SchemasAccessList ParseProtocolImpl::parseSchemas() const
{
    SchemasAccessList list;
    for (auto& s : m_schemas) {
        list.push_back(ParseSchema(s.get()));
    }

    return list;
}

ParseSchemaImpl& ParseProtocolImpl::parseCurrSchema()
{
    assert(m_currSchema != nullptr);
    return *m_currSchema;
}

const ParseSchemaImpl& ParseProtocolImpl::parseCurrSchema() const
{
    assert(m_currSchema != nullptr);
    return *m_currSchema;
}

const ParseFieldImpl* ParseProtocolImpl::parseFindField(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->parseFindField(parsedRef.second, checkRef);
}

const ParseMessageImpl* ParseProtocolImpl::parseFindMessage(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->parseFindMessage(parsedRef.second, checkRef);
}

const ParseInterfaceImpl* ParseProtocolImpl::parseFindInterface(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->parseFindInterface(parsedRef.second, checkRef);
}

bool ParseProtocolImpl::parseStrToEnumValue(
    const std::string& ref,
    std::intmax_t& val,
    bool checkRef) const
{
    if (checkRef) {
        if (!common::parseIsValidRefName(ref)) {
            return false;
        }
    }
    else {
        assert(common::parseIsValidRefName(ref));
    }

    auto dotCount = static_cast<std::size_t>(std::count(ref.begin(), ref.end(), '.'));
    if (dotCount < 1U) {
        return false;
    }

    auto nameSepPos = ref.find_last_of('.');
    assert(nameSepPos != std::string::npos);
    assert(0U < nameSepPos);
    auto signedNameSepPos = static_cast<std::ptrdiff_t>(nameSepPos);
    std::string elemName(ref.begin() + signedNameSepPos + 1, ref.end());
    std::string fieldRefPath(ref.begin(), ref.begin() + signedNameSepPos);
    auto* field = parseFindField(fieldRefPath, false);
    if ((field == nullptr) || (field->parseKind() != ParseField::Kind::Enum)) {
        return false;
    }

    auto* enumField = static_cast<const ParseEnumFieldImpl*>(field);
    auto& enumValues = enumField->parseValues();
    auto enumValueIter = enumValues.find(elemName);
    if (enumValueIter == enumValues.end()) {
        return false;
    }

    val = enumValueIter->second.m_value;
    return true;
}

bool ParseProtocolImpl::parseStrToNumeric(
    const std::string& ref,
    bool checkRef,
    std::intmax_t& val,
    bool& isBigUnsigned) const
{
    return
        parseStrToValue(
            ref, checkRef,
            [&val, &isBigUnsigned](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.parseStrToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseProtocolImpl::parseStrToFp(
    const std::string& ref,
    bool checkRef,
    double& val) const
{
    return
        parseStrToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.parseStrToFp(str, val);
            });
}

bool ParseProtocolImpl::parseStrToBool(
    const std::string& ref,
    bool checkRef,
    bool& val) const
{
    return
        parseStrToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.parseStrToBool(str, val);
            });
}

bool ParseProtocolImpl::parseStrToString(
    const std::string& ref,
    bool checkRef,
    std::string& val) const
{
    return
        parseStrToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.parseStrToString(str, val);
            });
}

bool ParseProtocolImpl::parseStrToData(
    const std::string& ref,
    bool checkRef,
    std::vector<std::uint8_t>& val) const
{
    return
        parseStrToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.parseStrToData(str, val);
            });
}

bool ParseProtocolImpl::parseIsFeatureSupported(unsigned minDslVersion) const
{
    auto currDslVersion = parseCurrSchema().parseDslVersion();
    if (currDslVersion == 0U) {
        return true;
    }

    return minDslVersion <= currDslVersion;
}

bool ParseProtocolImpl::parseIsFeatureDeprecated(unsigned deprecatedVersion) const
{
    auto currDslVersion = parseCurrSchema().parseDslVersion();
    if (currDslVersion == 0U) {
        return false;
    }

    return deprecatedVersion <= currDslVersion;
}

bool ParseProtocolImpl::parseIsPropertySupported(const std::string& name) const
{
    static const std::map<std::string, unsigned> Map = {
        {common::parseValidateMinLengthStr(), 4U},
        {common::parseDefaultValidValueStr(), 4U},
        {common::parseAvailableLengthLimitStr(), 4U},
        {common::parseCopyCodeFromStr(), 5U},
        {common::parseSemanticLayerTypeStr(), 5U},
        {common::parseChecksumFromStr(), 5U},
        {common::parseChecksumUntilStr(), 5U},
        {common::parseTermSuffixStr(), 5U},
        {common::parseMissingOnReadFailStr(), 5U},
        {common::parseMissingOnInvalparseIdStr(), 5U},
        {common::parseReuseCodeStr(), 5U},
        {common::parseConstructStr(), 6U},
        {common::parseReadCondStr(), 6U},
        {common::parseValidCondStr(), 6U},
        {common::parseConstructAsReadCondStr(), 6U},
        {common::parseConstructAsValidCondStr(), 6U},
        {common::parseFixedValueStr(), 7U},
        {common::parseCopyConstructFromStr(), 7U},
        {common::parseCopyReadCondFromStr(), 7U},
        {common::parseCopyValidCondFromStr(), 7U},
    };

    auto iter = Map.find(name);
    if (iter == Map.end()) {
        return true;
    }

    return parseIsFeatureSupported(iter->second);
}

bool ParseProtocolImpl::parseIsPropertyDeprecated(const std::string& name) const
{
    static const std::map<std::string, unsigned> Map = {
        {common::parseDisplayReadOnlyStr(), 7U},
        {common::parseDisplayHiddenStr(), 7U},
        {common::parseDisplaySpecialsStr(), 7U},
        {common::parseDisplayExtModeCtrlStr(), 7U},
        {common::parseDisplayIdxReadOnlyHiddenStr(), 7U},
    };

    auto iter = Map.find(name);
    if (iter == Map.end()) {
        return true;
    }

    return parseIsFeatureDeprecated(iter->second);
}

bool ParseProtocolImpl::parseIsFieldValueReferenceSupported() const
{
    return parseIsFeatureSupported(2U);
}

bool ParseProtocolImpl::parseIsSemanticTypeLengthSupported() const
{
    return parseIsFeatureSupported(2U);
}

bool ParseProtocolImpl::parseIsSemanticTypeRefInheritanceSupported() const
{
    return parseIsFeatureSupported(2U);
}

bool ParseProtocolImpl::parseIsNonUniqueSpecialsAllowedSupported() const
{
    return parseIsFeatureSupported(2U);
}

bool ParseProtocolImpl::parseIsFieldAliasSupported() const
{
    return parseIsFeatureSupported(3U);
}

bool ParseProtocolImpl::parseIsCopyFieldsFromBundleSupported() const
{
    return parseIsFeatureSupported(4U);
}

bool ParseProtocolImpl::parseIsOverrideTypeSupported() const
{
    return parseIsFeatureSupported(4U);
}

bool ParseProtocolImpl::parseIsNonIntSemanticTypeLengthSupported() const
{
    return parseIsFeatureSupported(5U);
}

bool ParseProtocolImpl::parseIsMemberReplaceSupported() const
{
    return parseIsFeatureSupported(5U);
}

bool ParseProtocolImpl::parseIsMultiSchemaSupported() const
{
    return parseIsFeatureSupported(5U);
}

bool ParseProtocolImpl::parseIsInterfaceFieldReferenceSupported() const
{
    return parseIsFeatureSupported(6U);
}

bool ParseProtocolImpl::parseIsFailOnInvalidInMessageSupported() const
{
    return parseIsFeatureSupported(6U);
}

bool ParseProtocolImpl::parseIsSizeCompInConditionalsSupported() const
{
    return parseIsFeatureSupported(6U);
}

bool ParseProtocolImpl::parseIsExistsCheckInConditionalsSupported() const
{
    return parseIsFeatureSupported(6U);
}

bool ParseProtocolImpl::parseIsValidValueInStringAndDataSupported() const 
{
    return parseIsFeatureSupported(7U);
}

bool ParseProtocolImpl::parseIsValidateMinLengthForFieldsSupported() const
{
    return parseIsFeatureSupported(7U);
}

bool ParseProtocolImpl::parseIsMessageReuseSupported() const
{
    return parseIsFeatureSupported(7U);
}

bool ParseProtocolImpl::parseIsInterfaceReuseSupported() const
{
    return parseIsMessageReuseSupported();
}

bool ParseProtocolImpl::parseIsValidCondSupportedInCompositeFields() const
{
    return parseIsFeatureSupported(7U);
}

void ParseProtocolImpl::parseCbXmlErrorFunc(void* userData, const xmlError* err)
{
    reinterpret_cast<ParseProtocolImpl*>(userData)->parseHandleXmlError(err);
}

void ParseProtocolImpl::parseCbXmlErrorFunc(void* userData, xmlErrorPtr err)
{
    reinterpret_cast<ParseProtocolImpl*>(userData)->parseHandleXmlError(err);
}

void ParseProtocolImpl::parseHandleXmlError(const xmlError* err)
{
    static const ParseErrorLevel Map[] = {
        /* XML_ERR_NONE */ ParseErrorLevel_Debug,
        /* XML_ERR_WARNING */ ParseErrorLevel_Warning,
        /* XML_ERR_ERROR */ ParseErrorLevel_Error,
        /* XML_ERR_FATAL */ ParseErrorLevel_Error
    };

    static_assert(XML_ERR_NONE == 0, "Invalid assumption");
    static_assert(XML_ERR_FATAL == 3, "Invalid assumption");

    ParseErrorLevel level = ParseErrorLevel_Error;
    if ((XML_ERR_NONE <= err->level) && (err->level <= XML_ERR_FATAL)) {
        level = Map[err->level];
    }

    m_logger.parseSetCurrLevel(level);
    do {
        if (err == nullptr) {
            break;
        }

        if (err->file != nullptr) {
            m_logger << std::string(err->file) << ':';
        }

        if (err->line != 0) {
            m_logger << err->line << ": ";
        }

        m_logger << err->message;
    } while (false);
    m_logger.parseFlush();
}

bool ParseProtocolImpl::parseValidateDoc(::xmlDocPtr doc)
{
    auto* root = ::xmlDocGetRootElement(doc);
    if (root == nullptr) {
        parseLogError() << "Failed to fine root element in the schema file \"" << doc->URL << "\"";
        return false;
    }

    static const std::string SchemaName("schema");
    std::string rootName(reinterpret_cast<const char*>(root->name));
    if (rootName != SchemaName) {
        parseLogError() << "Root element of \"" << doc->URL << "\" is not \"" << SchemaName << '\"';
        return false;
    }

    return
        parseValidateSchema(root) &&
        parseValidatePlatforms(root) &&
        parseValidateNamespaces(root);
}

bool ParseProtocolImpl::parseValidateSchema(::xmlNodePtr node)
{
    ParseSchemaImplPtr schema(new ParseSchemaImpl(node, *this));
    if (!schema->parseProcessNode()) {
        return false;
    }

    auto schemaName = schema->parseName();
    if (schemaName.empty() && (m_currSchema != nullptr)) {
        schemaName = m_currSchema->parseName();
    }

    if (schemaName.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(schema->parseGetNode()) <<
            "First schema definition must define \"" << common::parseNameStr() << "\" property.";
        return false;
    }    

    auto schemaIter = 
        std::find_if(
            m_schemas.begin(), m_schemas.end(), 
            [&schemaName](auto& s)
            {
                return schemaName == s->parseName();
            });

    if (schemaIter == m_schemas.end()) {
        assert(!schema->parseName().empty());
        if ((!m_schemas.empty()) && (!parseIsMultiSchemaSupported())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(schema->parseGetNode()) <<
                "Multiple schemas is not supported in the selected " << common::parseDslVersionStr();
            return false;
        }

        if ((!m_schemas.empty()) && (!m_multipleSchemasEnabled)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(schema->parseGetNode()) <<
                "Multiple schemas support must be explicitly enabled by the code generator.";
            return false;
        }

        m_schemas.push_back(std::move(schema));
        m_currSchema = m_schemas.back().get();
        return true;        
    } 

    m_currSchema = schemaIter->get();

    auto& props = schema->parseProps();
    auto& origProps = m_currSchema->parseProps();
    for (auto& p : props) {
        auto iter = origProps.find(p.first);
        if ((iter == origProps.end()) ||
            (iter->second != p.second)) {

            parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
                "Value of \"" << p.first <<
                "\" property of \"" << node->name << "\" element differs from the first one.";
            return false;
        }
    }

    auto& attrs = schema->parseExtraAttributes();
    auto& origAttrs = m_currSchema->parseExtraAttributes();
    for (auto& a : attrs) {
        auto iter = origAttrs.find(a.first);
        if (iter == origAttrs.end()) {
            origAttrs.insert(a);
            continue;
        }

        if (iter->second == a.second) {
            continue;
        }

        parseLogWarning() << ParseXmlWrap::parseLogPrefix(node) <<
            "Value of \"" << a.first <<
            "\" attribubes of \"" << node->name << "\" element differs from the previous one.";
    }

    auto& children = schema->parseExtraChildrenElements();
    auto& origChildren = m_currSchema->parseExtraChildrenElements();
    for (auto& c : children) {
        origChildren.push_back(c);
    }

    return true;
}

bool ParseProtocolImpl::parseValidatePlatforms(::xmlNodePtr root)
{
    auto platforms = ParseXmlWrap::parseGetChildren(root, common::parsePlatformsStr());
    for (auto& p : platforms) {
        auto pChildren = ParseXmlWrap::parseGetChildren(p);
        for (auto c : pChildren) {
            assert(c->name != nullptr);
            std::string name(reinterpret_cast<const char*>(c->name));
            if (name != common::parsePlatformStr()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(c) <<
                    "Unexpected element, \"" << common::parsePlatformStr() << "\" is expected.";
                return false;
            }

            if (!parseValidateSinglePlatform(c)) {
                return false;
            }
        }
    }

    auto singlePlatforms = ParseXmlWrap::parseGetChildren(root, common::parsePlatformStr());
    return std::all_of(
                singlePlatforms.begin(), singlePlatforms.end(),
                [this](auto p)
                {
                    return this->parseValidateSinglePlatform(p);
                });
}

bool ParseProtocolImpl::parseValidateSinglePlatform(::xmlNodePtr node)
{
    static const ParseXmlWrap::NamesList Names = {
        common::parseNameStr()
    };

    auto props = ParseXmlWrap::parseNodeProps(node);
    if (!ParseXmlWrap::parseChildrenAsProps(node, Names, m_logger, props)) {
        return false;
    }

    auto iter = props.find(common::parseNameStr());
    if (iter == props.end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
            "Required property \"" << common::parseNameStr() << "\" is not defined.";
        return false;
    }

    auto& name = iter->second;
    static const std::string InvalidChars("+-,");
    auto pos = name.find_first_of(InvalidChars);
    if (pos != std::string::npos) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(node) <<
            "Invalid platform name (" << name << ".";
        return false;
    }

    if (!parseCurrSchema().parseAddPlatform(name)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(node) <<
            "Platform \"" << name << "\" defined more than once.";
        return true;
    }

    return true;
}

bool ParseProtocolImpl::parseValidateNamespaces(::xmlNodePtr root)
{
    auto& namespaces = parseCurrSchema().parseNamespaces();
    auto& childrenNames = ParseNamespaceImpl::parseSupportedChildren();
    auto children = ParseXmlWrap::parseGetChildren(root, childrenNames);
    for (auto& c : children) {
        assert(c->name != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto cNameIter = std::find(childrenNames.begin(), childrenNames.end(), cName);
        if (cNameIter == childrenNames.end()) {
            continue;
        }

        if (cName == common::parseNsStr()) {
            ParseNamespaceImplPtr ns(new ParseNamespaceImpl(c, *this));
            ns->parseSetParent(&parseCurrSchema());
            if (!ns->parseProps()) {
                return false;
            }

            auto& nsName = ns->parseName();
            auto iter = namespaces.find(nsName);
            ParseNamespaceImpl* nsToProcess = nullptr;
            ParseNamespaceImpl* realNs = nullptr;
            do {
                if (iter == namespaces.end()) {
                    parseCurrSchema().parseAddNamespace(std::move(ns));
                    iter = namespaces.find(nsName);
                    assert(iter != namespaces.end());
                    nsToProcess = iter->second.get();
                    break;
                }

                nsToProcess = ns.get();
                realNs = iter->second.get();

                if ((!nsToProcess->parseDescription().empty()) &&
                    (nsToProcess->parseDescription() != realNs->parseDescription())) {
                    if (realNs->parseDescription().empty()) {
                        realNs->parseUpdateDescription(nsToProcess->parseDescription());
                    }
                    else {
                        parseLogWarning() << ParseXmlWrap::parseLogPrefix(nsToProcess->parseGetNode()) <<
                            "Description of namespace \"" << nsToProcess->parseName() << "\" differs to "
                            "one encountered before.";
                    }
                }

                if (!nsToProcess->parseExtraAttributes().empty()) {
                    for (auto& a : nsToProcess->parseExtraAttributes()) {
                        auto attIter = realNs->parseExtraAttributes().find(a.first);
                        if (attIter == realNs->parseExtraAttributes().end()) {
                            realNs->parseExtraAttributes().insert(a);
                        }
                        else if (a.second != attIter->second) {
                            parseLogWarning() << ParseXmlWrap::parseLogPrefix(nsToProcess->parseGetNode()) <<
                                "Value of attribute \"" << a.first << "\" differs to one defined before.";
                        }
                    }
                }

                realNs->parseExtraChildren().insert(realNs->parseExtraChildren().end(), nsToProcess->parseExtraChildren().begin(), nsToProcess->parseExtraChildren().end());

            } while (false);

            assert(iter->second);
            if (!nsToProcess->parseChildren(realNs)) {
                return false;
            }

            continue;
        }

        auto& globalNs = parseCurrSchema().parseDefaultNamespace();

        if (!globalNs.processChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseProtocolImpl::parseValidateAllMessages()
{
    return 
        std::all_of(
            m_schemas.begin(), m_schemas.end(),
            [](auto& s)
            {
                return s->parseValidateAllMessages();
            });
}

bool ParseProtocolImpl::parseStrToValue(const std::string& ref, bool checkRef, StrToValueConvertFunc&& func) const
{
    do {
        if (!checkRef) {
            assert(common::parseIsValidRefName(ref));
            break;
        }


        if (!common::parseIsValidRefName(ref)) {
            return false;
        }

    } while (false);

    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return false;
    }

    auto& namespaces = parsedRef.first->parseNamespaces();    

    auto redirectToGlobalNs =
        [&func, &parsedRef, &namespaces]() -> bool
        {
            auto iter = namespaces.find(common::parseEmptyString());
            if (iter == namespaces.end()) {
                return false;
            }
            assert(iter->second);
            return func(*iter->second, parsedRef.second);
        };

    auto firstDotPos = parsedRef.second.find_first_of('.');
    if (firstDotPos == std::string::npos) {
        return redirectToGlobalNs();
    }

    std::string ns(parsedRef.second, 0, firstDotPos);
    assert(!ns.empty());
    auto nsIter = namespaces.find(ns);
    if (nsIter == namespaces.end()) {
        return redirectToGlobalNs();
    }

    assert(nsIter->second);
    std::string subRef(parsedRef.second, firstDotPos + 1);
    return func(*nsIter->second, subRef);
}

std::pair<const ParseSchemaImpl*, std::string> ParseProtocolImpl::parseExternalRef(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    if (externalRef[0] != common::parseSchemaRefPrefix()) {
        return std::make_pair(&parseCurrSchema(), externalRef);
    }

    auto dotPos = externalRef.find('.');
    if (externalRef.size() <= dotPos) {
        return std::make_pair(nullptr, externalRef);
    }

    std::string schemaName = externalRef.substr(1, dotPos - 1);
    auto restRef = externalRef.substr(dotPos + 1);
    auto iter = 
        std::find_if(
            m_schemas.begin(), m_schemas.end(),
            [&schemaName](auto& s)
            {
                return schemaName == s->parseName();
            });

    if (iter == m_schemas.end()) {
        return std::make_pair(nullptr, std::move(restRef));
    }

    return std::make_pair(iter->get(), std::move(restRef));
}

LogWrapper ParseProtocolImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_logger);
}

LogWrapper ParseProtocolImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_logger);
}

bool ParseProtocolImpl::parseStrToStringValue(
    const std::string &str,
    std::string &val) const
{
    if (str.empty() || (!parseIsFieldValueReferenceSupported())) {
        val = str;
        return true;
    }

    static const char Prefix = common::parseStringRefPrefix();
    if (str[0] == Prefix) {
        return parseStrToString(std::string(str, 1), true, val);
    }

    auto prefixPos = str.find_first_of(Prefix);
    if (prefixPos == std::string::npos) {
        val = str;
        return true;
    }

    assert(0U < prefixPos);
    bool allBackSlashes =
        std::all_of(
            str.begin(), str.begin() + static_cast<std::ptrdiff_t>(prefixPos),
            [](char ch)
            {
                return ch == '\\';
            });
    if (!allBackSlashes) {
        val = str;
        return true;
    }

    val.assign(str, 1, std::string::npos);
    return true;
}

} // namespace parse

} // namespace commsdsl
