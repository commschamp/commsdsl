//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "ProtocolImpl.h"

#include <iostream>
#include <type_traits>
#include <cassert>
#include <algorithm>
#include <numeric>

#include "XmlWrap.h"
#include "FieldImpl.h"
#include "EnumFieldImpl.h"

namespace commsdsl
{

ProtocolImpl::ProtocolImpl()
  : m_logger(
        [this](ErrorLevel level, const std::string& msg)
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
            static_assert(PrefixCount == ErrorLevel_NumOfValues, "Invalid map");

            std::cerr << Prefix[level] << msg << std::endl;
        }
    )
{
    xmlSetStructuredErrorFunc(this, &ProtocolImpl::cbXmlErrorFunc);
    m_logger.setMinLevel(m_minLevel);
}

bool ProtocolImpl::parse(const std::string& input)
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

bool ProtocolImpl::validate()
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

    auto messageIdsCount = countMessageIds();
    if (1U < messageIdsCount) {
        logError() << "Only single field with \"" << common::messageIdStr() << "\" as semantic type is allowed.";
        return false;
    }

    m_validated = true;
    return true;
}

Schema ProtocolImpl::schema() const
{
    if ((!m_validated) && (!m_schema)) {
        logError() << "Invalid access to schema object.";
        return Schema(nullptr);
    }

    return Schema(m_schema.get());
}

SchemaImpl& ProtocolImpl::schemaImpl()
{
    assert(m_schema);
    return *m_schema;
}

const SchemaImpl& ProtocolImpl::schemaImpl() const
{
    assert(m_schema);
    return *m_schema;
}

ProtocolImpl::NamespacesList ProtocolImpl::namespacesList() const
{
    NamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        result.emplace_back(n.second.get());
    }
    return result;
}

const FieldImpl* ProtocolImpl::findField(const std::string& ref, bool checkRef) const
{
    std::string fieldName;
    auto ns = getNsFromPath(ref, checkRef, fieldName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->findField(fieldName);
}

const MessageImpl* ProtocolImpl::findMessage(const std::string& ref, bool checkRef) const
{
    std::string msgName;
    auto ns = getNsFromPath(ref, checkRef, msgName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->findMessage(msgName);
}

const InterfaceImpl* ProtocolImpl::findInterface(const std::string& ref, bool checkRef) const
{
    std::string name;
    auto ns = getNsFromPath(ref, checkRef, name);
    if (ns == nullptr) {
        return nullptr;
    }

    return ns->findInterface(name);
}

bool ProtocolImpl::strToEnumValue(
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
    auto signedNameSepPos = static_cast<std::intmax_t>(nameSepPos);
    std::string elemName(ref.begin() + signedNameSepPos + 1, ref.end());
    std::string fieldRefPath(ref.begin(), ref.begin() + signedNameSepPos);
    auto* field = findField(fieldRefPath, false);
    if ((field == nullptr) || (field->kind() != Field::Kind::Enum)) {
        return false;
    }

    auto* enumField = static_cast<const EnumFieldImpl*>(field);
    auto& enumValues = enumField->values();
    auto enumValueIter = enumValues.find(elemName);
    if (enumValueIter == enumValues.end()) {
        return false;
    }

    val = enumValueIter->second.m_value;
    return true;
}

bool ProtocolImpl::strToNumeric(
    const std::string& ref,
    bool checkRef,
    std::intmax_t& val,
    bool& isBigUnsigned) const
{
    return
        strToValue(
            ref, checkRef,
            [&val, &isBigUnsigned](const NamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToNumeric(str, val, isBigUnsigned);
            });
}

bool ProtocolImpl::strToFp(
    const std::string& ref,
    bool checkRef,
    double& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const NamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToFp(str, val);
            });
}

bool ProtocolImpl::strToBool(
    const std::string& ref,
    bool checkRef,
    bool& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const NamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToBool(str, val);
            });
}

bool ProtocolImpl::strToString(
    const std::string& ref,
    bool checkRef,
    std::string& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const NamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToString(str, val);
            });
}

bool ProtocolImpl::strToData(
    const std::string& ref,
    bool checkRef,
    std::vector<std::uint8_t>& val) const
{
    return
        strToValue(
            ref, checkRef,
            [&val](const NamespaceImpl& ns, const std::string& str) -> bool
            {
               return ns.strToData(str, val);
            });
}

ProtocolImpl::MessagesList ProtocolImpl::allMessages() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->messages().size();
            });

    NamespaceImpl::MessagesList messages;
    messages.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->messagesList();
        messages.insert(messages.end(), nsMsgs.begin(), nsMsgs.end());
    }

    std::sort(
        messages.begin(), messages.end(),
        [](const auto& msg1, const auto& msg2)
        {
            assert(msg1.valid());
            assert(msg2.valid());
            auto id1 = msg1.id();
            auto id2 = msg2.id();
            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1.order() < msg2.order();
        });

    return messages;
}

bool ProtocolImpl::isFeatureSupported(unsigned minDslVersion) const
{
    assert(m_schema);
    auto currDslVersion = m_schema->dslVersion();
    if (currDslVersion == 0U) {
        return true;
    }

    return minDslVersion <= currDslVersion;
}

bool ProtocolImpl::isFieldValueReferenceSupported() const
{
    return isFeatureSupported(2U);
}

bool ProtocolImpl::isSemanticTypeLengthSupported() const
{
    return isFeatureSupported(2U);
}

bool ProtocolImpl::isSemanticTypeRefInheritanceSupported() const
{
    return isFeatureSupported(2U);
}

bool ProtocolImpl::isNonUniqueSpecialsAllowedSupported() const
{
    return isFeatureSupported(2U);
}

bool ProtocolImpl::isFieldAliasSupported() const
{
    return isFeatureSupported(3U);
}

void ProtocolImpl::cbXmlErrorFunc(void* userData, xmlErrorPtr err)
{
    reinterpret_cast<ProtocolImpl*>(userData)->handleXmlError(err);
}

void ProtocolImpl::handleXmlError(xmlErrorPtr err)
{
    static const ErrorLevel Map[] = {
        /* XML_ERR_NONE */ ErrorLevel_Debug,
        /* XML_ERR_WARNING */ ErrorLevel_Warning,
        /* XML_ERR_ERROR */ ErrorLevel_Error,
        /* XML_ERR_FATAL */ ErrorLevel_Error
    };

    static_assert(XML_ERR_NONE == 0, "Invalid assumption");
    static_assert(XML_ERR_FATAL == 3, "Invalid assumption");

    ErrorLevel level = ErrorLevel_Error;
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

bool ProtocolImpl::validateDoc(::xmlDocPtr doc)
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

bool ProtocolImpl::validateSchema(::xmlNodePtr node)
{
    SchemaImplPtr schema(new SchemaImpl(node, *this));
    if (!schema->processNode()) {
        return false;
    }

    if (!m_schema) {
        m_schema = std::move(schema);

        if (m_schema->name().empty()) {
            logError() << XmlWrap::logPrefix(m_schema->getNode()) <<
                "First schema definition must define \"" << common::nameStr << "\" property.";
            return false;
        }
        return true;
    }

    auto& props = schema->props();
    auto& origProps = m_schema->props();
    for (auto& p : props) {
        auto iter = origProps.find(p.first);
        if ((iter == origProps.end()) ||
            (iter->second != p.second)) {

            logError() << XmlWrap::logPrefix(node) <<
                "Value of \"" << p.first <<
                "\" property of \"" << node->name << "\" element differs from the first one.";
            return false;
        }
    }

    auto& attrs = schema->extraAttributes();
    auto& origAttrs = m_schema->extraAttributes();
    for (auto& a : attrs) {
        auto iter = origAttrs.find(a.first);
        if (iter == origAttrs.end()) {
            origAttrs.insert(a);
            continue;
        }

        if (iter->second == a.second) {
            continue;
        }

        logWarning() << XmlWrap::logPrefix(node) <<
            "Value of \"" << a.first <<
            "\" attribubes of \"" << node->name << "\" element differs from the previous one.";
    }

    auto& children = schema->extraChildrenElements();
    auto& origChildren = m_schema->extraChildrenElements();
    for (auto& c : children) {
        origChildren.push_back(c);
    }

    return true;
}

bool ProtocolImpl::validatePlatforms(::xmlNodePtr root)
{
    auto platforms = XmlWrap::getChildren(root, common::platformsStr());
    for (auto& p : platforms) {
        auto pChildren = XmlWrap::getChildren(p);
        for (auto c : pChildren) {
            assert(c->name != nullptr);
            std::string name(reinterpret_cast<const char*>(c->name));
            if (name != common::platformStr()) {
                logError() << XmlWrap::logPrefix(c) <<
                    "Unexpected element, \"" << common::platformStr() << "\" is expected.";
                return false;
            }

            if (!validateSinglePlatform(c)) {
                return false;
            }
        }
    }

    auto singlePlatforms = XmlWrap::getChildren(root, common::platformStr());
    return std::all_of(
                singlePlatforms.begin(), singlePlatforms.end(),
                [this](auto p)
                {
                    return this->validateSinglePlatform(p);
                });
}

bool ProtocolImpl::validateSinglePlatform(::xmlNodePtr node)
{
    static const XmlWrap::NamesList Names = {
        common::nameStr()
    };

    auto props = XmlWrap::parseNodeProps(node);
    if (!XmlWrap::parseChildrenAsProps(node, Names, m_logger, props)) {
        return false;
    }

    auto iter = props.find(common::nameStr());
    if (iter == props.end()) {
        logError() << XmlWrap::logPrefix(node) <<
            "Required property \"" << common::nameStr() << "\" is not defined.";
        return false;
    }

    auto& name = iter->second;
    auto platIter = std::lower_bound(m_platforms.begin(), m_platforms.end(), name);
    if ((platIter != m_platforms.end()) && (*platIter == name)) {
        logWarning() << XmlWrap::logPrefix(node) <<
            "Platform \"" << name << "\" defined more than once.";
        return true;
    }

    static const std::string InvalidChars("+-,");
    auto pos = name.find_first_of(InvalidChars);
    if (pos != std::string::npos) {
        logWarning() << XmlWrap::logPrefix(node) <<
            "Invalid platform name (" << name << ".";
        return false;
    }

    m_platforms.insert(platIter, name);
    return true;
}

bool ProtocolImpl::validateNamespaces(::xmlNodePtr root)
{
    auto& childrenNames = NamespaceImpl::supportedChildren();
    auto children = XmlWrap::getChildren(root, childrenNames);
    for (auto& c : children) {
        assert(c->name != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto cNameIter = std::find(childrenNames.begin(), childrenNames.end(), cName);
        if (cNameIter == childrenNames.end()) {
            continue;
        }

        if (cName == common::nsStr()) {
            NamespaceImplPtr ns(new NamespaceImpl(c, *this));
            if (!ns->parseProps()) {
                return false;
            }

            auto& nsName = ns->name();
            auto iter = m_namespaces.find(nsName);
            NamespaceImpl* nsToProcess = nullptr;
            NamespaceImpl* realNs = nullptr;
            do {
                if (iter == m_namespaces.end()) {
                    m_namespaces.insert(std::make_pair(nsName, std::move(ns)));
                    iter = m_namespaces.find(nsName);
                    assert(iter != m_namespaces.end());
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
                        logWarning() << XmlWrap::logPrefix(nsToProcess->getNode()) <<
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
                            logWarning() << XmlWrap::logPrefix(nsToProcess->getNode()) <<
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

        auto& globalNsPtr = m_namespaces[common::emptyString()]; // create if needed
        if (!globalNsPtr) {
            globalNsPtr.reset(new NamespaceImpl(nullptr, *this));
        }

        if (!globalNsPtr->processChild(c)) {
            return false;
        }
    }

    return true;
}

bool ProtocolImpl::validateAllMessages()
{
    assert(m_schema);
    bool allowNonUniquIds = m_schema->nonUniqueMsgIdAllowed();
    auto allMsgs = allMessages();
    if (allMsgs.empty()) {
        return true;
    }

    for (auto iter = allMsgs.begin(); iter != (allMsgs.end() - 1); ++iter) {
        auto nextIter = iter + 1;
        assert(nextIter != allMsgs.end());

        assert(iter->valid());
        assert(nextIter->valid());
        if (iter->id() != nextIter->id()) {
            continue;
        }

        if (!allowNonUniquIds) {
            logError() << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same id.";
            return false;
        }

        if (iter->order() == nextIter->order()) {
            logError() << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same \"" <<
                          common::idStr() << "\" and \"" << common::orderStr() << "\" values.";
            return false;
        }

        assert(iter->order() < nextIter->order());
    }

    return true;
}

unsigned ProtocolImpl::countMessageIds() const
{
    return
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), unsigned(0U),
            [](unsigned soFar, auto& n)
            {
                return soFar + n.second->countMessageIds();
            });
}

const NamespaceImpl* ProtocolImpl::getNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const
{
    if (checkRef) {
        if (!common::isValidRefName(ref)) {
            logInfo(m_logger) << "Invalid ref name: " << ref;
            return nullptr;
        }
    }
    else {
        assert(common::isValidRefName(ref));
    }


    auto nameSepPos = ref.find_last_of('.');
    const NamespaceImpl* ns = nullptr;
    do {
        if (nameSepPos == std::string::npos) {
            auto iter = m_namespaces.find(common::emptyString());
            if (iter == m_namespaces.end()) {
                return nullptr;
            }

            remName = ref;
            ns = iter->second.get();
            assert(ns != nullptr);
            break;
        }
        
        auto signedNameSepPos = static_cast<std::intmax_t>(nameSepPos);
        remName.assign(ref.begin() + signedNameSepPos + 1, ref.end());
        std::size_t nsNamePos = 0;
        assert(nameSepPos != std::string::npos);
        while (nsNamePos < nameSepPos) {
            auto nextDotPos = ref.find_first_of('.', nsNamePos);
            assert(nextDotPos != std::string::npos);
            std::string nsName(
                    ref.begin() + static_cast<std::intmax_t>(nsNamePos), 
                    ref.begin() + static_cast<std::intmax_t>(nextDotPos));
            if (nsName.empty()) {
                return nullptr;
            }

            auto* nsMap = &m_namespaces;
            if (ns != nullptr) {
                nsMap = &(ns->namespacesMap());
            }

            auto iter = nsMap->find(nsName);
            if (iter == nsMap->end()) {
                return nullptr;
            }

            assert(iter->second);
            ns = iter->second.get();
            nsNamePos = nextDotPos + 1;
        }

    } while (false);

    assert(ns != nullptr);
    return ns;
}

bool ProtocolImpl::strToValue(const std::string& ref, bool checkRef, StrToValueConvertFunc&& func) const
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

    auto redirectToGlobalNs =
        [this, &func, &ref]() -> bool
        {
            auto iter = m_namespaces.find(common::emptyString());
            if (iter == m_namespaces.end()) {
                return false;
            }
            assert(iter->second);
            return func(*iter->second, ref);
        };

    auto firstDotPos = ref.find_first_of('.');
    if (firstDotPos == std::string::npos) {
        return redirectToGlobalNs();
    }

    std::string ns(ref, 0, firstDotPos);
    assert(!ns.empty());
    auto nsIter = m_namespaces.find(ns);
    if (nsIter == m_namespaces.end()) {
        return redirectToGlobalNs();
    }

    assert(nsIter->second);
    std::string subRef(ref, firstDotPos + 1);
    return func(*nsIter->second, subRef);
}

LogWrapper ProtocolImpl::logError() const
{
    return commsdsl::logError(m_logger);
}

LogWrapper ProtocolImpl::logWarning() const
{
    return commsdsl::logWarning(m_logger);
}

bool ProtocolImpl::strToStringValue(
    const std::string &str,
    std::string &val) const
{
    if (str.empty() || (!isFieldValueReferenceSupported())) {
        val = str;
        return true;
    }

    static const char Prefix = '^';
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
            str.begin(), str.begin() + static_cast<std::intmax_t>(prefixPos),
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

} // namespace commsdsl
