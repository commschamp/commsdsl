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
#include "CommsInterface.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
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
        "/// @brief Extra options for namespace.\n"
        "struct #^#NAME#$##^#EXT#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}; // struct #^#NAME#$#\n";  
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
            &CommsFrame::commsDefaultOptions,
            false
        );

    auto nsName = comms::namespaceName(name());
    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };
    return util::processTemplate(optsTemplInternal(nsName.empty()), repl);
}

std::string CommsNamespace::commsClientDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsClientDefaultOptions,
            nullptr,
            &CommsMessage::commsClientDefaultOptions,
            nullptr,
            true
        );

    if (body.empty()) {
        return strings::emptyString();
    }

    auto nsName = comms::namespaceName(name());
    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(generator());
    bool hasMainNs = commsGen.hasMainNamespaceInOptions(); 
    auto thisNsScope = comms::scopeFor(*this, generator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }
        
    return util::processTemplate(optsTemplInternal(nsName.empty()), repl);
}

std::string CommsNamespace::commsServerDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsServerDefaultOptions,
            nullptr,
            &CommsMessage::commsServerDefaultOptions,
            nullptr,
            true
        );

    if (body.empty()) {
        return strings::emptyString();
    }

    auto nsName = comms::namespaceName(name());
    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)}
    };

    auto& commsGen = static_cast<const CommsGenerator&>(generator());
    bool hasMainNs = commsGen.hasMainNamespaceInOptions(); 
    auto thisNsScope = comms::scopeFor(*this, generator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::processTemplate(optsTemplInternal(nsName.empty()), repl);
}

std::string CommsNamespace::commsDataViewDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsDataViewDefaultOptions,
            &CommsField::commsDataViewDefaultOptions,
            &CommsMessage::commsDataViewDefaultOptions,
            &CommsFrame::commsDataViewDefaultOptions,
            true
        );

    if (body.empty()) {
        return strings::emptyString();
    }

    auto nsName = comms::namespaceName(name());
    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(generator());
    bool hasMainNs = commsGen.hasMainNamespaceInOptions(); 
    auto thisNsScope = comms::scopeFor(*this, generator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::processTemplate(optsTemplInternal(nsName.empty()), repl);
}

std::string CommsNamespace::commsBareMetalDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsBareMetalDefaultOptions,
            &CommsField::commsBareMetalDefaultOptions,
            &CommsMessage::commsBareMetalDefaultOptions,
            &CommsFrame::commsBareMetalDefaultOptions,
            true
        );

    if (body.empty()) {
        return strings::emptyString();
    }

    auto nsName = comms::namespaceName(name());
    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(generator());
    bool hasMainNs = commsGen.hasMainNamespaceInOptions(); 
    auto thisNsScope = comms::scopeFor(*this, generator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::processTemplate(optsTemplInternal(nsName.empty()), repl);
}

bool CommsNamespace::commsHasReferencedMsgId() const
{
    return 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return 
                    (f->commsIsReferenced()) && 
                    (f->field().dslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId);
            });
}

bool CommsNamespace::commsHasAnyGeneratedCode() const
{
    if ((!frames().empty()) ||
        (!messages().empty()) || 
        (!interfaces().empty())) {
        return true;
    }

    bool hasReferencedFields = 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return f->commsIsReferenced();
            });


    if (hasReferencedFields) {
        return true;
    }

    auto& allNs = namespaces();
    return 
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto& nsPtr)
            {
                return static_cast<const CommsNamespace*>(nsPtr.get())->commsHasAnyGeneratedCode();
            });

}

bool CommsNamespace::commsHasAnyField() const
{
    if (!frames().empty()) {
        return true;
    }

    auto& allMsgs = messages();
    bool hasFieldInMessage = 
        std::any_of(
            allMsgs.begin(), allMsgs.end(),
            [](auto& msgPtr)
            {
                auto& msgFields = static_cast<const CommsMessage*>(msgPtr.get())->commsFields();
                return !msgFields.empty();
            });

    if (hasFieldInMessage) {
        return true;
    }

    auto& allInterfaces = interfaces();
    bool hasFieldInInterface = 
        std::any_of(
            allInterfaces.begin(), allInterfaces.end(),
            [](auto& interfacePtr)
            {
                auto& interfaceFields = static_cast<const CommsInterface*>(interfacePtr.get())->commsFields();
                return !interfaceFields.empty();
            });

    if (hasFieldInInterface) {
        return true;
    }    

    bool hasReferencedFields = 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return f->commsIsReferenced();
            });

    if (hasReferencedFields) {
        return true;
    }

    auto& allNs = namespaces();
    return 
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto& nsPtr)
            {
                return static_cast<const CommsNamespace*>(nsPtr.get())->commsHasAnyField();
            });    
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
    FrameOptsFunc frameOptsFunc,
    bool hasBase) const
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

    static const std::string Templ = 
        "/// @brief Extra options for #^#DESC#$#.\n"
        "struct #^#NAME#$##^#EXT#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}; // struct #^#NAME#$#\n";

    auto addSubElemFunc = 
        [](std::string&& str, util::StringsList& list)
        {
            if (!str.empty()) {
                list.push_back(std::move(str));
            }
        };

    auto& commsGen = static_cast<const CommsGenerator&>(generator());
    bool hasMainNs = commsGen.hasMainNamespaceInOptions(); 
    auto thisNsScope = comms::scopeFor(*this, generator(), hasMainNs);
    if (!thisNsScope.empty()) {
        thisNsScope.append("::");
    }

    if (fieldOptsFunc != nullptr) {
        util::StringsList fieldElems;
        for (auto* commsField : m_commsFields) {
            addSubElemFunc((commsField->*fieldOptsFunc)(), fieldElems);
        }

        if (!fieldElems.empty()) {
            util::ReplacementMap repl {
                {"NAME", strings::fieldNamespaceStr()},
                {"BODY", util::strListToString(fieldElems, "\n", "")},
                {"DESC", "fields"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::fieldNamespaceStr();
            }

            addStrFunc(util::processTemplate(Templ, repl));
        }
    }

    if (messageOptsFunc != nullptr) {
        util::StringsList messageElems;
        for (auto& msgPtr : messages()) {
            assert(msgPtr);
            addSubElemFunc((static_cast<const CommsMessage*>(msgPtr.get())->*messageOptsFunc)(), messageElems);
        }

        if (!messageElems.empty()) {
            util::ReplacementMap repl {
                {"NAME", strings::messageNamespaceStr()},
                {"BODY", util::strListToString(messageElems, "\n", "")},
                {"DESC", "messages"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::messageNamespaceStr();
            }

            addStrFunc(util::processTemplate(Templ, repl));
        }
    }

    if (frameOptsFunc != nullptr) {
        util::StringsList frameElems;
        for (auto& framePtr : frames()) {
            assert(framePtr);
            addSubElemFunc((static_cast<const CommsFrame*>(framePtr.get())->*frameOptsFunc)(), frameElems);
        } 

        if (!frameElems.empty()) {
            util::ReplacementMap repl {
                {"NAME", strings::frameNamespaceStr()},
                {"BODY", util::strListToString(frameElems, "\n", "")},
                {"DESC", "frames"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::frameNamespaceStr();
            }

            addStrFunc(util::processTemplate(Templ, repl));
        }
    }

    return util::strListToString(elems, "\n", "");
}


} // namespace commsdsl2comms
