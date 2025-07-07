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

#include "commsdsl/version.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/ParseProtocol.h"

#include <algorithm>
#include <cassert>
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
    using ParseSchema = GenSchema::ParseSchema;
    using ParseField = commsdsl::parse::ParseField;
    using ParseProtocol = commsdsl::parse::ParseProtocol;

    using GenNamespacesList = GenSchema::GenNamespacesList;
    using GenPlatformNamesList = GenSchema::GenPlatformNamesList;
    using GenFieldsAccessList = GenSchema::GenFieldsAccessList;

    explicit GenSchemaImpl(GenGenerator& generator, ParseSchema parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    const ParseSchema& genParseObj() const
    {
        return m_parseObj;
    }

    GenNamespacesList& genNamespaces()
    {
        return m_namespaces;
    }

    const GenNamespacesList& genNamespaces() const
    {
        return m_namespaces;
    }    

    const std::string& genSchemaName() const
    {
        return m_parseObj.parseName();
    }

    parse::ParseEndian genSchemaEndian() const
    {
        return m_parseObj.parseEndian();
    }

    unsigned genSchemaVersion() const
    {
        if (0 <= m_forcedSchemaVersion) {
            return static_cast<unsigned>(m_forcedSchemaVersion);
        }

        return m_parseObj.parseVersion();
    }

    const GenField* genFindField(const std::string& externalRef) const
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
                    return ns->genName() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->genName() != nsName)) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->genFindField(remStr);
        if (result == nullptr) {
            m_generator.genLogger().genError("Internal error: unknown external reference \"" + externalRef + "\" in schema " + m_parseObj.parseName());
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }

    GenField* genFindField(const std::string& externalRef)
    {
        return const_cast<GenField*>(static_cast<const GenSchemaImpl*>(this)->genFindField(externalRef));
    }

    const GenMessage* genGindMessage(const std::string& externalRef) const
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
                    return ns->genName() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->genName() != nsName)) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->genGindMessage(remStr);
        if (result == nullptr) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }  

    GenMessage* genGindMessage(const std::string& externalRef)
    {
        return const_cast<GenMessage*>(static_cast<const GenSchemaImpl*>(this)->genGindMessage(externalRef));
    }

    const GenFrame* genFindFrame(const std::string& externalRef) const
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
                    return ns->genName() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->genName() != nsName)) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->genFindFrame(remStr);
        if (result == nullptr) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }

    const GenInterface* genFindInterface(const std::string& externalRef) const
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
                    return ns->genName() < n;
                });

        if ((nsIter == m_namespaces.end()) || ((*nsIter)->genName() != nsName)) {
            return nullptr;
        }

        std::size_t fromPos = 0U;
        if (pos != std::string::npos) {
            fromPos = pos + 1U;
        }
        std::string remStr(externalRef, fromPos);
        auto result = (*nsIter)->genFindInterface(remStr);
        if (result == nullptr) {
            m_generator.genLogger().genError("Internal error: unknown external reference: " + externalRef);
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
        }
        return result;        
    }                

    bool genCreateAll()
    {
        auto namespaces = m_parseObj.parseNamespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.genCreateNamespace(n, m_parent);
            if (!ptr->genCreateAll()) {
                return false;
            }
            assert(ptr);
            m_namespaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool genPrepare()
    {
        auto dslVersion = m_parseObj.parseDslVersion();
        if (MaxDslVersion < dslVersion) {
            m_generator.genLogger().genError(
                "Required DSL version is too big (" + std::to_string(dslVersion) +
                "), upgrade your code generator.");
            return false;
        }

        auto parsedSchemaVersion = m_parseObj.parseVersion();
        if ((0 <= m_forcedSchemaVersion) && 
            (parsedSchemaVersion < static_cast<decltype(parsedSchemaVersion)>(m_forcedSchemaVersion))) {
            m_generator.genLogger().genError("Cannot force version to be greater than " + util::genNumToString(parsedSchemaVersion));
            return false;
        }   

        if (!m_versionIndependentCodeForced) {
            m_versionDependentCode = genAnyInterfaceHasVersion();
        }       

        assert(!m_parseObj.parseName().empty());
        m_origNamespace = util::genStrToName(m_parseObj.parseName());
        if (m_mainNamespace.empty()) {
            m_mainNamespace = m_origNamespace;
        }              

        bool namespacesResult = 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    assert(n);
                    return n->genPrepare();
                });

        if (!namespacesResult) {
            return false;
        }

        m_messageIdFields = genFindMessageIdFields();
        return true;
    }

    bool genWrite()
    {
        return 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->genWrite();
                });
    }

    GenGenerator& genGenerator()
    {
        return m_generator;
    }

    GenFieldsAccessList genFindMessageIdFields() const
    {
        GenFieldsAccessList result;
        for (auto& n : m_namespaces) {
            auto nsResult = n->genFindMessageIdFields();
            std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
        }
        return result;
    }  

    bool genAnyInterfaceHasVersion() const
    {
        return
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    auto interfaces = n->genGetAllInterfaces();

                    return 
                        std::any_of(
                            interfaces.begin(), interfaces.end(),
                            [](auto& i)
                            {

                                auto& fields = i->genFields();
                                return
                                    std::any_of(
                                        fields.begin(), fields.end(),
                                        [](auto& f)
                                        {
                                            return f->genParseObj().parseSemanticType() == ParseField::ParseSemanticType::Version;
                                        });

                            });
                });
    }      

    void genForceSchemaVersion(unsigned value)
    {
        m_forcedSchemaVersion = static_cast<decltype(m_forcedSchemaVersion)>(value);
    }    

    const GenPlatformNamesList& platformNames()
    {
        return m_parseObj.parsePlatforms();
    }

    void genSetVersionIndependentCodeForced(bool value)
    {
        m_versionIndependentCodeForced = value;
    }    

    bool genVersionDependentCode() const
    {
        return m_versionDependentCode;
    }    

    const std::string& genMainNamespace() const
    {
        return m_mainNamespace;
    }

    const std::string& genOrigNamespace() const
    {
        return m_origNamespace;
    }    

    GenFieldsAccessList genGetAllMessageIdFields() const
    {
        return m_messageIdFields;
    }    

    void genSetMainNamespaceOverride(const std::string& value)
    {
        m_mainNamespace = value;
    }
    
    bool genDoesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const
    {
        if (genSchemaVersion() < sinceVersion) {
            return false;
        }

        if (deprecatedRemoved && (deprecatedSince <= m_minRemoteVersion)) {
            return false;
        }

        return true;
    }

    bool genIsElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const
    {
        if (m_minRemoteVersion < sinceVersion) {
            return true;
        }

        if (deprecatedRemoved && (deprecatedSince < ParseProtocol::parseNotYetDeprecated())) {
            return true;
        }

        return false;
    }

    void genSetMinRemoteVersion(unsigned value)
    {
        m_minRemoteVersion = value;
    }

    void genSetAllInterfacesReferenced()
    {
        for (auto& nPtr : m_namespaces) {
            nPtr->genSetAllInterfacesReferenced();
        }
    }    

    void genSetAllMessagesReferenced()
    {
        for (auto& nPtr : m_namespaces) {
            nPtr->genSetAllMessagesReferenced();
        }
    }

    bool genHasReferencedMessageIdField() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->genHasReferencedMessageIdField();
                });
    }

    bool genHasAnyReferencedMessage() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->genHasAnyReferencedMessage();
                });
    }

    bool genHasAnyReferencedComponent() const
    {
        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->genHasAnyReferencedComponent();
                });
    }    

private:
    GenGenerator& m_generator;
    ParseSchema m_parseObj;
    GenElem* m_parent = nullptr;
    GenNamespacesList m_namespaces;
    int m_forcedSchemaVersion = -1;
    GenFieldsAccessList m_messageIdFields;
    std::string m_mainNamespace;
    std::string m_origNamespace;
    unsigned m_minRemoteVersion = 0U;
    bool m_versionIndependentCodeForced = false;
    bool m_versionDependentCode = false;
}; 

GenSchema::GenSchema(GenGenerator& generator, ParseSchema parseObj, GenElem* parent) : 
    Base(parent),
    m_impl(std::make_unique<GenSchemaImpl>(generator, parseObj, this))
{
}

GenSchema::~GenSchema() = default;

const GenSchema::ParseSchema& GenSchema::genParseObj() const
{
    return m_impl->genParseObj();
}

const std::string& GenSchema::genSchemaName() const
{
    return m_impl->genSchemaName();
}

parse::ParseEndian GenSchema::genSchemaEndian() const
{
    return m_impl->genSchemaEndian();
}

unsigned GenSchema::genSchemaVersion() const 
{
    return m_impl->genSchemaVersion();
}

GenSchema::GenFieldsAccessList GenSchema::genGetAllMessageIdFields() const
{
    return m_impl->genGetAllMessageIdFields();
}

const GenField* GenSchema::genFindField(const std::string& externalRef) const
{
    auto* field = m_impl->genFindField(externalRef);
    assert(field->genIsPrepared());
    return field;
}

GenField* GenSchema::genFindField(const std::string& externalRef)
{
    auto* field = m_impl->genFindField(externalRef);
    do {
        if (field->genIsPrepared()) {
            break;
        }    

        if (field->genPrepare()) {
            break;
        }
         
        m_impl->genGenerator().genLogger().genWarning("Failed to prepare field: " + field->genParseObj().parseExternalRef());
        field = nullptr;
    } while (false);
    return field;
}

const GenMessage* GenSchema::genGindMessage(const std::string& externalRef) const
{
    return m_impl->genGindMessage(externalRef);
}

GenMessage* GenSchema::genGindMessage(const std::string& externalRef) 
{
    auto* msg = m_impl->genGindMessage(externalRef);
    do {
        if (msg->genIsPrepared()) {
            break;
        }

        if (msg->genPrepare()) {
            break;
        }

        m_impl->genGenerator().genLogger().genWarning("Failed to prepare message: " + msg->genParseObj().parseExternalRef());
        msg = nullptr;
    } while (false);
    return msg;
}

const GenFrame* GenSchema::genFindFrame(const std::string& externalRef) const
{
    return m_impl->genFindFrame(externalRef);
}

const GenInterface* GenSchema::genFindInterface(const std::string& externalRef) const
{
    return m_impl->genFindInterface(externalRef);
}

bool GenSchema::genAnyInterfaceHasVersion() const
{
    return m_impl->genAnyInterfaceHasVersion();
}

GenSchema::GenNamespacesAccessList GenSchema::genGetAllNamespaces() const
{
    GenNamespacesAccessList result;
    for (auto& n : m_impl->genNamespaces()) {
        auto subResult = n->genGetAllNamespaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
        result.push_back(n.get());
    }
    return result;
}

GenSchema::GenInterfacesAccessList GenSchema::genGetAllInterfaces() const
{
    GenInterfacesAccessList result;
    for (auto& n : m_impl->genNamespaces()) {
        auto subResult = n->genGetAllInterfaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

GenSchema::GenMessagesAccessList GenSchema::genGetAllMessages() const
{
    GenMessagesAccessList result;
    for (auto& n : m_impl->genNamespaces()) {
        auto subResult = n->genGetAllMessages();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

GenSchema::GenMessagesAccessList GenSchema::genGetAllMessagesIdSorted() const
{
    auto result = genGetAllMessages();
    GenGenerator::genSortMessages(result);
    return result;
}

GenSchema::GenFramesAccessList GenSchema::genGetAllFrames() const
{
    GenFramesAccessList result;
    for (auto& n : m_impl->genNamespaces()) {
        auto nList = n->genGetAllFrames();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;
}

GenSchema::GenFieldsAccessList GenSchema::genGetAllFields() const
{
    GenFieldsAccessList result;
    for (auto& n : m_impl->genNamespaces()) {
        auto nList = n->genGetAllFields();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;    
}

bool GenSchema::genCreateAll()
{
    return m_impl->genCreateAll();
}

bool GenSchema::genPrepare()
{
    // Make sure the logger is created

    if (!m_impl->genPrepare()) {
        return false;
    }

    return genPrepareImpl();
}

bool GenSchema::genWrite()
{
    if (!m_impl->genWrite()) {
        return false;
    }
    
    return genWriteImpl();
}

GenSchema::GenNamespacesList& GenSchema::genNamespaces()
{
    return m_impl->genNamespaces();
}

const GenSchema::GenNamespacesList& GenSchema::genNamespaces() const
{
    return m_impl->genNamespaces();
}

const GenSchema::GenPlatformNamesList& GenSchema::platformNames() const
{
    return m_impl->platformNames();
}

bool GenSchema::genVersionDependentCode() const
{
    return m_impl->genVersionDependentCode();
}

const std::string& GenSchema::genMainNamespace() const
{
    return m_impl->genMainNamespace();
}

const std::string& GenSchema::genOrigNamespace() const
{
    return m_impl->genOrigNamespace();
}

GenNamespace* GenSchema::genAddDefaultNamespace()
{
    auto& nsList = m_impl->genNamespaces();
    for (auto& nsPtr : nsList) {
        assert(nsPtr);
        if ((!nsPtr->genParseObj().parseValid()) || nsPtr->genParseObj().parseName().empty()) {
            return nsPtr.get();
        }
    }

    auto iter = nsList.insert(nsList.begin(), m_impl->genGenerator().genCreateNamespace(commsdsl::parse::ParseNamespace(nullptr), this));
    return iter->get();
}

void GenSchema::genForceSchemaVersion(unsigned value)
{
    m_impl->genForceSchemaVersion(value);
}

void GenSchema::genSetVersionIndependentCodeForced(bool value)
{
    m_impl->genSetVersionIndependentCodeForced(value);
}

void GenSchema::genSetMainNamespaceOverride(const std::string& value)
{
    m_impl->genSetMainNamespaceOverride(value);
}

void GenSchema::genSetMinRemoteVersion(unsigned value)
{
    m_impl->genSetMinRemoteVersion(value);
}

bool GenSchema::genDoesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->genDoesElementExist(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenSchema::genIsElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    return m_impl->genIsElementOptional(sinceVersion, deprecatedSince, deprecatedRemoved);
}

bool GenSchema::genIsElementDeprecated(unsigned deprecatedSince) const
{
    return deprecatedSince < genSchemaVersion();
}

void GenSchema::genSetAllInterfacesReferenced()
{
    m_impl->genSetAllInterfacesReferenced();
}

void GenSchema::genSetAllMessagesReferenced()
{
    m_impl->genSetAllMessagesReferenced();
}

bool GenSchema::genHasReferencedMessageIdField() const
{
    return m_impl->genHasReferencedMessageIdField();
}

bool GenSchema::genHasAnyReferencedMessage() const
{
    return m_impl->genHasAnyReferencedMessage();
}

bool GenSchema::genHasAnyReferencedComponent() const
{
    return m_impl->genHasAnyReferencedComponent();
}

GenElem::Type GenSchema::genElemTypeImpl() const
{
    return GenElem::Type_Schema;
}

bool GenSchema::genPrepareImpl()
{
    return true;
}

bool GenSchema::genWriteImpl()
{
    return true;
}


} // namespace gen

} // namespace commsdsl
