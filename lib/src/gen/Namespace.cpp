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

#include <cassert>
#include <algorithm>

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

    bool write()
    {
        return
            writeElements(m_namespaces) &&
            writeElements(m_fields) &&
            writeElements(m_interfaces) &&
            writeElements(m_messages) &&
            writeElements(m_frames);
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

    const MessagesList& messages() const
    {
        return m_messages;
    }

private:
    bool prepareNamespaces()
    {
        auto namespaces = m_dslObj.namespaces();
        m_namespaces.reserve(namespaces.size());
        for (auto& n : namespaces) {
            auto ptr = m_generator.createNamespace(n, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_namespaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepareFields()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

        auto fields = m_dslObj.fields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = Field::create(m_generator, dslObj, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_fields.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepareInterfaces()
    {
        auto interfaces = m_dslObj.interfaces();
        m_interfaces.reserve(interfaces.size());
        for (auto& i : interfaces) {
            auto ptr = m_generator.createInterface(i, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_interfaces.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepareMessages()
    {
        auto messages = m_dslObj.messages();
        m_messages.reserve(messages.size());
        for (auto& m : messages) {
            auto ptr = m_generator.createMessage(m, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_messages.push_back(std::move(ptr));
        }

        return true;
    }

    bool prepareFrames()
    {
        auto frames = m_dslObj.frames();
        m_frames.reserve(frames.size());
        for (auto& f : frames) {
            auto ptr = m_generator.createFrame(f, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_frames.push_back(std::move(ptr));
        }

        return true;
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

bool Namespace::prepare()
{
    return m_impl->prepare();
}

bool Namespace::write()
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
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

const Field* Namespace::findMessageIdField() const
{
    for (auto& f : fields()) {
        if (f->dslObj().semanticType() != commsdsl::parse::Field::SemanticType::MessageId) {
            continue;
        }

        if ((f->dslObj().kind() != commsdsl::parse::Field::Kind::Enum) ||
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

Elem::Type Namespace::elemTypeImpl() const
{
    return Type_Namespace;
}

bool Namespace::writeImpl()
{
    return true;
}

} // namespace gen

} // namespace commsdsl
