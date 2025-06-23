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

#include "commsdsl/gen/GenSchema.h"

#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/ParseProtocol.h"
#include "commsdsl/version.h"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <system_error>

namespace commsdsl
{

namespace gen
{
    
namespace 
{

const unsigned MaxDslVersion = COMMSDSL_MAJOR_VERSION;

} // namespace 

class GenSchemaImpl
{
public:
    using NamespacesList = GenSchema::NamespacesList;
    using PlatformNamesList = GenSchema::PlatformNamesList;
    using FieldsAccessList = GenSchema::FieldsAccessList;

    explicit GenSchemaImpl(GenGenerator& generator, commsdsl::parse::ParseSchema dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    const commsdsl::parse::ParseSchema& dslObj() const
    {
        return m_dslObj;
    }

    NamespacesList& namespaces()
    {
        return m_namespaces;
    }

    const NamespacesList& namespaces() const
    {
        return m_namespaces;
    }    

    const std::string& schemaName() const
    {
        return m_dslObj.name();
    }

    parse::ParseEndian schemaEndian() const
    {
        return m_dslObj.endian();
    }

    unsigned schemaVersion() const
    {
        if (0 <= m_forcedSchemaVersion) {
            return static_cast<unsigned>(m_forcedSchemaVersion);
        }

        return m_dslObj.version();
    }

    const GenField* findField(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto pos = externalRef.find_first_of('.');
        std::string nsName;
        if (pos != std::string::npos) {
            nsName.assign(externalRef.begin(), externalRef.begin() + pos);
        }

        auto nsIter =
            std::lower_bound(
                m_namespaces.begin(), m_namespaces.end(), nsName,
                [](auto& ns, const std::string& n)
                {
                    return ns->name() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->findField(remStr);
        if (result == nullptr) {
            m_generator.logger().error("Internal error: unknown external reference \"" + externalRef + "\" in schema " + m_dslObj.name());
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }

    GenField* findField(const std::string& externalRef)
    {
        return const_cast<GenField*>(static_cast<const GenSchemaImpl*>(this)->findField(externalRef));
    }

    const GenMessage* findMessage(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto pos = externalRef.find_first_of('.');
        std::string nsName;
        if (pos != std::string::npos) {
            nsName.assign(externalRef.begin(), externalRef.begin() + pos);
        }

        auto nsIter =
            std::lower_bound(
                m_namespaces.begin(), m_namespaces.end(), nsName,
                [](auto& ns, const std::string& n)
                {
                    return ns->name() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->findMessage(remStr);
        if (result == nullptr) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }  

    GenMessage* findMessage(const std::string& externalRef)
    {
        return const_cast<GenMessage*>(static_cast<const GenSchemaImpl*>(this)->findMessage(externalRef));
    }

    const GenFrame* findFrame(const std::string& externalRef) const
    {
        assert(!externalRef.empty());
        auto pos = externalRef.find_first_of('.');
        std::string nsName;
        if (pos != std::string::npos) {
            nsName.assign(externalRef.begin(), externalRef.begin() + pos);
        }

        auto nsIter =
            std::lower_bound(
                m_namespaces.begin(), m_namespaces.end(), nsName,
                [](auto& ns, const std::string& n)
                {
                    return ns->name() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->findFrame(remStr);
        if (result == nullptr) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }

    const GenInterface* findInterface(const std::string& externalRef) const
    {
        auto pos = externalRef.find_first_of('.');
        std::string nsName;
        if (pos != std::string::npos) {
            nsName.assign(externalRef.begin(), externalRef.begin() + pos);
        }

        auto nsIter =
            std::lower_bound(
                m_namespaces.begin(), m_namespaces.end(), nsName,
                [](auto& ns, const std::string& n)
                {
                    return ns->name() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->findInterface(remStr);
        if (result == nullptr) {
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }                

    bool createAll()
    {
        auto namespaces = m_dslObj.namespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.createNamespace(n, m_parent);
            if (!ptr->createAll()) {
                return false;
            }
            assert(ptr);
            m_namespaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepare()
    {
        auto dslVersion = m_dslObj.dslVersion();
        if (MaxDslVersion < dslVersion) {
            m_generator.logger().error(
                "Required DSL version is too big (" + std::to_string(dslVersion) +
                "), upgrade your code generator.");
            return false;
        }

        auto parsedSchemaVersion = m_dslObj.version();
        if ((0 <= m_forcedSchemaVersion) && 
            (parsedSchemaVersion < static_cast<decltype(parsedSchemaVersion)>(m_forcedSchemaVersion))) {
            m_generator.logger().error("Cannot force version to be greater than " + util::numToString(parsedSchemaVersion));
            return false;
        }   

        if (!m_versionIndependentCodeForced) {
            m_versionDependentCode = anyInterfaceHasVersion();
        }       

        assert(!m_dslObj.name().empty());
        m_origNamespace = util::strToName(m_dslObj.name());
        if (m_mainNamespace.empty()) {
            m_mainNamespace = m_origNamespace;
        }              

        bool namespacesResult = 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    assert(n);
                    return n->prepare();
                });

        if (!namespacesResult) {
            return false;
        }

        m_messageIdFields = findMessageIdFields();
        return true;
    }

    bool write()
    {
        return 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->write();
                });
    }

    GenGenerator& generator()
    {
        return m_generator;
    }

    FieldsAccessList findMessageIdFields() const
    {
        FieldsAccessList result;
        for (auto& n : m_namespaces) {
            auto nsResult = n->findMessageIdFields();
            std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
        }
        return result;
    }  

    bool anyInterfaceHasVersion() const
    {
        return
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    auto interfaces = n->getAllInterfaces();

                    return 
                        std::any_of(
                            interfaces.begin(), interfaces.end(),
                            [](auto& i)
                            {

                                auto& fields = i->fields();
                                return
                                    std::any_of(
                                        fields.begin(), fields.end(),
                                        [](auto& f)
                                        {
                                            return f->dslObj().semanticType() == commsdsl::parse::ParseField::SemanticType::Version;
                                        });

                            });
                });
    }      

    void forceSchemaVersion(unsigned value)
    {
        m_forcedSchemaVersion = static_cast<decltype(m_forcedSchemaVersion)>(value);
    }    

    const PlatformNamesList& platformNames()
    {
        return m_dslObj.platforms();
    }

    void setVersionIndependentCodeForced(bool value)
    {
        m_versionIndependentCodeForced = value;
    }    

    bool versionDependentCode() const
    {
        return m_versionDependentCode;
    }    

    const std::string& mainNamespace() const
    {
        return m_mainNamespace;
    }

    const std::string& origNamespace() const
    {
        return m_origNamespace;
    }    

    FieldsAccessList getAllMessageIdFields() const
    {
        return m_messageIdFields;
    }    

    void setMainNamespaceOverride(const std::string& value)
    {
        m_mainNamespace = value;
    }
    
    bool doesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const
    {
        if (schemaVersion() < sinceVersion) {
            return false;
        }

        if (deprecatedRemoved && (deprecatedSince <= m_minRemoteVersion)) {
            return false;
        }

        return true;
    }

    bool isElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const
    {
        if (m_minRemoteVersion < sinceVersion) {
            return true;
        }

        if (deprecatedRemoved && (deprecatedSince < commsdsl::parse::ParseProtocol::notYetDeprecated())) {
            return true;
        }

        return false;
    }

    void setMinRemoteVersion(unsigned value)
    {
        m_minRemoteVersion = value;
    }

    void setAllInterfacesReferenced()
    {
        for (auto& nPtr : m_namespaces) {
            nPtr->setAllInterfacesReferenced();
        }
    }    

    void setAllMessagesReferenced()
    {
        for (auto& nPtr : m_namespaces) {
            nPtr->setAllMessagesReferenced();
        }
    }

    bool hasReferencedMessageIdField() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->hasReferencedMessageIdField();
                });
    }

    bool hasAnyReferencedMessage() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->hasAnyReferencedMessage();
                });
    }

    bool hasAnyReferencedComponent() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->hasAnyReferencedComponent();
                });
    }    

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseSchema m_dslObj;
    GenElem* m_parent = nullptr;
    NamespacesList m_namespaces;
    int m_forcedSchemaVersion = -1;
    FieldsAccessList m_messageIdFields;
    std::string m_mainNamespace;
    std::string m_origNamespace;
    unsigned m_minRemoteVersion = 0U;
    bool m_versionIndependentCodeForced = false;
    bool m_versionDependentCode = false;
}; 

GenSchema::GenSchema(GenGenerator& generator, commsdsl::parse::ParseSchema dslObj, GenElem* parent) : 
    Base(parent),
    m_impl(std::make_unique<GenSchemaImpl>(generator, dslObj, this))
{
}

GenSchema::~GenSchema() = default;

const commsdsl::parse::ParseSchema& GenSchema::dslObj() const
{
    return m_impl->dslObj();
}

const std::string& GenSchema::schemaName() const
{
    return m_impl->schemaName();
}

parse::ParseEndian GenSchema::schemaEndian() const
{
    return m_impl->schemaEndian();
}

unsigned GenSchema::schemaVersion() const 
{
    return m_impl->schemaVersion();
}

GenSchema::FieldsAccessList GenSchema::getAllMessageIdFields() const
{
    return m_impl->getAllMessageIdFields();
}

const GenField* GenSchema::findField(const std::string& externalRef) const
{
    auto* field = m_impl->findField(externalRef);
    assert(field->isPrepared());
    return field;
}

GenField* GenSchema::findField(const std::string& externalRef)
{
    auto* field = m_impl->findField(externalRef);
    do {
        if (field->isPrepared()) {
            break;
        }    

        if (field->prepare()) {
            break;
        }
         
        m_impl->generator().logger().warning("Failed to prepare field: " + field->dslObj().externalRef());
        field = nullptr;
    } while (false);
    return field;
}

const GenMessage* GenSchema::findMessage(const std::string& externalRef) const
{
    return m_impl->findMessage(externalRef);
}

GenMessage* GenSchema::findMessage(const std::string& externalRef) 
{
    auto* msg = m_impl->findMessage(externalRef);
    do {
        if (msg->isPrepared()) {
            break;
        }

        if (msg->prepare()) {
            break;
        }

        m_impl->generator().logger().warning("Failed to prepare message: " + msg->dslObj().externalRef());
        msg = nullptr;
    } while (false);
    return msg;
}

const GenFrame* GenSchema::findFrame(const std::string& externalRef) const
{
    return m_impl->findFrame(externalRef);
}

const GenInterface* GenSchema::findInterface(const std::string& externalRef) const
{
    return m_impl->findInterface(externalRef);
}

bool GenSchema::anyInterfaceHasVersion() const
{
    return m_impl->anyInterfaceHasVersion();
}

GenSchema::NamespacesAccessList GenSchema::getAllNamespaces() const
{
    NamespacesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllNamespaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
        result.push_back(n.get());
    }
    return result;
}

GenSchema::InterfacesAccessList GenSchema::getAllInterfaces() const
{
    InterfacesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllInterfaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

GenSchema::MessagesAccessList GenSchema::getAllMessages() const
{
    MessagesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllMessages();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

GenSchema::MessagesAccessList GenSchema::getAllMessagesIdSorted() const
{
    auto result = getAllMessages();
    GenGenerator::sortMessages(result);
    return result;
}

GenSchema::FramesAccessList GenSchema::getAllFrames() const
{
    FramesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto nList = n->getAllFrames();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;
}

GenSchema::FieldsAccessList GenSchema::getAllFields() const
{
    FieldsAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto nList = n->getAllFields();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;    
}

bool GenSchema::createAll()
{
    return m_impl->createAll();
}

bool GenSchema::prepare()
{
    // Make sure the logger is created

    if (!m_impl->prepare()) {
        return false;
    }

    return prepareImpl();
}

bool GenSchema::write()
{
    if (!m_impl->write()) {
        return false;
    }
    
    return writeImpl();
}

GenSchema::NamespacesList& GenSchema::namespaces()
{
    return m_impl->namespaces();
}

const GenSchema::NamespacesList& GenSchema::namespaces() const
{
    return m_impl->namespaces();
}

const GenSchema::PlatformNamesList& GenSchema::platformNames() const
{
    return m_impl->platformNames();
}

bool GenSchema::versionDependentCode() const
{
    return m_impl->versionDependentCode();
}

const std::string& GenSchema::mainNamespace() const
{
    return m_impl->mainNamespace();
}

const std::string& GenSchema::origNamespace() const
{
    return m_impl->origNamespace();
}

GenNamespace* GenSchema::addDefaultNamespace()
{
    auto& nsList = m_impl->namespaces();
    for (auto& nsPtr : nsList) {
        assert(nsPtr);
        if ((!nsPtr->dslObj().valid()) || nsPtr->dslObj().name().empty()) {
            return nsPtr.get();
        }
    }

    auto iter = nsList.insert(nsList.begin(), m_impl->generator().createNamespace(commsdsl::parse::ParseNamespace(nullptr), this));
    return iter->get();
}

void GenSchema::forceSchemaVersion(unsigned value)
{
    m_impl->forceSchemaVersion(value);
}

void GenSchema::setVersionIndependentCodeForced(bool value)
{
    m_impl->setVersionIndependentCodeForced(value);
}

void GenSchema::setMainNamespaceOverride(const std::string& value)
{
    m_impl->setMainNamespaceOverride(value);
}

void GenSchema::setMinRemoteVersion(unsigned value)
{
    m_impl->setMinRemoteVersion(value);
}

bool GenSchema::doesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->doesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenSchema::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->isElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenSchema::isElementDeprecated(unsigned deprecatedSince) const
{
    return deprecatedSince < schemaVersion();
}

void GenSchema::setAllInterfacesReferenced()
{
    m_impl->setAllInterfacesReferenced();
}

void GenSchema::setAllMessagesReferenced()
{
    m_impl->setAllMessagesReferenced();
}

bool GenSchema::hasReferencedMessageIdField() const
{
    return m_impl->hasReferencedMessageIdField();
}

bool GenSchema::hasAnyReferencedMessage() const
{
    return m_impl->hasAnyReferencedMessage();
}

bool GenSchema::hasAnyReferencedComponent() const
{
    return m_impl->hasAnyReferencedComponent();
}

GenElem::Type GenSchema::elemTypeImpl() const
{
    return GenElem::Type_Schema;
}

bool GenSchema::prepareImpl()
{
    return true;
}

bool GenSchema::writeImpl()
{
    return true;
}


} // namespace gen

} // namespace commsdsl
