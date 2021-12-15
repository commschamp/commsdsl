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

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class NamespaceImpl
{
public:
    using NamespacesList = Namespace::NamespacesList;
    using FieldsList = Field::FieldsList;

    NamespaceImpl(Generator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
        static_cast<void>(m_parent);
    }

    bool prepare()
    {
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
            writeNamespaces() &&
            writeFields() &&
            writeInterfaces() &&
            writeMessages() &&
            writeFrames();
    }

private:
    bool prepareNamespaces()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

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
        // TODO:
        return true;
    }

    bool prepareMessages()
    {
        // TODO
        return true;
    }

    bool prepareFrames()
    {
        // TODO
        return true;
    }

    bool writeNamespaces()
    {
        if (m_namespaces.empty()) {
            return true;
        }

        return std::all_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& ns)
            {
                return ns->write();
            }
        );
    }

    bool writeFields()
    {
        // TODO:
        return true;
    }

    bool writeInterfaces()
    {
        // TODO:
        return true;
    }

    bool writeMessages()
    {
        // TODO
        return true;
    }

    bool writeFrames()
    {
        // TODO
        return true;
    }


    Generator& m_generator;
    commsdsl::parse::Namespace m_dslObj;
    Elem* m_parent = nullptr;
    NamespacesList m_namespaces;
    FieldsList m_fields;
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
    if (m_impl->write()) {
        return false;
    }

    return writeImpl();
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
