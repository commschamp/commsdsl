//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsNamespace.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{

namespace 
{

const std::string& optsTemplInternal(bool defaultNs)
{
    if (defaultNs) {
        static const std::string Templ = 
            "#^#BODY#$#\n";
        return Templ;
    }

    static const std::string Templ = 
        "struct #^#NAME#$##^#EXT\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n";  
    return Templ;
}

} // namespace 
    

CommsNamespace::CommsNamespace(CommsGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

CommsNamespace::~CommsNamespace() = default;

std::string CommsNamespace::commsDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsDefaultOptions,
            &CommsField::commsDefaultOptions,
            &CommsMessage::commsDefaultOptions,
            &CommsFrame::commsDefaultOptions
        );

    auto name = comms::namespaceName(dslObj().name());
    util::ReplacementMap repl = {
        {"NAME", name},
        {"BODY", std::move(body)},
    };
    auto result = util::processTemplate(optsTemplInternal(name.empty()), repl);
    return result;
}

std::string CommsNamespace::commsClientDefaultOptions() const
{
    // TODO
    return std::string();
}

std::string CommsNamespace::commsServerDefaultOptions() const
{
    // TODO
    return std::string();
}

std::string CommsNamespace::commsDataViewDefaultOptions() const
{
    // TODO
    return std::string();
}

std::string CommsNamespace::commsBareMetalDefaultOptions() const
{
    // TODO
    return std::string();
}

bool CommsNamespace::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_commsFields = CommsField::commsTransformFieldsList(fields());
    return true;
}

std::string CommsNamespace::commsOptionsInternal(
    NamespaceOptsFunc nsOptsFunc,
    FieldOptsFunc fieldOptsFunc,
    MessageOptsFunc messageOptsFunc,
    FrameOptsFunc frameOptsFunc) const
{
    util::StringsList elems;
    auto addStrFunc = 
        [&elems](std::string&& str)
        {
            if (!str.empty()) {
                elems.push_back(std::move(str));
            }
        };

    auto& subNsList = namespaces();
    for (auto& nsPtr : subNsList) {
        addStrFunc((static_cast<const CommsNamespace*>(nsPtr.get())->*nsOptsFunc)());
    }

    if (fieldOptsFunc != nullptr) {
        for (auto* commsField : m_commsFields) {
            addStrFunc((commsField->*fieldOptsFunc)());
        }
    }

    for (auto& msgPtr : messages()) {
        assert(msgPtr);
        addStrFunc((static_cast<const CommsMessage*>(msgPtr.get())->*messageOptsFunc)());
    }

    for (auto& framePtr : frames()) {
        assert(framePtr);
        addStrFunc((static_cast<const CommsFrame*>(framePtr.get())->*frameOptsFunc)());
    }    

    return util::strListToString(elems, "\n", "");
}


} // namespace commsdsl2new
