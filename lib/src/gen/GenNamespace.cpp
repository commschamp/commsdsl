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
bool writeElements(TList& list)
{
    return std::all_of(
        list.begin(), list.end(),
        [](auto& elem)
        {
            return elem->write();
        });    
}

} // namespace 
    

class GenNamespaceImpl
{
public:
    using NamespacesList = GenNamespace::NamespacesList;
    using FieldsList = GenNamespace::FieldsList;
    using InterfacesList = GenNamespace::InterfacesList;
    using MessagesList = GenNamespace::MessagesList;
    using FramesList = GenNamespace::FramesList;

    GenNamespaceImpl(GenGenerator& generator, commsdsl::parse::ParseNamespace dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool createAll()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        return
            createNamespaces() &&
            createFields() &&
            createInterfaces() &&
            createMessages() &&
            createFrames();
    }    

    bool prepare()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        return
            prepareNamespaces() &&
            prepareFields() &&
            prepareInterfaces() &&
            prepareMessages() &&
            prepareFrames();
    }

    bool write() const
    {
        return
            writeElements(m_namespaces) &&
            writeElements(m_fields) &&
            writeElements(m_interfaces) &&
            writeElements(m_messages) &&
            writeElements(m_frames);
    }

    commsdsl::parse::ParseNamespace dslObj() const
    {
        return m_dslObj;
    }

    const NamespacesList& namespaces() const
    {
        return m_namespaces;
    }

    const FieldsList& fields() const
    {
        return m_fields;
    }

    const InterfacesList& interfaces() const
    {
        return m_interfaces;
    }

    InterfacesList& interfaces()
    {
        return m_interfaces;
    }    

    const MessagesList& messages() const
    {
        return m_messages;
    }

    const FramesList& frames() const
    {
        return m_frames;
    }

    bool hasFramesRecursive() const
    {
        if (!m_frames.empty()) {
            return true;
        }

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->hasFramesRecursive();
                });
    }

    bool hasMessagesRecursive() const
    {
        if (!m_messages.empty()) {
            return true;
        }

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& ns)
                {
                    return ns->hasMessagesRecursive();
                });
    }    

    GenGenerator& generator()
    {
        return m_generator;
    }

    const GenGenerator& generator() const
    {
        return m_generator;
    }    

    void setAllInterfacesReferenced()
    {
        for (auto& iPtr : m_interfaces) {
            iPtr->setReferenced(true);
        }
    }

    void setAllMessagesReferenced()
    {
        for (auto& mPtr : m_messages) {
            mPtr->setReferenced(true);
        }
    }

    bool hasReferencedMessageIdField() const
    {
        bool hasInFields = 
            std::any_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    return 
                        (f->isReferenced()) && 
                        (f->dslObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::MessageId);
                });   

        if (hasInFields) {
            return true;
        }

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
        bool hasMessage = 
            std::any_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    return m->isReferenced();
                });   

        if (hasMessage) {
            return true;
        }

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
        if (!m_frames.empty()) {
            return true;
        }

        bool hasMessage = 
            std::any_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    return m->isReferenced();
                });   

        if (hasMessage) {
            return true;
        }

        bool hasInterface = 
            std::any_of(
                m_interfaces.begin(), m_interfaces.end(),
                [](auto& i)
                {
                    return i->isReferenced();
                });   

        if (hasInterface) {
            return true;
        }     

        bool hasField = 
            std::any_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    return f->isReferenced();
                });   

        if (hasField) {
            return true;
        }             

        return 
            std::any_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    return n->hasAnyReferencedComponent();
                });        
    }    

private:
    bool createNamespaces()
    {
        auto namespaces = m_dslObj.parseNamespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.createNamespace(n, m_parent);
            assert(ptr);
            if (!ptr->createAll()) {
                return false;
            }
            m_namespaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepareNamespaces()
    {
        return 
            std::all_of(
                m_namespaces.begin(), m_namespaces.end(),
                [](auto& n)
                {
                    assert(n);
                    return n->prepare();
                });
    }

    bool createFields()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto fields = m_dslObj.parseFields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = GenField::create(m_generator, dslObj, m_parent);
            assert(ptr);
            m_fields.push_back(std::move(ptr));
        }

        return true;
    }    

    bool prepareFields()
    {
        return 
            std::all_of(
                m_fields.begin(), m_fields.end(),
                [](auto& f)
                {
                    assert(f);
                    return f->prepare();
                });

    }

    bool createInterfaces()
    {
        auto interfaces = m_dslObj.parseInterfaces();
        m_interfaces.reserve(interfaces.size());
        for (auto& i : interfaces) {
            auto ptr = m_generator.createInterface(i, m_parent);
            assert(ptr);
            if (!ptr->createAll()) {
                return false;
            }
            
            m_interfaces.push_back(std::move(ptr));
        }

        return true;
    }    

    bool prepareInterfaces()
    {
        return 
            std::all_of(
                m_interfaces.begin(), m_interfaces.end(),
                [](auto& i)
                {
                    assert(i);
                    return i->prepare();
                });
    }

    bool createMessages()
    {
        auto messages = m_dslObj.parseMessages();
        m_messages.reserve(messages.size());
        for (auto& m : messages) {
            auto ptr = m_generator.createMessage(m, m_parent);
            if (!ptr->createAll()) {
                return false;
            }            
            assert(ptr);
            m_messages.push_back(std::move(ptr));
        }

        return true;
    }    

    bool prepareMessages()
    {
        return 
            std::all_of(
                m_messages.begin(), m_messages.end(),
                [](auto& m)
                {
                    assert(m);
                    return m->prepare();
                });
    }

    bool createFrames()
    {
        auto frames = m_dslObj.parseFrames();
        m_frames.reserve(frames.size());
        for (auto& f : frames) {
            auto ptr = m_generator.createFrame(f, m_parent);
            assert(ptr);
            m_frames.push_back(std::move(ptr));
        }

        return true;
    }    

    bool prepareFrames()
    {
        return 
            std::all_of(
                m_frames.begin(), m_frames.end(),
                [](auto& f)
                {
                    assert(f);
                    return f->prepare();
                });
    }

    GenGenerator& m_generator;
    commsdsl::parse::ParseNamespace m_dslObj;
    GenElem* m_parent = nullptr;
    NamespacesList m_namespaces;
    FieldsList m_fields;
    InterfacesList m_interfaces;
    MessagesList m_messages;
    FramesList m_frames;
}; 

GenNamespace::GenNamespace(GenGenerator& generator, commsdsl::parse::ParseNamespace dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenNamespaceImpl>(generator, dslObj, this))
{
}

GenNamespace::~GenNamespace() = default;

bool GenNamespace::createAll()
{
    return m_impl->createAll();
}

bool GenNamespace::prepare()
{
    return m_impl->prepare() && prepareImpl();
}

bool GenNamespace::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
}

commsdsl::parse::ParseNamespace GenNamespace::dslObj() const
{
    return m_impl->dslObj();
}

std::string GenNamespace::adjustedExternalRef() const
{
    auto obj = dslObj();
    if (obj.parseValid()) {
        return obj.parseExternalRef();
    }

    auto* parent = getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == GenElem::Type_Schema);
    auto* schema = static_cast<const GenSchema*>(parent);
    assert(schema->dslObj().parseValid());
    return schema->dslObj().parseExternalRef();
}

const GenNamespace::NamespacesList& GenNamespace::namespaces() const
{
    return m_impl->namespaces();
}

const GenNamespace::FieldsList& GenNamespace::fields() const
{
    return m_impl->fields();
}

const GenNamespace::InterfacesList& GenNamespace::interfaces() const
{
    return m_impl->interfaces();
}

const GenNamespace::MessagesList& GenNamespace::messages() const
{
    return m_impl->messages();
}

const GenNamespace::FramesList& GenNamespace::frames() const
{
    return m_impl->frames();
}

bool GenNamespace::hasFramesRecursive() const
{
    return m_impl->hasFramesRecursive();
}

bool GenNamespace::hasMessagesRecursive() const
{
    return m_impl->hasMessagesRecursive();
}

GenNamespace::FieldsAccessList GenNamespace::findMessageIdFields() const
{
    FieldsAccessList result;
    for (auto& f : fields()) {
        if (f->dslObj().parseSemanticType() != commsdsl::parse::ParseField::SemanticType::MessageId) {
            continue;
        }

        if ((f->dslObj().parseKind() != commsdsl::parse::ParseField::Kind::Enum) &&
            (f->dslObj().parseKind() != commsdsl::parse::ParseField::Kind::Int)) {
            [[maybe_unused]] static constexpr bool Unexpected_kind = false;
            assert(Unexpected_kind);  
            continue;
        }

        result.push_back(f.get());
    }

    for (auto& n : namespaces()) {
        auto nsResult = n->findMessageIdFields();
        std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
    }

    return result;
}

const GenField* GenNamespace::findField(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& fList = fields();
    if (nsName.empty()) {
        auto fieldIter =
            std::lower_bound(
                fList.begin(), fList.end(), externalRef,
                [](auto& f, auto& n)
                {
                    return f->name() < n;
                });

        if ((fieldIter == fList.end()) || ((*fieldIter)->name() != externalRef)) {
            return nullptr;
        }

        return fieldIter->get();
    }

    auto& nsList = namespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->name() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->name() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->findField(remStr);
}

const GenMessage* GenNamespace::findMessage(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& mList = messages();
    if (nsName.empty()) {
        auto messageIter =
            std::lower_bound(
                mList.begin(), mList.end(), externalRef,
                [](auto& m, auto& n)
                {
                    return m->name() < n;
                });

        if ((messageIter == mList.end()) || ((*messageIter)->name() != externalRef)) {
            return nullptr;
        }

        return messageIter->get();
    }

    auto& nsList = namespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->name() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->name() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->findMessage(remStr);
}

const GenFrame* GenNamespace::findFrame(const std::string& externalRef) const
{
    assert(!externalRef.empty());
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& framesList = frames();
    if (nsName.empty()) {
        auto frameIter =
            std::lower_bound(
                framesList.begin(), framesList.end(), externalRef,
                [](auto& f, auto& n)
                {
                    return f->name() < n;
                });

        if ((frameIter == framesList.end()) || ((*frameIter)->name() != externalRef)) {
            return nullptr;
        }

        return frameIter->get();
    }

    auto& nsList = namespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->name() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->name() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->findFrame(remStr);
}

const GenInterface* GenNamespace::findInterface(const std::string& externalRef) const
{
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& ifList = interfaces();
    if (nsName.empty()) {
        auto& adjustedExternalRef = externalRef.empty() ? strings::messageClassStr() : externalRef;
        auto ifIter =
            std::lower_bound(
                ifList.begin(), ifList.end(), adjustedExternalRef,
                [](auto& f, auto& n)
                {
                    return f->adjustedName() < n;
                });

        if ((ifIter == ifList.end()) || ((*ifIter)->adjustedName() != adjustedExternalRef)) {
            return nullptr;
        }

        return ifIter->get();
    }

    auto& nsList = namespaces();
    auto nsIter =
        std::lower_bound(
            nsList.begin(), nsList.end(), nsName,
            [](auto& ns, const std::string& n)
            {
                return ns->name() < n;
            });

    if ((nsIter == nsList.end()) || ((*nsIter)->name() != nsName)) {
        return nullptr;
    }

    std::size_t fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    return (*nsIter)->findInterface(remStr);
}

GenNamespace::NamespacesAccessList GenNamespace::getAllNamespaces() const
{
    NamespacesAccessList result;
    auto& subNs = m_impl->namespaces();
    for (auto& n : subNs) {
        auto list = n->getAllNamespaces();
        result.insert(result.end(), list.begin(), list.end());
        result.emplace_back(n.get());
    }

    return result;
}

GenNamespace::InterfacesAccessList GenNamespace::getAllInterfaces() const
{
    InterfacesAccessList result;
    auto& subNs = m_impl->namespaces();
    for (auto& n : subNs) {
        auto list = n->getAllInterfaces();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->interfaces().size());
    for (auto& i : m_impl->interfaces()) {
        result.emplace_back(i.get());
    }

    return result;
}

GenNamespace::MessagesAccessList GenNamespace::getAllMessages() const
{
    MessagesAccessList result;
    auto& subNs = m_impl->namespaces();
    for (auto& n : subNs) {
        auto list = n->getAllMessages();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->messages().size());
    for (auto& i : m_impl->messages()) {
        result.emplace_back(i.get());
    }

    return result;
}

GenNamespace::MessagesAccessList GenNamespace::getAllMessagesIdSorted() const
{
    auto result = getAllMessages();
    GenGenerator::sortMessages(result);
    return result;
}

GenNamespace::FramesAccessList GenNamespace::getAllFrames() const
{
    FramesAccessList result;
    auto& subNs = m_impl->namespaces();
    for (auto& n : subNs) {
        auto list = n->getAllFrames();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->frames().size());
    for (auto& f : m_impl->frames()) {
        result.emplace_back(f.get());
    }

    return result;
}

GenNamespace::FieldsAccessList GenNamespace::getAllFields() const
{
    FieldsAccessList result;
    auto& subNs = m_impl->namespaces();
    for (auto& n : subNs) {
        auto list = n->getAllFields();
        result.insert(result.end(), list.begin(), list.end());
    }

    result.reserve(result.size() + m_impl->fields().size());
    for (auto& f : m_impl->fields()) {
        result.emplace_back(f.get());
    }

    return result;
}

GenGenerator& GenNamespace::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenNamespace::generator() const
{
    return m_impl->generator();
}

GenInterface* GenNamespace::addDefaultInterface()
{
    auto& intList = m_impl->interfaces();
    for (auto& intPtr : intList) {
        assert(intPtr);
        if ((!intPtr->dslObj().parseValid()) || intPtr->dslObj().parseName().empty()) {
            return intPtr.get();
        }
    }

    auto iter = intList.insert(intList.begin(), generator().createInterface(commsdsl::parse::ParseInterface(nullptr), this));
    (*iter)->setReferenced(true);
    if (!(*iter)->prepare()) {
        intList.erase(iter);
        return nullptr;
    }
    
    return iter->get();    
}

void GenNamespace::setAllInterfacesReferenced()
{
    m_impl->setAllInterfacesReferenced();
}

void GenNamespace::setAllMessagesReferenced()
{
    m_impl->setAllMessagesReferenced();
}

bool GenNamespace::hasReferencedMessageIdField() const
{
    return m_impl->hasReferencedMessageIdField();
}

bool GenNamespace::hasAnyReferencedMessage() const
{
    return m_impl->hasAnyReferencedMessage();
}

bool GenNamespace::hasAnyReferencedComponent() const
{
    return m_impl->hasAnyReferencedComponent();
}

GenElem::Type GenNamespace::elemTypeImpl() const
{
    return Type_Namespace;
}

bool GenNamespace::prepareImpl()
{
    return true;
}

bool GenNamespace::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
