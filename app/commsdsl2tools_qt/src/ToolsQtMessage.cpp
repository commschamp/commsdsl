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

#include "ToolsQtMessage.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <type_traits>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string& toolsHeaderCodeMultipleInterfacesTemplInternal() 
{
    static const std::string Templ = 
        "struct #^#CLASS_NAME#$#Fields\n"
        "{\n"
        "    static const QVariantList& props();\n"
        "};\n\n"
        "template <typename TInterface>\n"
        "class #^#CLASS_NAME#$# : public\n"
        "    cc_tools_qt::ProtocolMessageBase<\n"
        "        ::#^#PROT_MESSAGE#$#<TInterface, #^#DEF_OPTIONS#$#>,\n"
        "        #^#CLASS_NAME#$#<TInterface>\n"
        "    >\n"
        "{\n"
        "protected:\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override\n"
        "    {\n"
        "        return #^#CLASS_NAME#$#Fields::props();\n"
        "    }\n"
        "};";    
    return Templ;
}

const std::string& toolsHeaderCodeSinglePimplInterfaceTemplInternal() 
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$#Impl;\n"
        "class #^#CLASS_NAME#$# : public #^#TOP_NS#$#::#^#INTERFACE#$#\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = delete;\n"
        "    #^#CLASS_NAME#$#(#^#CLASS_NAME#$#&&) = delete;\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#& operator=(const #^#CLASS_NAME#$#& other);\n"
        "    #^#CLASS_NAME#$#& operator=(#^#CLASS_NAME#$#&&);\n\n"
        "protected:\n"
        "    virtual const char* nameImpl() const override;\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override;\n"
        "    virtual void dispatchImpl(cc_tools_qt::MessageHandler& handler) override;\n"
        "    virtual void resetImpl() override;\n"
        "    virtual bool assignImpl(const cc_tools_qt::Message& other) override;\n"
        "    virtual MsgIdParamType getIdImpl() const override;\n"
        "    virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override;\n"
        "    virtual comms::ErrorStatus writeImpl(WriteIterator& iter, std::size_t len) const override;\n"
        "    virtual bool validImpl() const override;\n"
        "    virtual std::size_t lengthImpl() const override;\n"
        "    virtual bool refreshImpl() override;\n\n"
        "private:\n"
        "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
        "};";

    return Templ;
}

const std::string& toolsHeaderCodeSingleInterfaceWithFieldsTemplInternal() 
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public\n"
        "    cc_tools_qt::ProtocolMessageBase<\n"
        "        ::#^#PROT_MESSAGE#$#<#^#TOP_NS#$#::#^#INTERFACE#$#>,\n"
        "        #^#CLASS_NAME#$#\n"
        "    >\n"    
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = delete;\n"
        "    #^#CLASS_NAME#$#(#^#CLASS_NAME#$#&&) = delete;\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#& operator=(const #^#CLASS_NAME#$#&);\n"
        "    #^#CLASS_NAME#$#& operator=(#^#CLASS_NAME#$#&&);\n\n"
        "protected:\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override;\n"
        "};";    

    return Templ;
}

const std::string& toolsSrcCodeMultipleInterfacesTemplInternal() 
{
    static const std::string Templ =
        "namespace\n"
        "{\n\n"
        "#^#FIELDS_PROPS#$#\n"
        "QVariantList createProps()\n"
        "{\n"
        "    QVariantList props;\n"
        "    #^#PROPS_APPENDS#$#\n"
        "    return props;\n"
        "}\n\n"
        "} // namespace\n\n"
        "const QVariantList& #^#CLASS_NAME#$#Fields::props()\n"
        "{\n"
        "    static const QVariantList Props = createProps();\n"
        "    return Props;\n"
        "}";    
    return Templ;
}

const std::string& toolsSrcCodeSinglePimplInterfaceTemplInternal() 
{
    static const std::string Templ = 
        "namespace\n"
        "{\n\n"
        "#^#FIELDS_PROPS#$#\n"
        "QVariantList createProps()\n"
        "{\n"
        "    QVariantList props;\n"
        "    #^#PROPS_APPENDS#$#\n"
        "    return props;\n"
        "}\n\n"
        "} // namespace\n\n"
        "class #^#CLASS_NAME#$#Impl : public\n"
        "    cc_tools_qt::ProtocolMessageBase<\n"
        "        ::#^#PROT_MESSAGE#$#<#^#TOP_NS#$#::#^#INTERFACE#$#, #^#DEF_OPTIONS#$#>,\n"
        "        #^#CLASS_NAME#$#Impl\n"
        "    >\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#Impl() = default;\n"
        "    #^#CLASS_NAME#$#Impl(const #^#CLASS_NAME#$#Impl&) = delete;\n"
        "    #^#CLASS_NAME#$#Impl(#^#CLASS_NAME#$#Impl&&) = delete;\n"
        "    virtual ~#^#CLASS_NAME#$#Impl() = default;\n"
        "    #^#CLASS_NAME#$#Impl& operator=(const #^#CLASS_NAME#$#Impl&) = default;\n"
        "    #^#CLASS_NAME#$#Impl& operator=(#^#CLASS_NAME#$#Impl&&) = default;\n\n"
        "protected:\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override\n"
        "    {\n"
        "        static const QVariantList Props = createProps();\n"
        "        return Props;\n"
        "    }\n"
        "};\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() : m_pImpl(new #^#CLASS_NAME#$#Impl) {}\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(const #^#CLASS_NAME#$#& other)\n"
        "{\n"
        "    *m_pImpl = *other.m_pImpl;\n"
        "    return *this;\n"
        "}\n\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(#^#CLASS_NAME#$#&& other)\n"
        "{\n"
        "    *m_pImpl = std::move(*other.m_pImpl);\n"
        "    return *this;\n"
        "}\n\n"
        "const char* #^#CLASS_NAME#$#::nameImpl() const\n"
        "{\n"
        "    return static_cast<const cc_tools_qt::Message*>(m_pImpl.get())->name();\n"
        "}\n\n"
        "const QVariantList& #^#CLASS_NAME#$#::fieldsPropertiesImpl() const\n"
        "{\n"
        "    return m_pImpl->fieldsProperties();\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::dispatchImpl(cc_tools_qt::MessageHandler& handler)\n"
        "{\n"
        "    static_cast<cc_tools_qt::Message*>(m_pImpl.get())->dispatch(handler);\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::resetImpl()\n"
        "{\n"
        "    m_pImpl->reset();\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::assignImpl(const cc_tools_qt::Message& other)\n"
        "{\n"
        "    auto* castedOther = dynamic_cast<const #^#CLASS_NAME#$#*>(&other);\n"
        "    if (castedOther == nullptr) {\n"
        "        return false;\n"
        "    }\n"
        "    return m_pImpl->assign(*castedOther->m_pImpl);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::MsgIdParamType #^#CLASS_NAME#$#::getIdImpl() const\n"
        "{\n"
        "    return m_pImpl->getId();\n"
        "}\n\n"
        "comms::ErrorStatus #^#CLASS_NAME#$#::readImpl(ReadIterator& iter, std::size_t len)\n"
        "{\n"
        "    return m_pImpl->read(iter, len);\n"
        "}\n\n"
        "comms::ErrorStatus #^#CLASS_NAME#$#::writeImpl(WriteIterator& iter, std::size_t len) const\n"
        "{\n"
        "    return m_pImpl->write(iter, len);\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::validImpl() const\n"
        "{\n"
        "    return m_pImpl->valid();\n"
        "}\n\n"
        "std::size_t #^#CLASS_NAME#$#::lengthImpl() const\n"
        "{\n"
        "    return m_pImpl->length();\n"
        "}\n\n"  
        "bool #^#CLASS_NAME#$#::refreshImpl()\n"
        "{\n"
        "    return m_pImpl->refresh();\n"
        "}";    

    return Templ;
}

const std::string& toolsSrcCodeSingleInterfaceWithFieldsInternal()
{
    static const std::string Templ =
        "namespace\n"
        "{\n\n"
        "#^#FIELDS_PROPS#$#\n"
        "QVariantList createProps()\n"
        "{\n"
        "    QVariantList props;\n"
        "    #^#PROPS_APPENDS#$#\n"
        "    return props;\n"
        "}\n\n"
        "} // namespace\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() = default;\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(const #^#CLASS_NAME#$#&) = default;\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(#^#CLASS_NAME#$#&&) = default;\n\n"
        "const QVariantList& #^#CLASS_NAME#$#::fieldsPropertiesImpl() const\n"
        "{\n"
        "    static const QVariantList Props = createProps();\n"
        "    return Props;\n"
        "}";    
    return Templ;
}
    
} // namespace 
    

ToolsQtMessage::ToolsQtMessage(ToolsQtGenerator& generator, commsdsl::parse::Message dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

ToolsQtMessage::StringsList ToolsQtMessage::toolsSourceFiles() const
{
    return StringsList{toolsRelPathInternal() + strings::cppSourceSuffixStr()};
}

bool ToolsQtMessage::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_exists = 
        generator().doesElementExist(
            dslObj().sinceVersion(),
            dslObj().deprecatedSince(),
            dslObj().isDeprecatedRemoved());   

    if (!m_exists) {
        return true;
    }

    m_toolsFields = ToolsQtField::toolsTransformFieldsList(fields());
    return true;
}

bool ToolsQtMessage::writeImpl() const
{
    if (!m_exists) {
        return true;
    }

    return toolsWriteHeaderInternal() && toolsWriteSrcInternal();
}

bool ToolsQtMessage::toolsWriteHeaderInternal() const
{
    auto& gen = generator();
    auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal() + strings::cppHeaderSuffixStr();

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }

    auto includes = toolsHeaderIncludesInternal();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"DEF", toolsHeaderCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtMessage::toolsWriteSrcInternal() const
{
    auto& gen = generator();
    auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal() + strings::cppSourceSuffixStr();

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    auto includes = toolsSrcIncludesInternal();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"DEF", toolsSrcCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string ToolsQtMessage::toolsRelPathInternal() const
{
    auto scope = comms::scopeFor(*this, generator());
    return generator().getTopNamespace() + '/' + util::strReplace(scope, "::", "/");
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsHeaderIncludesInternal() const
{
    using Func = IncludesList (ToolsQtMessage::*)() const;
    static const Func Map[] = {
        /* CodeType_MultipleInterfaces */ &ToolsQtMessage::toolsHeaderIncludesMultipleInterfacesInternal,
        /* CodeType_SinglePimplInterface */ &ToolsQtMessage::toolsHeaderIncludesSinglePimplInterfaceInternal,
        /* CodeType_SingleInterfaceWithFields */ &ToolsQtMessage::toolsHeaderIncludesSingleInterfaceWithFieldsInternal,        
    };
    static const auto MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CodeType_NumOfValues);

    auto codeType = toolCodeType();
    assert(codeType <= MapSize);
    auto func = Map[codeType];
    return (this->*func)();
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsHeaderIncludesMultipleInterfacesInternal() const
{
    return IncludesList {
        "<QtCore/QVariantList>",
        "cc_tools_qt/ProtocolMessageBase.h",
        comms::relHeaderPathFor(*this, generator()),
        ToolsQtDefaultOptions::toolsRelHeaderPath(static_cast<const ToolsQtGenerator&>(generator())),
    };
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsHeaderIncludesSinglePimplInterfaceInternal() const
{
    auto interfaces = generator().getAllInterfaces();
    assert(!interfaces.empty());
    auto* defaultInterface = static_cast<const ToolsQtInterface*>(interfaces.front());
    assert(defaultInterface != nullptr);
    return IncludesList {
        "<memory>",
        "<QtCore/QVariantList>",
        defaultInterface->toolsHeaderFilePath()
    };
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsHeaderIncludesSingleInterfaceWithFieldsInternal() const
{
    auto interfaces = generator().getAllInterfaces();
    assert(!interfaces.empty());
    auto* defaultInterface = static_cast<const ToolsQtInterface*>(interfaces.front());
    assert(defaultInterface != nullptr);

    return IncludesList {
        "<QtCore/QVariantList>",
        "cc_tools_qt/ProtocolMessageBase.h",
        comms::relHeaderPathFor(*this, generator()),
        defaultInterface->toolsHeaderFilePath(),
        ToolsQtDefaultOptions::toolsRelHeaderPath(static_cast<const ToolsQtGenerator&>(generator())),
    };
}

std::string ToolsQtMessage::toolsHeaderCodeInternal() const
{
    using Func = const std::string& (*)();
    static const Func Map[] = {
        /* CodeType_MultipleInterfaces */ &toolsHeaderCodeMultipleInterfacesTemplInternal,
        /* CodeType_SinglePimplInterface */ &toolsHeaderCodeSinglePimplInterfaceTemplInternal,
        /* CodeType_SingleInterfaceWithFields */ &toolsHeaderCodeSingleInterfaceWithFieldsTemplInternal,        
    };
    static const auto MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CodeType_NumOfValues);

    auto codeType = toolCodeType();
    assert(codeType <= MapSize);
    auto func = Map[codeType];

    auto& gen = static_cast<const ToolsQtGenerator&>(generator());
    auto interfaces = gen.getAllInterfaces();
    assert(!interfaces.empty());
    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"TOP_NS", generator().getTopNamespace()},
        {"INTERFACE", comms::scopeFor(*interfaces.front(), gen)},
        {"PROT_MESSAGE", comms::scopeFor(*this, gen)},
        {"DEF_OPTIONS", ToolsQtDefaultOptions::toolsScope(gen)},
    };

    return util::processTemplate(func(), repl);
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsSrcIncludesInternal() const
{
    using Func = IncludesList (ToolsQtMessage::*)() const;
    static const Func Map[] = {
        /* CodeType_MultipleInterfaces */ &ToolsQtMessage::toolsSrcIncludesMultipleInterfacesInternal,
        /* CodeType_SinglePimplInterface */ &ToolsQtMessage::toolsSrcIncludesSinglePimplInterfaceInternal,
        /* CodeType_SingleInterfaceWithFields */ &ToolsQtMessage::toolsSrcIncludesSingleInterfaceWithFieldsInternal,        
    };
    static const auto MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CodeType_NumOfValues);

    auto codeType = toolCodeType();
    assert(codeType <= MapSize);
    auto func = Map[codeType];
    auto result = (this->*func)();
    for (auto* f : m_toolsFields) {
        auto incs = f->toolsSrcIncludes();
        result.reserve(result.size() + incs.size());
        std::move(incs.begin(), incs.end(), std::back_inserter(result));
    }   

    return result; 
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsSrcIncludesMultipleInterfacesInternal() const
{
    return IncludesList {
        "cc_tools_qt/property/field.h"
    };
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsSrcIncludesSinglePimplInterfaceInternal() const
{
    return IncludesList {
        "cc_tools_qt/property/field.h",
        "cc_tools_qt/ProtocolMessageBase.h",
        comms::relHeaderPathFor(*this, generator()),
        ToolsQtDefaultOptions::toolsRelHeaderPath(static_cast<const ToolsQtGenerator&>(generator())),
    };
}

ToolsQtMessage::IncludesList ToolsQtMessage::toolsSrcIncludesSingleInterfaceWithFieldsInternal() const
{
    return IncludesList {
        "cc_tools_qt/property/field.h"
    };
}

std::string ToolsQtMessage::toolsSrcCodeInternal() const
{
    using Func = const std::string& (*)();
    static const Func Map[] = {
        /* CodeType_MultipleInterfaces */ &toolsSrcCodeMultipleInterfacesTemplInternal,
        /* CodeType_SinglePimplInterface */ &toolsSrcCodeSinglePimplInterfaceTemplInternal,
        /* CodeType_SingleInterfaceWithFields */ &toolsSrcCodeSingleInterfaceWithFieldsInternal,        
    };
    static const auto MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CodeType_NumOfValues);

    auto codeType = toolCodeType();
    assert(codeType <= MapSize);
    auto func = Map[codeType];

    auto& gen = static_cast<const ToolsQtGenerator&>(generator());
    auto interfaces = generator().getAllInterfaces();
    assert(!interfaces.empty());

    util::StringsList fieldsProps;
    util::StringsList appends;
    fieldsProps.reserve(m_toolsFields.size());
    appends.reserve(m_toolsFields.size());
    for (auto* f : m_toolsFields) {
        auto membersStr = f->toolsDefMembers();
        if (!membersStr.empty()) {
            fieldsProps.push_back(std::move(membersStr));
        }

        fieldsProps.push_back(f->toolsDefFunc());
        appends.push_back("props.append(createProps_" + comms::accessName(f->field().dslObj().name()) + "(false));");
    }    

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"PROT_MESSAGE", comms::scopeFor(*this, gen)},
        {"DEF_OPTIONS", ToolsQtDefaultOptions::toolsScope(gen)},
        {"TOP_NS", generator().getTopNamespace()},
        {"INTERFACE", comms::scopeFor(*interfaces.front(), generator())},
        {"FIELDS_PROPS", util::strListToString(fieldsProps, "\n", "")},
        {"PROPS_APPENDS", util::strListToString(appends, "\n", "")}
    };

    return util::processTemplate(func(), repl);    
}

ToolsQtMessage::CodeType ToolsQtMessage::toolCodeType() const
{
    auto interfaces = generator().getAllInterfaces();
    assert(!interfaces.empty());
    if (1U < interfaces.size()) {
        return CodeType_MultipleInterfaces;
    }

    auto defaultInterface = static_cast<const ToolsQtInterface*>(interfaces.front());
    assert(defaultInterface != nullptr);
    auto& fields = defaultInterface->fields();
    if (fields.empty()) {
        return CodeType_SinglePimplInterface;
    }

    return CodeType_SingleInterfaceWithFields;
}

} // namespace commsdsl2tools_qt
