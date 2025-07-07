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

#include "commsdsl/gen/GenNamespace.h"

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace strings = commsdsl::gen::strings;

namespace commsdsl
{

namespace gen
{

namespace 
{

template <typename TList>
bool genWriteElements(TList& list)
{
    return std::all_of(
        list.begin(), list.end(),
        [](auto& elem)
        {
            return elem->genWrite();
        });    
}

} // namespace 
    

class GenNamespaceImpl
{
public:
    using ParseNamespace = GenNamespace::ParseNamespace;
    
    using GenNamespacesList = GenNamespace::GenNamespacesList;
    using GenFieldsList = GenNamespace::GenFieldsList;
    using GenInterfacesList = GenNamespace::GenInterfacesList;
    using GenMessagesList = GenNamespace::GenMessagesList;
    using GenFramesList = GenNamespace::GenFramesList;

    GenNamespaceImpl(GenGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genCreateAll()
    {
        if (!m_parseObj.parseValid()) {
            return true;
        }

        return
            genCreateNamespaces() &&
            genCreateFields() &&
            genCreateInterfaces() &&
            genCreateMessages() &&
            genCreateFrames();
    }    

    bool genPrepare()
    {
        if (!m_parseObj.parseValid()) {
            return true;
        }

        return
            genPrepareNamespaces() &&
            genPrepareFields() &&
            genPrepareInterfaces() &&
            genPrepareMessages() &&
            genPrepareFrames();
    }

    bool genWrite() const
    {
        return
            genWriteElements(m_namespaces) &&
            genWriteElements(m_fields) &&
            genWriteElements(m_interfaces) &&
            genWriteElements(m_messages) &&
            genWriteElements(m_frames);
    }

    ParseNamespace genParseObj() const
    {
        return m_parseObj;
    }

    const GenNamespacesList& genNamespaces() const
    {
        return m_namespaces;
    }

    const GenFieldsList& genFields() const
    {
        return m_fields;
    }

    const GenInterfacesList& genInterfaces() const
    {
        return m_interfaces;
    }

    GenInterfacesList& genInterfaces()
    {
        return m_interfaces;
    }    

    const GenMessagesList& genMessages() const
    {
        return m_messages;
    }

    const GenFramesList& genFrames() const
    {
        return m_frames;
    }

    bool genHasFramesRecursive() const
    {
        if (!m_frames.empty()) {
            return true;
        }

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->genHasFramesRecursive();
                });
    }

    bool genHasMessagesRecursive() const
    {
        if (!m_messages.empty()) {
            return true;
        }

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->genHasMessagesRecursive();
                });
    }    

    GenGenerator& genGenerator()
    {
        return m_generator;
    }

    const GenGenerator& genGenerator() const
    {
        return m_generator;
    }    

    void genSetAllInterfacesReferenced()
    {
        for (auto& iPtr : m_interfaces) {
            iPtr->genSetReferenced(true);
        }
    }

    void genSetAllMessagesReferenced()
    {
        for (auto& mPtr : m_messages) {
            mPtr->genSetReferenced(true);
        }
    }

    bool genHasReferencedMessageIdField() const
    {
        bool hasInFields = 
            std::any_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    return 
                        (f->genIsReferenced()) && 
                        (f->genParseObj().parseSemanticType() == commsdsl::parse::ParseField::ParseSemanticType::MessageId);
                });   

        if (hasInFields) {
            return true;
        }

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
        bool hasMessage = 
            std::any_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    return m->genIsReferenced();
                });   

        if (hasMessage) {
            return true;
        }

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
        if (!m_frames.empty()) {
            return true;
        }

        bool hasMessage = 
            std::any_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    return m->genIsReferenced();
                });   

        if (hasMessage) {
            return true;
        }

        bool hasInterface = 
            std::any_of(
                m_interfaces.begin(), m_interfaces.end(),
                [](auto& i)
                {
                    return i->genIsReferenced();
                });   

        if (hasInterface) {
            return true;
        }     

        bool hasField = 
            std::any_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    return f->genIsReferenced();
                });   

        if (hasField) {
            return true;
        }             

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->genHasAnyReferencedComponent();
                });        
    }    

private:
    bool genCreateNamespaces()
    {
        auto namespaces = m_parseObj.parseNamespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.genCreateNamespace(n, m_parent);
            assert(ptr);
            if (!ptr->genCreateAll()) {
                return false;
            }
            m_namespaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool genPrepareNamespaces()
    {
        return 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    assert(n);
                    return n->genPrepare();
                });
    }

    bool genCreateFields()
    {
        if (!m_parseObj.parseValid()) {
            return true;
        }

        auto fields = m_parseObj.parseFields();
        m_fields.reserve(fields.size());
        for (auto& parseObj : fields) {
            auto ptr = GenField::genCreate(m_generator, parseObj, m_parent);
            assert(ptr);
            m_fields.push_back(std::move(ptr));
        }

        return true;
    }    

    bool genPrepareFields()
    {
        return 
            std::all_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    assert(f);
                    return f->genPrepare();
                });

    }

    bool genCreateInterfaces()
    {
        auto interfaces = m_parseObj.parseInterfaces();
        m_interfaces.reserve(interfaces.size());
        for (auto& i : interfaces) {
            auto ptr = m_generator.genCreateInterface(i, m_parent);
            assert(ptr);
            if (!ptr->genCreateAll()) {
                return false;
            }
            
            m_interfaces.push_back(std::move(ptr));
        }

        return true;
    }    

    bool genPrepareInterfaces()
    {
        return 
            std::all_of(
                m_interfaces.begin(), m_interfaces.end(),
                [](auto& i)
                {
                    assert(i);
                    return i->genPrepare();
                });
    }

    bool genCreateMessages()
    {
        auto messages = m_parseObj.parseMessages();
        m_messages.reserve(messages.size());
        for (auto& m : messages) {
            auto ptr = m_generator.genCreateMessage(m, m_parent);
            if (!ptr->genCreateAll()) {
                return false;
            }            
            assert(ptr);
            m_messages.push_back(std::move(ptr));
        }

        return true;
    }    

    bool genPrepareMessages()
    {
        return 
            std::all_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    assert(m);
                    return m->genPrepare();
                });
    }

    bool genCreateFrames()
    {
        auto frames = m_parseObj.parseFrames();
        m_frames.reserve(frames.size());
        for (auto& f : frames) {
            auto ptr = m_generator.genCreateFrame(f, m_parent);
            assert(ptr);
            m_frames.push_back(std::move(ptr));
        }

        return true;
    }    

    bool genPrepareFrames()
    {
        return 
            std::all_of(
                m_frames.begin(), m_frames.end(),
                [](auto& f)
                {
                    assert(f);
                    return f->genPrepare();
                });
    }

    GenGenerator& m_generator;
    ParseNamespace m_parseObj;
    GenElem* m_parent = nullptr;
    GenNamespacesList m_namespaces;
    GenFieldsList m_fields;
    GenInterfacesList m_interfaces;
    GenMessagesList m_messages;
    GenFramesList m_frames;
}; 

GenNamespace::GenNamespace(GenGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenNamespaceImpl>(generator, parseObj, this))
{
}

GenNamespace::~GenNamespace() = default;

bool GenNamespace::genCreateAll()
{
    return m_impl->genCreateAll();
}

bool GenNamespace::genPrepare()
{
    return m_impl->genPrepare() && genPrepareImpl();
}

bool GenNamespace::genWrite() const
{
    if (!m_impl->genWrite()) {
        return false;
    }

    return genWriteImpl();
}

GenNamespace::ParseNamespace GenNamespace::genParseObj() const
{
    return m_impl->genParseObj();
}

std::string GenNamespace::genAdjustedExternalRef() const
{
    auto obj = genParseObj();
    if (obj.parseValid()) {
        return obj.parseExternalRef();
    }

    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == GenElem::Type_Schema);
    auto* schema = static_cast<const GenSchema*>(parent);
    assert(schema->genParseObj().parseValid());
    return schema->genParseObj().parseExternalRef();
}

const GenNamespace::GenNamespacesList& GenNamespace::genNamespaces() const
{
    return m_impl->genNamespaces();
}

const GenNamespace::GenFieldsList& GenNamespace::genFields() const
{
    return m_impl->genFields();
}

const GenNamespace::GenInterfacesList& GenNamespace::genInterfaces() const
{
    return m_impl->genInterfaces();
}

const GenNamespace::GenMessagesList& GenNamespace::genMessages() const
{
    return m_impl->genMessages();
}

const GenNamespace::GenFramesList& GenNamespace::genFrames() const
{
    return m_impl->genFrames();
}

bool GenNamespace::genHasFramesRecursive() const
{
    return m_impl->genHasFramesRecursive();
}

bool GenNamespace::genHasMessagesRecursive() const
{
    return m_impl->genHasMessagesRecursive();
}

GenNamespace::GenFieldsAccessList GenNamespace::genFindMessageIdFields() const
{
    GenFieldsAccessList result;
    for (auto& f : genFields()) {
        if (f->genParseObj().parseSemanticType() != commsdsl::parse::ParseField::ParseSemanticType::MessageId) {
            continue;
        }

        if ((f->genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Enum) &&
            (f->genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Int)) {
            [[maybe_unused]] static constexpr bool Unexpected_kind = false;
            assert(Unexpected_kind);  
            continue;
        }

        result.push_back(f.get());
    }

    for (auto& n : genNamespaces()) {
        auto nsResult = n->genFindMessageIdFields();
        std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
    }

    return result;
}

const GenField* GenNamespace::genFindField(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& fList = genFields();
    if (nsName.empty()) {
        auto fieldIter =
            std::lower_bound(
                fList.begin(), fList.end(), externalRef,
                [](auto& f, auto& n)
                {
                    return f->genName() < n;
                });

        if ((fieldIter == fList.end()) || ((*fieldIter)->genName() != externalRef)) {
            return nullptr;
        }

        return fieldIter->get();
    }

    auto& nsList = genNamespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->genName() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->genName() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->genFindField(remStr);
}

const GenMessage* GenNamespace::genGindMessage(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& mList = genMessages();
    if (nsName.empty()) {
        auto messageIter =
            std::lower_bound(
                mList.begin(), mList.end(), externalRef,
                [](auto& m, auto& n)
                {
                    return m->genName() < n;
                });

        if ((messageIter == mList.end()) || ((*messageIter)->genName() != externalRef)) {
            return nullptr;
        }

        return messageIter->get();
    }

    auto& nsList = genNamespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->genName() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->genName() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->genGindMessage(remStr);
}

const GenFrame* GenNamespace::genFindFrame(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& framesList = genFrames();
    if (nsName.empty()) {
        auto frameIter =
            std::lower_bound(
                framesList.begin(), framesList.end(), externalRef,
                [](auto& f, auto& n)
                {
                    return f->genName() < n;
                });

        if ((frameIter == framesList.end()) || ((*frameIter)->genName() != externalRef)) {
            return nullptr;
        }

        return frameIter->get();
    }

    auto& nsList = genNamespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->genName() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->genName() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->genFindFrame(remStr);
}

const GenInterface* GenNamespace::genFindInterface(const std::string& externalRef) const
{
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& ifList = genInterfaces();
    if (nsName.empty()) {
        auto& adjustedExternalRef = externalRef.empty() ? strings::genMessageClassStr() : externalRef;
        auto ifIter =
            std::lower_bound(
                ifList.begin(), ifList.end(), adjustedExternalRef,
                [](auto& f, auto& n)
                {
                    return f->genAdjustedName() < n;
                });

        if ((ifIter == ifList.end()) || ((*ifIter)->genAdjustedName() != adjustedExternalRef)) {
            return nullptr;
        }

        return ifIter->get();
    }

    auto& nsList = genNamespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->genName() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->genName() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->genFindInterface(remStr);
}

GenNamespace::GenNamespacesAccessList GenNamespace::genGetAllNamespaces() const
{
    GenNamespacesAccessList result;
    auto& subNs = m_impl->genNamespaces();
    for (auto& n : subNs) {
        auto list = n->genGetAllNamespaces();
        result.insert(result.end(), list.begin(), list.end());
        result.emplace_back(n.get());
    }

    return result;
}

GenNamespace::GenInterfacesAccessList GenNamespace::genGetAllInterfaces() const
{
    GenInterfacesAccessList result;
    auto& subNs = m_impl->genNamespaces();
    for (auto& n : subNs) {
        auto list = n->genGetAllInterfaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->genInterfaces().size());
    for (auto& i : m_impl->genInterfaces()) {
        result.emplace_back(i.get());
    }

    return result;
}

GenNamespace::GenMessagesAccessList GenNamespace::genGetAllMessages() const
{
    GenMessagesAccessList result;
    auto& subNs = m_impl->genNamespaces();
    for (auto& n : subNs) {
        auto list = n->genGetAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->genMessages().size());
    for (auto& i : m_impl->genMessages()) {
        result.emplace_back(i.get());
    }

    return result;
}

GenNamespace::GenMessagesAccessList GenNamespace::genGetAllMessagesIdSorted() const
{
    auto result = genGetAllMessages();
    GenGenerator::genSortMessages(result);
    return result;
}

GenNamespace::GenFramesAccessList GenNamespace::genGetAllFrames() const
{
    GenFramesAccessList result;
    auto& subNs = m_impl->genNamespaces();
    for (auto& n : subNs) {
        auto list = n->genGetAllFrames();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->genFrames().size());
    for (auto& f : m_impl->genFrames()) {
        result.emplace_back(f.get());
    }

    return result;
}

GenNamespace::GenFieldsAccessList GenNamespace::genGetAllFields() const
{
    GenFieldsAccessList result;
    auto& subNs = m_impl->genNamespaces();
    for (auto& n : subNs) {
        auto list = n->genGetAllFields();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->genFields().size());
    for (auto& f : m_impl->genFields()) {
        result.emplace_back(f.get());
    }

    return result;
}

GenGenerator& GenNamespace::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenNamespace::genGenerator() const
{
    return m_impl->genGenerator();
}

GenInterface* GenNamespace::genAddDefaultInterface()
{
    auto& intList = m_impl->genInterfaces();
    for (auto& intPtr : intList) {
        assert(intPtr);
        if ((!intPtr->genParseObj().parseValid()) || intPtr->genParseObj().parseName().empty()) {
            return intPtr.get();
        }
    }

    auto iter = intList.insert(intList.begin(), genGenerator().genCreateInterface(commsdsl::parse::ParseInterface(nullptr), this));
    (*iter)->genSetReferenced(true);
    if (!(*iter)->genPrepare()) {
        intList.erase(iter);
        return nullptr;
    }
    
    return iter->get();    
}

void GenNamespace::genSetAllInterfacesReferenced()
{
    m_impl->genSetAllInterfacesReferenced();
}

void GenNamespace::genSetAllMessagesReferenced()
{
    m_impl->genSetAllMessagesReferenced();
}

bool GenNamespace::genHasReferencedMessageIdField() const
{
    return m_impl->genHasReferencedMessageIdField();
}

bool GenNamespace::genHasAnyReferencedMessage() const
{
    return m_impl->genHasAnyReferencedMessage();
}

bool GenNamespace::genHasAnyReferencedComponent() const
{
    return m_impl->genHasAnyReferencedComponent();
}

GenElem::Type GenNamespace::genElemTypeImpl() const
{
    return Type_Namespace;
}

bool GenNamespace::genPrepareImpl()
{
    return true;
}

bool GenNamespace::genWriteImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
