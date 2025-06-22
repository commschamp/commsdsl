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

ParseProtocolImpl::ParseProtocolImpl()
  : m_logger(
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
    xmlSetStructuredErrorFunc(this, static_cast<xmlStructuredErrorFunc>(&ParseProtocolImpl::cbXmlErrorFunc));
    m_logger.setMinLevel(m_minLevel);
}

bool ParseProtocolImpl::parse(const std::string& input)
{
    if (m_validated) {
        logError() << "Parsing extra files after validation is not allowed";
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

bool ParseProtocolImpl::validate()
{
    if (m_validated) {
        return true;
    }

    if (m_docs.empty()) {
        logError() << "Cannot validate without any schema files";
        return false;
    }

    for (auto& d : m_docs) {
        if (!validateDoc(d.get())) {
            return false;
        }
    }

    if (!validateAllMessages()) {
        return false;
    }

    m_validated = true;
    return true;
}

ParseProtocolImpl::SchemasAccessList ParseProtocolImpl::schemas() const
{
    SchemasAccessList list;
    for (auto& s : m_schemas) {
        list.push_back(ParseSchema(s.get()));
    }

    return list;
}

ParseSchemaImpl& ParseProtocolImpl::currSchema()
{
    assert(m_currSchema != nullptr);
    return *m_currSchema;
}

const ParseSchemaImpl& ParseProtocolImpl::currSchema() const
{
    assert(m_currSchema != nullptr);
    return *m_currSchema;
}

const ParseFieldImpl* ParseProtocolImpl::findField(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->findField(parsedRef.second, checkRef);
}

const ParseMessageImpl* ParseProtocolImpl::findMessage(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->findMessage(parsedRef.second, checkRef);
}

const ParseInterfaceImpl* ParseProtocolImpl::findInterface(const std::string& ref, bool checkRef) const
{
    assert(!ref.empty());
    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return nullptr;
    }

    return parsedRef.first->findInterface(parsedRef.second, checkRef);
}

bool ParseProtocolImpl::strToEnumValue(
    const std::string& ref,
    std::intmax_t& val,
    bool checkRef) const
{
    if (checkRef) {
        if (!common::isValidRefName(ref)) {
            return false;
        }
    }
    else {
        assert(common::isValidRefName(ref));
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
    auto* field = findField(fieldRefPath, false);
    if ((field == nullptr) || (field->kind() != ParseField::Kind::Enum)) {
        return false;
    }

    auto* enumField = static_cast<const ParseEnumFieldImpl*>(field);
    auto& enumValues = enumField->values();
    auto enumValueIter = enumValues.find(elemName);
    if (enumValueIter == enumValues.end()) {
        return false;
    }

    val = enumValueIter->second.m_value;
    return true;
}

bool ParseProtocolImpl::strToNumeric(
    const std::string& ref,
    bool checkRef,
    std::intmax_t& val,
    bool& isBigUnsigned) const
{
    return
        strToValue(
            ref, checkRef,
            [&val, &isBigUnsigned](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseProtocolImpl::strToFp(
    const std::string& ref,
    bool checkRef,
    double& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToFp(str, val);
            });
}

bool ParseProtocolImpl::strToBool(
    const std::string& ref,
    bool checkRef,
    bool& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToBool(str, val);
            });
}

bool ParseProtocolImpl::strToString(
    const std::string& ref,
    bool checkRef,
    std::string& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToString(str, val);
            });
}

bool ParseProtocolImpl::strToData(
    const std::string& ref,
    bool checkRef,
    std::vector<std::uint8_t>& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const ParseNamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToData(str, val);
            });
}

bool ParseProtocolImpl::isFeatureSupported(unsigned minDslVersion) const
{
    auto currDslVersion = currSchema().dslVersion();
    if (currDslVersion == 0U) {
        return true;
    }

    return minDslVersion <= currDslVersion;
}

bool ParseProtocolImpl::isFeatureDeprecated(unsigned deprecatedVersion) const
{
    auto currDslVersion = currSchema().dslVersion();
    if (currDslVersion == 0U) {
        return false;
    }

    return deprecatedVersion <= currDslVersion;
}

bool ParseProtocolImpl::isPropertySupported(const std::string& name) const
{
    static const std::map<std::string, unsigned> Map = {
        {common::validateMinLengthStr(), 4U},
        {common::defaultValidValueStr(), 4U},
        {common::availableLengthLimitStr(), 4U},
        {common::copyCodeFromStr(), 5U},
        {common::semanticLayerTypeStr(), 5U},
        {common::checksumFromStr(), 5U},
        {common::checksumUntilStr(), 5U},
        {common::termSuffixStr(), 5U},
        {common::missingOnReadFailStr(), 5U},
        {common::missingOnInvalidStr(), 5U},
        {common::reuseCodeStr(), 5U},
        {common::constructStr(), 6U},
        {common::readCondStr(), 6U},
        {common::validCondStr(), 6U},
        {common::constructAsReadCondStr(), 6U},
        {common::constructAsValidCondStr(), 6U},
        {common::fixedValueStr(), 7U},
        {common::copyConstructFromStr(), 7U},
        {common::copyReadCondFromStr(), 7U},
        {common::copyValidCondFromStr(), 7U},
    };

    auto iter = Map.find(name);
    if (iter == Map.end()) {
        return true;
    }

    return isFeatureSupported(iter->second);
}

bool ParseProtocolImpl::isPropertyDeprecated(const std::string& name) const
{
    static const std::map<std::string, unsigned> Map = {
        {common::displayReadOnlyStr(), 7U},
        {common::displayHiddenStr(), 7U},
        {common::displaySpecialsStr(), 7U},
        {common::displayExtModeCtrlStr(), 7U},
        {common::displayIdxReadOnlyHiddenStr(), 7U},
    };

    auto iter = Map.find(name);
    if (iter == Map.end()) {
        return true;
    }

    return isFeatureDeprecated(iter->second);
}

bool ParseProtocolImpl::isFieldValueReferenceSupported() const
{
    return isFeatureSupported(2U);
}

bool ParseProtocolImpl::isSemanticTypeLengthSupported() const
{
    return isFeatureSupported(2U);
}

bool ParseProtocolImpl::isSemanticTypeRefInheritanceSupported() const
{
    return isFeatureSupported(2U);
}

bool ParseProtocolImpl::isNonUniqueSpecialsAllowedSupported() const
{
    return isFeatureSupported(2U);
}

bool ParseProtocolImpl::isFieldAliasSupported() const
{
    return isFeatureSupported(3U);
}

bool ParseProtocolImpl::isCopyFieldsFromBundleSupported() const
{
    return isFeatureSupported(4U);
}

bool ParseProtocolImpl::isOverrideTypeSupported() const
{
    return isFeatureSupported(4U);
}

bool ParseProtocolImpl::isNonIntSemanticTypeLengthSupported() const
{
    return isFeatureSupported(5U);
}

bool ParseProtocolImpl::isMemberReplaceSupported() const
{
    return isFeatureSupported(5U);
}

bool ParseProtocolImpl::isMultiSchemaSupported() const
{
    return isFeatureSupported(5U);
}

bool ParseProtocolImpl::isInterfaceFieldReferenceSupported() const
{
    return isFeatureSupported(6U);
}

bool ParseProtocolImpl::isFailOnInvalidInMessageSupported() const
{
    return isFeatureSupported(6U);
}

bool ParseProtocolImpl::isSizeCompInConditionalsSupported() const
{
    return isFeatureSupported(6U);
}

bool ParseProtocolImpl::isExistsCheckInConditionalsSupported() const
{
    return isFeatureSupported(6U);
}

bool ParseProtocolImpl::isValidValueInStringAndDataSupported() const 
{
    return isFeatureSupported(7U);
}

bool ParseProtocolImpl::isValidateMinLengthForFieldsSupported() const
{
    return isFeatureSupported(7U);
}

bool ParseProtocolImpl::isMessageReuseSupported() const
{
    return isFeatureSupported(7U);
}

bool ParseProtocolImpl::isInterfaceReuseSupported() const
{
    return isMessageReuseSupported();
}

bool ParseProtocolImpl::isValidCondSupportedInCompositeFields() const
{
    return isFeatureSupported(7U);
}

void ParseProtocolImpl::cbXmlErrorFunc(void* userData, const xmlError* err)
{
    reinterpret_cast<ParseProtocolImpl*>(userData)->handleXmlError(err);
}

void ParseProtocolImpl::cbXmlErrorFunc(void* userData, xmlErrorPtr err)
{
    reinterpret_cast<ParseProtocolImpl*>(userData)->handleXmlError(err);
}

void ParseProtocolImpl::handleXmlError(const xmlError* err)
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

    m_logger.setCurrLevel(level);
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
    m_logger.flush();
}

bool ParseProtocolImpl::validateDoc(::xmlDocPtr doc)
{
    auto* root = ::xmlDocGetRootElement(doc);
    if (root == nullptr) {
        logError() << "Failed to fine root element in the schema file \"" << doc->URL << "\"";
        return false;
    }

    static const std::string SchemaName("schema");
    std::string rootName(reinterpret_cast<const char*>(root->name));
    if (rootName != SchemaName) {
        logError() << "Root element of \"" << doc->URL << "\" is not \"" << SchemaName << '\"';
        return false;
    }

    return
        validateSchema(root) &&
        validatePlatforms(root) &&
        validateNamespaces(root);
}

bool ParseProtocolImpl::validateSchema(::xmlNodePtr node)
{
    ParseSchemaImplPtr schema(new ParseSchemaImpl(node, *this));
    if (!schema->processNode()) {
        return false;
    }

    auto schemaName = schema->name();
    if (schemaName.empty() && (m_currSchema != nullptr)) {
        schemaName = m_currSchema->name();
    }

    if (schemaName.empty()) {
        logError() << ParseXmlWrap::logPrefix(schema->getNode()) <<
            "First schema definition must define \"" << common::nameStr() << "\" property.";
        return false;
    }    

    auto schemaIter = 
        std::find_if(
            m_schemas.begin(), m_schemas.end(), 
            [&schemaName](auto& s)
            {
                return schemaName == s->name();
            });

    if (schemaIter == m_schemas.end()) {
        assert(!schema->name().empty());
        if ((!m_schemas.empty()) && (!isMultiSchemaSupported())) {
            logError() << ParseXmlWrap::logPrefix(schema->getNode()) <<
                "Multiple schemas is not supported in the selected " << common::dslVersionStr();
            return false;
        }

        if ((!m_schemas.empty()) && (!m_multipleSchemasEnabled)) {
            logError() << ParseXmlWrap::logPrefix(schema->getNode()) <<
                "Multiple schemas support must be explicitly enabled by the code generator.";
            return false;
        }

        m_schemas.push_back(std::move(schema));
        m_currSchema = m_schemas.back().get();
        return true;        
    } 

    m_currSchema = schemaIter->get();

    auto& props = schema->props();
    auto& origProps = m_currSchema->props();
    for (auto& p : props) {
        auto iter = origProps.find(p.first);
        if ((iter == origProps.end()) ||
            (iter->second != p.second)) {

            logError() << ParseXmlWrap::logPrefix(node) <<
                "Value of \"" << p.first <<
                "\" property of \"" << node->name << "\" element differs from the first one.";
            return false;
        }
    }

    auto& attrs = schema->extraAttributes();
    auto& origAttrs = m_currSchema->extraAttributes();
    for (auto& a : attrs) {
        auto iter = origAttrs.find(a.first);
        if (iter == origAttrs.end()) {
            origAttrs.insert(a);
            continue;
        }

        if (iter->second == a.second) {
            continue;
        }

        logWarning() << ParseXmlWrap::logPrefix(node) <<
            "Value of \"" << a.first <<
            "\" attribubes of \"" << node->name << "\" element differs from the previous one.";
    }

    auto& children = schema->extraChildrenElements();
    auto& origChildren = m_currSchema->extraChildrenElements();
    for (auto& c : children) {
        origChildren.push_back(c);
    }

    return true;
}

bool ParseProtocolImpl::validatePlatforms(::xmlNodePtr root)
{
    auto platforms = ParseXmlWrap::getChildren(root, common::platformsStr());
    for (auto& p : platforms) {
        auto pChildren = ParseXmlWrap::getChildren(p);
        for (auto c : pChildren) {
            assert(c->name != nullptr);
            std::string name(reinterpret_cast<const char*>(c->name));
            if (name != common::platformStr()) {
                logError() << ParseXmlWrap::logPrefix(c) <<
                    "Unexpected element, \"" << common::platformStr() << "\" is expected.";
                return false;
            }

            if (!validateSinglePlatform(c)) {
                return false;
            }
        }
    }

    auto singlePlatforms = ParseXmlWrap::getChildren(root, common::platformStr());
    return std::all_of(
                singlePlatforms.begin(), singlePlatforms.end(),
                [this](auto p)
                {
                    return this->validateSinglePlatform(p);
                });
}

bool ParseProtocolImpl::validateSinglePlatform(::xmlNodePtr node)
{
    static const ParseXmlWrap::NamesList Names = {
        common::nameStr()
    };

    auto props = ParseXmlWrap::parseNodeProps(node);
    if (!ParseXmlWrap::parseChildrenAsProps(node, Names, m_logger, props)) {
        return false;
    }

    auto iter = props.find(common::nameStr());
    if (iter == props.end()) {
        logError() << ParseXmlWrap::logPrefix(node) <<
            "Required property \"" << common::nameStr() << "\" is not defined.";
        return false;
    }

    auto& name = iter->second;
    static const std::string InvalidChars("+-,");
    auto pos = name.find_first_of(InvalidChars);
    if (pos != std::string::npos) {
        logWarning() << ParseXmlWrap::logPrefix(node) <<
            "Invalid platform name (" << name << ".";
        return false;
    }

    if (!currSchema().addPlatform(name)) {
        logWarning() << ParseXmlWrap::logPrefix(node) <<
            "Platform \"" << name << "\" defined more than once.";
        return true;
    }

    return true;
}

bool ParseProtocolImpl::validateNamespaces(::xmlNodePtr root)
{
    auto& namespaces = currSchema().namespaces();
    auto& childrenNames = ParseNamespaceImpl::supportedChildren();
    auto children = ParseXmlWrap::getChildren(root, childrenNames);
    for (auto& c : children) {
        assert(c->name != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto cNameIter = std::find(childrenNames.begin(), childrenNames.end(), cName);
        if (cNameIter == childrenNames.end()) {
            continue;
        }

        if (cName == common::nsStr()) {
            ParseNamespaceImplPtr ns(new ParseNamespaceImpl(c, *this));
            ns->setParent(&currSchema());
            if (!ns->parseProps()) {
                return false;
            }

            auto& nsName = ns->name();
            auto iter = namespaces.find(nsName);
            ParseNamespaceImpl* nsToProcess = nullptr;
            ParseNamespaceImpl* realNs = nullptr;
            do {
                if (iter == namespaces.end()) {
                    currSchema().addNamespace(std::move(ns));
                    iter = namespaces.find(nsName);
                    assert(iter != namespaces.end());
                    nsToProcess = iter->second.get();
                    break;
                }

                nsToProcess = ns.get();
                realNs = iter->second.get();

                if ((!nsToProcess->description().empty()) &&
                    (nsToProcess->description() != realNs->description())) {
                    if (realNs->description().empty()) {
                        realNs->updateDescription(nsToProcess->description());
                    }
                    else {
                        logWarning() << ParseXmlWrap::logPrefix(nsToProcess->getNode()) <<
                            "Description of namespace \"" << nsToProcess->name() << "\" differs to "
                            "one encountered before.";
                    }
                }

                if (!nsToProcess->extraAttributes().empty()) {
                    for (auto& a : nsToProcess->extraAttributes()) {
                        auto attIter = realNs->extraAttributes().find(a.first);
                        if (attIter == realNs->extraAttributes().end()) {
                            realNs->extraAttributes().insert(a);
                        }
                        else if (a.second != attIter->second) {
                            logWarning() << ParseXmlWrap::logPrefix(nsToProcess->getNode()) <<
                                "Value of attribute \"" << a.first << "\" differs to one defined before.";
                        }
                    }
                }

                realNs->extraChildren().insert(realNs->extraChildren().end(), nsToProcess->extraChildren().begin(), nsToProcess->extraChildren().end());

            } while (false);

            assert(iter->second);
            if (!nsToProcess->parseChildren(realNs)) {
                return false;
            }

            continue;
        }

        auto& globalNs = currSchema().defaultNamespace();

        if (!globalNs.processChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseProtocolImpl::validateAllMessages()
{
    return 
        std::all_of(
            m_schemas.begin(), m_schemas.end(),
            [](auto& s)
            {
                return s->validateAllMessages();
            });
}

bool ParseProtocolImpl::strToValue(const std::string& ref, bool checkRef, StrToValueConvertFunc&& func) const
{
    do {
        if (!checkRef) {
            assert(common::isValidRefName(ref));
            break;
        }


        if (!common::isValidRefName(ref)) {
            return false;
        }

    } while (false);

    auto parsedRef = parseExternalRef(ref);
    if ((parsedRef.first == nullptr) || (parsedRef.second.empty())) {
        return false;
    }

    auto& namespaces = parsedRef.first->namespaces();    

    auto redirectToGlobalNs =
        [&func, &parsedRef, &namespaces]() -> bool
        {
            auto iter = namespaces.find(common::emptyString());
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
    if (externalRef[0] != common::schemaRefPrefix()) {
        return std::make_pair(&currSchema(), externalRef);
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
                return schemaName == s->name();
            });

    if (iter == m_schemas.end()) {
        return std::make_pair(nullptr, std::move(restRef));
    }

    return std::make_pair(iter->get(), std::move(restRef));
}

LogWrapper ParseProtocolImpl::logError() const
{
    return commsdsl::parse::logError(m_logger);
}

LogWrapper ParseProtocolImpl::logWarning() const
{
    return commsdsl::parse::logWarning(m_logger);
}

bool ParseProtocolImpl::strToStringValue(
    const std::string &str,
    std::string &val) const
{
    if (str.empty() || (!isFieldValueReferenceSupported())) {
        val = str;
        return true;
    }

    static const char Prefix = common::stringRefPrefix();
    if (str[0] == Prefix) {
        return strToString(std::string(str, 1), true, val);
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
