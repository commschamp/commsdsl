//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

const std::string& commsOptsTemplInternal(bool defaultNs)
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


CommsNamespace::CommsNamespace(CommsGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    m_dispatch(generator, *this),
    m_factory(generator, *this),
    m_input(generator, *this),
    m_msgId(generator, *this)
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

    auto nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };
    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
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
        return strings::genEmptyString();
    }

    auto& nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }
        
    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
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
        return strings::genEmptyString();
    }

    auto& nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)}
    };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
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
        return strings::genEmptyString();
    }

    auto& nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
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
        return strings::genEmptyString();
    }

    auto& nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
}

std::string CommsNamespace::commsMsgFactoryDefaultOptions() const
{
    auto body = 
        commsOptionsInternal(
            &CommsNamespace::commsMsgFactoryDefaultOptions,
            nullptr,
            nullptr,
            &CommsFrame::commsMsgFactoryDefaultOptions,
            true
        );

    if (body.empty()) {
        return strings::genEmptyString();
    }

    auto& nsName = genName();
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", std::move(body)},
    };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);

    if (!thisNsScope.empty()) {
        repl["EXT"] = ": public TBase::" + thisNsScope;
    }

    return util::genProcessTemplate(commsOptsTemplInternal(nsName.empty()), repl);
}

bool CommsNamespace::commsHasReferencedMsgId() const
{
    return 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return 
                    (f->commsGenField().genIsReferenced()) && 
                    (f->commsGenField().genParseObj().parseSemanticType() == commsdsl::parse::ParseField::ParseSemanticType::MessageId);
            });
}

bool CommsNamespace::commsHasAnyGeneratedCode() const
{
    if ((!genFrames().empty()) ||
        (!genMessages().empty()) || 
        (!genInterfaces().empty())) {
        return true;
    }

    bool hasReferencedFields = 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return f->commsGenField().genIsReferenced();
            });


    if (hasReferencedFields) {
        return true;
    }

    auto& allNs = genNamespaces();
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
    if (!genFrames().empty()) {
        return true;
    }

    auto& allMsgs = genMessages();
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

    auto& allInterfaces = genInterfaces();
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
                return f->commsGenField().genIsReferenced();
            });

    if (hasReferencedFields) {
        return true;
    }

    auto& allNs = genNamespaces();
    return 
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto& nsPtr)
            {
                return static_cast<const CommsNamespace*>(nsPtr.get())->commsHasAnyField();
            });    
}

const CommsField* CommsNamespace::commsFindValidInterfaceReferencedField(const std::string& refStr) const
{
    for (auto& iPtr : genInterfaces()) {
        auto* commsInterface = CommsInterface::commsCast(iPtr.get());
        auto field = commsInterface->commsFindValidReferencedField(refStr);
        if (field != nullptr) {
            return field;
        }
    }

    for (auto& nPtr : genNamespaces()) {
        auto* commsNamespace = CommsNamespace::commsCast(nPtr.get());
        auto field = commsNamespace->commsFindValidInterfaceReferencedField(refStr);
        if (field != nullptr) {
            return field;
        }
    }

    return nullptr;
}

std::string CommsNamespace::commsMsgFactoryAliasType() const
{
    auto result = util::genStrReplace(comms::genScopeFor(*this, genGenerator(), false, true), "::" , "_");
    if (!genName().empty()) {
        result += "_";
    }    
    result += strings::genMsgFactorySuffixStr();
    return result;
}

std::string CommsNamespace::commsMsgFactoryAliasDef(const std::string& namePrefix, const std::string& typeSuffix) const
{
    if ((!genHasFramesRecursive()) ||
        (!genHasMessagesRecursive())) {
        return std::string();
    }

    static const std::string Templ = 
        "using #^#TYPE#$# =\n"
        "    #^#SCOPE#$##^#TYPE_SUFFIX#$#;\n"
    ;

    util::GenReplacementMap repl = {
        {"PREFIX", namePrefix},
        {"TYPE_SUFFIX", typeSuffix},
        {"TYPE", commsMsgFactoryAliasType()},
        {"SCOPE", m_factory.commsScope(namePrefix)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsNamespace::commsRelHeaderPath(const std::string& namePrefix) const
{
    return m_factory.commsRelHeaderPath(namePrefix);
}

bool CommsNamespace::commsHasMsgId() const
{
    return (!genInterfaces().empty());
}

bool CommsNamespace::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_commsFields = CommsField::commsTransformFieldsList(genFields());
    return true;
}

bool CommsNamespace::genWriteImpl() const
{
    if ((commsHasMsgId()) && 
        (!m_msgId.commsWrite())) {
        return false;
    }
    
    if ((!genHasFramesRecursive()) ||
        (!genHasMessagesRecursive())) {
        return true;
    }

    return 
        m_dispatch.commsWrite() &&
        m_factory.commsWrite() &&
        m_input.commsWrite();
}

std::string CommsNamespace::commsOptionsInternal(
    CommsNamespaceOptsFunc nsOptsFunc,
    CommsFieldOptsFunc fieldOptsFunc,
    CommsMessageOptsFunc messageOptsFunc,
    CommsFrameOptsFunc frameOptsFunc,
    bool hasBase) const
{
    util::GenStringsList elems;
    auto addStrFunc = 
        [&elems](std::string&& str)
        {
            if (!str.empty()) {
                elems.push_back(std::move(str));
            }
        };

    auto& subNsList = genNamespaces();
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
        [](std::string&& str, util::GenStringsList& list)
        {
            if (!str.empty()) {
                list.push_back(std::move(str));
            }
        };

    auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
    bool hasMainNs = commsGen.commsHasMainNamespaceInOptions(); 
    auto thisNsScope = comms::genScopeFor(*this, genGenerator(), hasMainNs);
    if (!thisNsScope.empty()) {
        thisNsScope.append("::");
    }

    if (fieldOptsFunc != nullptr) {
        util::GenStringsList fieldElems;
        for (auto* commsField : m_commsFields) {
            addSubElemFunc((commsField->*fieldOptsFunc)(), fieldElems);
        }

        if (!fieldElems.empty()) {
            util::GenReplacementMap repl {
                {"NAME", strings::genFieldNamespaceStr()},
                {"BODY", util::genStrListToString(fieldElems, "\n", "")},
                {"DESC", "fields"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::genFieldNamespaceStr();
            }

            addStrFunc(util::genProcessTemplate(Templ, repl));
        }
    }

    if (messageOptsFunc != nullptr) {
        util::GenStringsList messageElems;
        for (auto& msgPtr : genMessages()) {
            assert(msgPtr);
            addSubElemFunc((static_cast<const CommsMessage*>(msgPtr.get())->*messageOptsFunc)(), messageElems);
        }

        if (!messageElems.empty()) {
            util::GenReplacementMap repl {
                {"NAME", strings::genMessageNamespaceStr()},
                {"BODY", util::genStrListToString(messageElems, "\n", "")},
                {"DESC", "messages"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::genMessageNamespaceStr();
            }

            addStrFunc(util::genProcessTemplate(Templ, repl));
        }
    }

    if (frameOptsFunc != nullptr) {
        util::GenStringsList frameElems;
        for (auto& framePtr : genFrames()) {
            assert(framePtr);
            addSubElemFunc((static_cast<const CommsFrame*>(framePtr.get())->*frameOptsFunc)(), frameElems);
        } 

        if (!frameElems.empty()) {
            util::GenReplacementMap repl {
                {"NAME", strings::genFrameNamespaceStr()},
                {"BODY", util::genStrListToString(frameElems, "\n", "")},
                {"DESC", "frames"}
            };

            if (hasBase) {
                repl["EXT"] = " : public TBase::" + thisNsScope + strings::genFrameNamespaceStr();
            }

            addStrFunc(util::genProcessTemplate(Templ, repl));
        }
    }

    return util::genStrListToString(elems, "\n", "");
}


} // namespace commsdsl2comms
