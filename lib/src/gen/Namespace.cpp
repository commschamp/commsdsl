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

#include "commsdsl/gen/Namespace.h"

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Interface.h"

#include <algorithm>
#include <cassert>

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
    

class NamespaceImpl
{
public:
    using NamespacesList = Namespace::NamespacesList;
    using FieldsList = Namespace::FieldsList;
    using InterfacesList = Namespace::InterfacesList;
    using MessagesList = Namespace::MessagesList;
    using FramesList = Namespace::FramesList;

    NamespaceImpl(Generator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool createAll()
    {
        if (!m_dslObj.valid()) {
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
        if (!m_dslObj.valid()) {
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

    commsdsl::parse::Namespace dslObj() const
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

    Generator& generator()
    {
        return m_generator;
    }

    const Generator& generator() const
    {
        return m_generator;
    }    

private:
    bool createNamespaces()
    {
        auto namespaces = m_dslObj.namespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.createNamespace(n, m_parent);
            assert(ptr);
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
        if (!m_dslObj.valid()) {
            return true;
        }

        auto fields = m_dslObj.fields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = Field::create(m_generator, dslObj, m_parent);
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
        auto interfaces = m_dslObj.interfaces();
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
        auto messages = m_dslObj.messages();
        m_messages.reserve(messages.size());
        for (auto& m : messages) {
            auto ptr = m_generator.createMessage(m, m_parent);
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
        auto frames = m_dslObj.frames();
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

    Generator& m_generator;
    commsdsl::parse::Namespace m_dslObj;
    Elem* m_parent = nullptr;
    NamespacesList m_namespaces;
    FieldsList m_fields;
    InterfacesList m_interfaces;
    MessagesList m_messages;
    FramesList m_frames;
}; 

Namespace::Namespace(Generator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<NamespaceImpl>(generator, dslObj, this))
{
}

Namespace::~Namespace() = default;

bool Namespace::createAll()
{
    return m_impl->createAll();
}

bool Namespace::prepare()
{
    return m_impl->prepare() && prepareImpl();
}

bool Namespace::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
}

commsdsl::parse::Namespace Namespace::dslObj() const
{
    return m_impl->dslObj();
}

const Namespace::NamespacesList& Namespace::namespaces() const
{
    return m_impl->namespaces();
}

const Namespace::FieldsList& Namespace::fields() const
{
    return m_impl->fields();
}

const Namespace::InterfacesList& Namespace::interfaces() const
{
    return m_impl->interfaces();
}

const Namespace::MessagesList& Namespace::messages() const
{
    return m_impl->messages();
}

const Namespace::FramesList& Namespace::frames() const
{
    return m_impl->frames();
}

const Field* Namespace::findMessageIdField() const
{
    for (auto& f : fields()) {
        if (f->dslObj().semanticType() != commsdsl::parse::Field::SemanticType::MessageId) {
            continue;
        }

        if ((f->dslObj().kind() != commsdsl::parse::Field::Kind::Enum) &&
            (f->dslObj().kind() != commsdsl::parse::Field::Kind::Int)) {
            static constexpr bool Unexpected_kind = false;
            static_cast<void>(Unexpected_kind);
            assert(Unexpected_kind);  
            return nullptr;
        }

        return f.get();
    }

    for (auto& n : namespaces()) {
        auto ptr = n->findMessageIdField();
        if (ptr != nullptr) {
            return ptr;
        }
    }

    return nullptr;
}

const Field* Namespace::findField(const std::string& externalRef) const
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

const Message* Namespace::findMessage(const std::string& externalRef) const
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

const Frame* Namespace::findFrame(const std::string& externalRef) const
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

const Interface* Namespace::findInterface(const std::string& externalRef) const
{
    auto pos = externalRef.find_first_of('.');
    std::string nsName;
    if (pos != std::string::npos) {
        nsName.assign(externalRef.begin(), externalRef.begin() + pos);
    }

    auto& ifList = interfaces();
    if (nsName.empty()) {
        auto ifIter =
            std::lower_bound(
                ifList.begin(), ifList.end(), externalRef,
                [](auto& f, auto& n)
                {
                    return f->name() < n;
                });

        if ((ifIter == ifList.end()) || ((*ifIter)->name() != externalRef)) {
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

Namespace::NamespacesAccessList Namespace::getAllNamespaces() const
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

Namespace::InterfacesAccessList Namespace::getAllInterfaces() const
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

Namespace::MessagesAccessList Namespace::getAllMessages() const
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

Namespace::FramesAccessList Namespace::getAllFrames() const
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

Generator& Namespace::generator()
{
    return m_impl->generator();
}

const Generator& Namespace::generator() const
{
    return m_impl->generator();
}

Interface* Namespace::addDefaultInterface()
{
    auto& intList = m_impl->interfaces();
    for (auto& intPtr : intList) {
        assert(intPtr);
        if ((!intPtr->dslObj().valid()) || intPtr->dslObj().name().empty()) {
            return intPtr.get();
        }
    }

    auto iter = intList.insert(intList.begin(), generator().createInterface(commsdsl::parse::Interface(nullptr), this));
    if (!(*iter)->prepare()) {
        intList.erase(iter);
        return nullptr;
    }
    
    return iter->get();    
}

Elem::Type Namespace::elemTypeImpl() const
{
    return Type_Namespace;
}

bool Namespace::prepareImpl()
{
    return true;
}

bool Namespace::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
