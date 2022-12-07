//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Schema.h"

#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Protocol.h"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <system_error>

namespace commsdsl
{

namespace gen
{
    
namespace 
{

const unsigned MaxDslVersion = 5U;

} // namespace 

class SchemaImpl
{
public:
    using NamespacesList = Schema::NamespacesList;
    using PlatformNamesList = Schema::PlatformNamesList;

    explicit SchemaImpl(Generator& generator, commsdsl::parse::Schema dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    const commsdsl::parse::Schema& dslObj() const
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

    parse::Endian schemaEndian() const
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

    const Field* findField(const std::string& externalRef) const
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
        }
        return result;        
    }

    Field* findField(const std::string& externalRef)
    {
        return const_cast<Field*>(static_cast<const SchemaImpl*>(this)->findField(externalRef));
    }

    const Message* findMessage(const std::string& externalRef) const
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
        }
        return result;        
    }  

    Message* findMessage(const std::string& externalRef)
    {
        return const_cast<Message*>(static_cast<const SchemaImpl*>(this)->findMessage(externalRef));
    }

    const Frame* findFrame(const std::string& externalRef) const
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
        }
        return result;        
    }

    const Interface* findInterface(const std::string& externalRef) const
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
            m_generator.logger().error("Internal error: unknown external reference: " + externalRef);
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
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

        m_messageIdField = findMessageIdField();
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

    Generator& generator()
    {
        return m_generator;
    }

    const Field* findMessageIdField() const
    {
        for (auto& n : m_namespaces) {
            auto ptr = n->findMessageIdField();
            if (ptr != nullptr) {
                return ptr;
            }
        }
        return nullptr;
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
                                            return f->dslObj().semanticType() == commsdsl::parse::Field::SemanticType::Version;
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

    const Field* getMessageIdField() const
    {
        return m_messageIdField;
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

        if (deprecatedRemoved && (deprecatedSince < commsdsl::parse::Protocol::notYetDeprecated())) {
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

private:
    Generator& m_generator;
    commsdsl::parse::Schema m_dslObj;
    Elem* m_parent = nullptr;
    NamespacesList m_namespaces;
    int m_forcedSchemaVersion = -1;
    const Field* m_messageIdField = nullptr;
    std::string m_mainNamespace;
    std::string m_origNamespace;
    unsigned m_minRemoteVersion = 0U;
    bool m_versionIndependentCodeForced = false;
    bool m_versionDependentCode = false;
}; 

Schema::Schema(Generator& generator, commsdsl::parse::Schema dslObj, Elem* parent) : 
    Base(parent),
    m_impl(std::make_unique<SchemaImpl>(generator, dslObj, this))
{
}

Schema::~Schema() = default;

const commsdsl::parse::Schema& Schema::dslObj() const
{
    return m_impl->dslObj();
}

const std::string& Schema::schemaName() const
{
    return m_impl->schemaName();
}

parse::Endian Schema::schemaEndian() const
{
    return m_impl->schemaEndian();
}

unsigned Schema::schemaVersion() const 
{
    return m_impl->schemaVersion();
}

const Field* Schema::getMessageIdField() const
{
    return m_impl->getMessageIdField();
}

const Field* Schema::findField(const std::string& externalRef) const
{
    auto* field = m_impl->findField(externalRef);
    assert(field->isPrepared());
    return field;
}

Field* Schema::findField(const std::string& externalRef)
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

const Message* Schema::findMessage(const std::string& externalRef) const
{
    return m_impl->findMessage(externalRef);
}

Message* Schema::findMessage(const std::string& externalRef) 
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

const Frame* Schema::findFrame(const std::string& externalRef) const
{
    return m_impl->findFrame(externalRef);
}

const Interface* Schema::findInterface(const std::string& externalRef) const
{
    return m_impl->findInterface(externalRef);
}

const Field* Schema::findMessageIdField() const
{
    return m_impl->findMessageIdField();
}

bool Schema::anyInterfaceHasVersion() const
{
    return m_impl->anyInterfaceHasVersion();
}

Schema::NamespacesAccessList Schema::getAllNamespaces() const
{
    NamespacesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllNamespaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
        result.push_back(n.get());
    }
    return result;
}

Schema::InterfacesAccessList Schema::getAllInterfaces() const
{
    InterfacesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllInterfaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

Schema::MessagesAccessList Schema::getAllMessages() const
{
    MessagesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto subResult = n->getAllMessages();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

Schema::MessagesAccessList Schema::getAllMessagesIdSorted() const
{
    auto result = getAllMessages();
    std::sort(
        result.begin(), result.end(),
        [](auto* msg1, auto* msg2)
        {
            auto id1 = msg1->dslObj().id();
            auto id2 = msg2->dslObj().id();

            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1->dslObj().order() < msg2->dslObj().order();
        });
    return result;
}

Schema::FramesAccessList Schema::getAllFrames() const
{
    FramesAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto nList = n->getAllFrames();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;
}

Schema::FieldsAccessList Schema::getAllFields() const
{
    FieldsAccessList result;
    for (auto& n : m_impl->namespaces()) {
        auto nList = n->getAllFields();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;    
}

bool Schema::createAll()
{
    return m_impl->createAll();
}

bool Schema::prepare()
{
    // Make sure the logger is created

    if (!m_impl->prepare()) {
        return false;
    }

    return prepareImpl();
}

bool Schema::write()
{
    if (!m_impl->write()) {
        return false;
    }
    
    return writeImpl();
}

Schema::NamespacesList& Schema::namespaces()
{
    return m_impl->namespaces();
}

const Schema::NamespacesList& Schema::namespaces() const
{
    return m_impl->namespaces();
}

const Schema::PlatformNamesList& Schema::platformNames() const
{
    return m_impl->platformNames();
}

bool Schema::versionDependentCode() const
{
    return m_impl->versionDependentCode();
}

const std::string& Schema::mainNamespace() const
{
    return m_impl->mainNamespace();
}

const std::string& Schema::origNamespace() const
{
    return m_impl->origNamespace();
}

Namespace* Schema::addDefaultNamespace()
{
    auto& nsList = m_impl->namespaces();
    for (auto& nsPtr : nsList) {
        assert(nsPtr);
        if ((!nsPtr->dslObj().valid()) || nsPtr->dslObj().name().empty()) {
            return nsPtr.get();
        }
    }

    auto iter = nsList.insert(nsList.begin(), m_impl->generator().createNamespace(commsdsl::parse::Namespace(nullptr), this));
    return iter->get();
}

void Schema::forceSchemaVersion(unsigned value)
{
    m_impl->forceSchemaVersion(value);
}

void Schema::setVersionIndependentCodeForced(bool value)
{
    m_impl->setVersionIndependentCodeForced(value);
}

void Schema::setMainNamespaceOverride(const std::string& value)
{
    m_impl->setMainNamespaceOverride(value);
}

void Schema::setMinRemoteVersion(unsigned value)
{
    m_impl->setMinRemoteVersion(value);
}

bool Schema::doesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->doesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool Schema::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->isElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool Schema::isElementDeprecated(unsigned deprecatedSince) const
{
    return deprecatedSince < schemaVersion();
}

void Schema::setAllInterfacesReferenced()
{
    m_impl->setAllInterfacesReferenced();
}

void Schema::setAllMessagesReferenced()
{
    m_impl->setAllMessagesReferenced();
}

bool Schema::hasReferencedMessageIdField() const
{
    return m_impl->hasReferencedMessageIdField();
}

bool Schema::hasAnyReferencedMessage() const
{
    return m_impl->hasAnyReferencedMessage();
}

Elem::Type Schema::elemTypeImpl() const
{
    return Elem::Type_Schema;
}

bool Schema::prepareImpl()
{
    return true;
}

bool Schema::writeImpl()
{
    return true;
}


} // namespace gen

} // namespace commsdsl
