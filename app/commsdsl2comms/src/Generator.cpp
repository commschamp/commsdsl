//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "Generator.h"

#include <fstream>
#include <iterator>
#include <algorithm>
#include <cctype>

#include <boost/algorithm/string.hpp>

#include "Namespace.h"
#include "FieldBase.h"
#include "DefaultOptions.h"
#include "MsgId.h"
#include "Interface.h"
#include "common.h"
#include "EnumField.h"
#include "AllMessages.h"
#include "Cmake.h"
#include "Doxygen.h"
#include "Version.h"
#include "Test.h"
#include "Dispatch.h"
#include "License.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const unsigned MaxDslVersion = 3U;
const std::string MinCommsVersionStr("2, 2, 1");
const std::string ScopeSep("::");
const std::string ReplaceSuffix(".replace");
const std::string ExtendSuffix(".extend");
const std::string ReadSuffix(".read");
const std::string WriteSuffix(".write");
const std::string LengthSuffix(".length");
const std::string ValidSuffix(".valid");
const std::string RefreshSuffix(".refresh");
const std::string NameSuffix(".name");
const std::string PublicSuffix(".public");
const std::string ProtectedSuffix(".protected");
const std::string PrivateSuffix(".private");
const std::string IncSuffix(".inc");
const std::string AppendSuffix(".append");

const std::string ReservedExt[] = {
    ReplaceSuffix,
    ExtendSuffix,
    PublicSuffix,
    ProtectedSuffix,
    PrivateSuffix,
    ReadSuffix,
    WriteSuffix,
    LengthSuffix,
    ValidSuffix,
    RefreshSuffix,
    NameSuffix,
    IncSuffix,
    AppendSuffix,
};

std::string refToNs(const std::string& ref)
{
    std::size_t pos = ref.find_last_of('.');
    if (pos == std::string::npos) {
        return common::emptyString();
    }

    return std::string(ref, 0, pos);
}

std::string refToName(const std::string& ref)
{
    std::size_t pos = ref.find_last_of('.');
    if (pos == std::string::npos) {
        return ref;
    }

    return std::string(ref, pos + 1);
}

bf::path refToPath(const std::string& ref)
{
    std::vector<std::string> tokens;
    static const auto Sep = ba::is_any_of(".");
    ba::split(tokens, ref, Sep);

    bf::path result;
    if (tokens.empty()) {
        assert(!"Should not happen");
        return result;
    }

    result = tokens[0];
    for (auto idx = 1U; idx < tokens.size(); ++idx) {
        result /= tokens[idx];
    }

    return result;
}

std::vector<std::string> splitRefPath(const std::string& ref)
{
    std::vector<std::string> tokens;
    static const auto Sep = ba::is_any_of(".");
    ba::split(tokens, ref, Sep);
    return tokens;
}

} // namespace

bool Generator::generate(const FilesList& files)
{
    m_protocol.setErrorReportCallback(
        [this](commsdsl::ErrorLevel level, const std::string& msg)
        {
            m_logger.log(level, msg);
        });

    return
        parseOptions() &&
        parseSchemaFiles(files) &&
        prepare() &&
        writeFiles();
}

bool Generator::doesElementExist(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    unsigned sVersion = schemaVersion();

    if (sVersion < sinceVersion) {
        return false;
    }

    if (deprecatedRemoved && (deprecatedSince <= m_minRemoteVersion)) {
        return false;
    }

    return true;
}

bool Generator::isElementOptional(
    unsigned sinceVersion,
    unsigned deprecatedSince,
    bool deprecatedRemoved) const
{
    if (m_minRemoteVersion < sinceVersion) {
        return true;
    }

    if (deprecatedRemoved && (deprecatedSince < commsdsl::Protocol::notYetDeprecated())) {
        return true;
    }

    return false;
}

std::vector<std::string> Generator::extraMessagesBundles() const
{
    std::vector<std::string> result;
    result.reserve(m_extraMessages.size());
    std::transform(
        m_extraMessages.begin(), m_extraMessages.end(), std::back_inserter(result),
        [](const auto& elem) -> const std::string&
        {
            return elem.m_name;
        });
    return result;
}

std::vector<std::string> Generator::bundlesForMessage(const std::string& externalRef)
{
    std::vector<std::string> result;
    for (auto& info : m_extraMessages) {
        auto iter =
            std::lower_bound(
                info.m_dslNames.begin(), info.m_dslNames.end(), externalRef,
                [](const std::string& n, const std::string& ref)
                {
                    return n < ref;
                });

        if ((iter == info.m_dslNames.end()) || (*iter != externalRef)) {
            continue;
        }

        result.push_back(info.m_name);
    }
    return result;
}

std::string Generator::protocolDefRootDir()
{
    auto dir = getProtocolDefRootDir();
    if (!createDir(dir)) {
        m_logger.error("Failed to create \"" + dir.string() + "\" directory.");
        return common::emptyString();
    }

    return dir.string();
}

std::string Generator::outputDir()
{
    if (!createDir(m_pathPrefix)) {
        m_logger.error("Failed to create \"" + m_pathPrefix.string() + "\" directory.");
        return common::emptyString();
    }

    return m_pathPrefix.string();
}

std::string Generator::pluginDir()
{
    auto dir = m_pathPrefix / common::pluginNsStr();
    if (!createDir(dir)) {
        m_logger.error("Failed to create \"" + dir.string() + "\" directory.");
        return common::emptyString();
    }

    return dir.string();
}

std::string Generator::testDir()
{
    auto dir = m_pathPrefix / common::testStr();
    if (!createDir(dir)) {
        m_logger.error("Failed to create \"" + dir.string() + "\" directory.");
        return common::emptyString();
    }

    return dir.string();
}

std::pair<std::string, std::string> Generator::startMessageProtocolWrite(
    const std::string& externalRef)
{
    return startProtocolWrite(externalRef, common::messageStr());
}

std::pair<std::string, std::string> Generator::startFrameProtocolWrite(
    const std::string& externalRef)
{
    return startProtocolWrite(externalRef, common::frameStr());
}

std::pair<std::string, std::string>
Generator::startFrameTransportMessageProtocolHeaderWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef + common::transportMessageSuffixStr(), true, common::frameStr());
}

std::pair<std::string, std::string>
Generator::startFrameTransportMessageProtocolSrcWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef + common::transportMessageSuffixStr(), false, common::frameStr());
}

std::pair<std::string, std::string>
Generator::startFrameProtocolHeaderWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, true, common::frameStr());
}

std::pair<std::string, std::string> Generator::startInterfaceProtocolWrite(
    const std::string& externalRef)
{
    if (!externalRef.empty()) {
        return startProtocolWrite(externalRef);
    }
    return startProtocolWrite(common::messageClassStr());
}

std::pair<std::string, std::string> Generator::startFieldProtocolWrite(
    const std::string& externalRef)
{
    return startProtocolWrite(externalRef, common::fieldStr());
}

std::pair<std::string, std::string>
Generator::startFieldPluginHeaderWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, true, common::fieldStr());
}

std::pair<std::string, std::string>
Generator::startFieldPluginSrcWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, false, common::fieldStr());
}

std::pair<std::string, std::string>
Generator::startInterfacePluginHeaderWrite(const std::string& externalRef)
{
    if (!externalRef.empty()) {
        return startPluginWrite(externalRef, true);
    }

    return startPluginWrite(common::messageClassStr(), true);
}

std::pair<std::string, std::string>
Generator::startInterfacePluginSrcWrite(const std::string& externalRef)
{
    if (!externalRef.empty()) {
        return startPluginWrite(externalRef, false);
    }

    return startPluginWrite(common::messageClassStr(), false);
}

std::pair<std::string, std::string>
Generator::startMessagePluginHeaderWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, true, common::messageStr());
}

std::pair<std::string, std::string>
Generator::startMessagePluginSrcWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, false, common::messageStr());
}

std::pair<std::string, std::string>
Generator::startProtocolPluginHeaderWrite(const std::string& name)
{
    return startPluginWrite(name, true, common::pluginStr());
}

std::pair<std::string, std::string>
Generator::startProtocolPluginSrcWrite(const std::string& name)
{
    return startPluginWrite(name, false, common::pluginStr());
}

std::string Generator::startProtocolPluginCommonWrite(
    const std::string& name,
    const std::string& extension)
{
    if (name.empty()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto ns = refToNs(name);
    auto relDirPath = bf::path(common::pluginNsStr()) / refToPath(ns) / common::pluginStr();
    auto dirPath = m_pathPrefix / relDirPath;

    if (!createDir(dirPath)) {
        return common::emptyString();
    }

    auto className = refToName(name);
    assert(!className.empty());
    common::nameToClass(className);

    auto fileName = className + extension;
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto overwriteFile = *iter / relDirPath / fileName;
        boost::system::error_code ec;
        if (bf::exists(overwriteFile, ec)) {
            m_logger.info("Skipping generation of " + fullPathStr);
            return common::emptyString();
        }

        auto replaceFile = *iter / relDirPath / (fileName + ReplaceSuffix);
        if (bf::exists(replaceFile, ec)) {
            m_logger.info("Replacing " + fullPathStr + " with " + replaceFile.string());
            bf::copy_file(replaceFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write " + fullPathStr);
                assert(!"Should not happen");
            }
            return common::emptyString();
        }
    }

    m_logger.info("Generating " + fullPathStr);
    return fullPathStr;
}

std::pair<std::string, std::string>
Generator::startOptionsProtocolWrite(const std::string& name)
{
    return startProtocolWrite(name, common::optionsStr());
}

std::pair<std::string, std::string>
Generator::startInputProtocolWrite(const std::string& name)
{
    return startProtocolWrite(name, common::inputStr());
}

std::pair<std::string, std::string>
Generator::startInputPluginHeaderWrite(const std::string& externalRef)
{
    return startPluginWrite(externalRef, true, common::inputStr());
}

std::pair<std::string, std::string>
Generator::startDispatchProtocolWrite(const std::string& name)
{
    return startProtocolWrite(name, common::dispatchStr());
}

std::pair<std::string, std::string>
Generator::startGenericProtocolWrite(const std::string& name)
{
    return startProtocolWrite(name);
}

std::pair<std::string, std::string>
Generator::startGenericPluginHeaderWrite(const std::string& name)
{
    return startPluginWrite(name, true);
}

std::string Generator::startProtocolDocWrite(const std::string& name)
{
    return startGenericWrite(name, common::docStr());
}

std::string Generator::startLicenseWrite(const std::string& name)
{
    return startGenericWrite(name);
}

std::pair<std::string, std::string>
Generator::startGenericPluginSrcWrite(const std::string& name)
{
    return startPluginWrite(name, false);
}

std::pair<std::string, std::string> Generator::namespacesForMessage(
    const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::messageStr());
}

std::pair<std::string, std::string>
Generator::namespacesForMessageInPlugin(const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::messageStr(), true);
}

std::pair<std::string, std::string> Generator::namespacesForFrame(
    const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::frameStr());
}

std::pair<std::string, std::string>
Generator::namespacesForFrameInPlugin(const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::frameStr(), true);
}

std::pair<std::string, std::string> Generator::namespacesForField(
    const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::fieldStr());
}

std::pair<std::string, std::string> Generator::namespacesForFieldInPlugin(
    const std::string& externalRef) const
{
    return namespacesForElement(externalRef, common::fieldStr(), true);
}

std::pair<std::string, std::string> Generator::namespacesForInterface(
    const std::string& externalRef) const
{
    if (externalRef.empty()) {
        return namespacesForRoot();
    }

    return namespacesForElement(externalRef);
}

std::pair<std::string, std::string>
Generator::namespacesForInterfaceInPlugin(const std::string& externalRef) const
{
    if (externalRef.empty()) {
        return namespacesForElement(common::messageClassStr(), common::emptyString(), true);
    }

    return namespacesForElement(externalRef, common::emptyString(), true);
}

std::pair<std::string, std::string>
Generator::namespacesForPluginDef(const std::string& name) const
{
    return namespacesForElement(name, common::pluginStr(), true);
}

std::pair<std::string, std::string>
Generator::namespacesForOptions() const
{
    return namespacesForElement(common::emptyString(), common::optionsStr());
}

std::pair<std::string, std::string>
Generator::namespacesForInput() const
{
    return namespacesForElement(common::emptyString(), common::inputStr());
}

std::pair<std::string, std::string>
Generator::namespacesForInputInPlugin() const
{
    return namespacesForElement(common::emptyString(), common::inputStr(), true);
}

std::pair<std::string, std::string>
Generator::namespacesForDispatch() const
{
    return namespacesForElement(common::emptyString(), common::dispatchStr());
}

std::pair<std::string, std::string>
Generator::namespacesForRoot() const
{
    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n";

    std::string endStr = "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::pair<std::string, std::string> Generator::namespacesForPlugin() const
{
    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n\n"
        "namespace cc_plugin\n"
        "{\n";

    std::string endStr =
        "} // namespace cc_plugin\n\n"
        "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::string Generator::headerfileForMessage(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::messageStr());
}

std::string Generator::headerfileForMessageInPlugin(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::messageStr(), true);
}

std::string Generator::headerfileForFrame(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::frameStr());
}

std::string Generator::headerfileForFrameInPlugin(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::frameStr(), true);
}

std::string Generator::headerfileForField(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::fieldStr());
}

std::string Generator::headerfileForFieldInPlugin(const std::string& externalRef, bool quotes)
{
    return headerfileForElement(externalRef, quotes, common::fieldStr(), true);
}

std::string Generator::headerfileForInterface(const std::string& externalRef, bool quotes)
{
    std::string externalRefCpy(externalRef);
    if (externalRefCpy.empty()) {
        externalRefCpy = common::messageClassStr();
    }

    return headerfileForElement(externalRefCpy, quotes);
}

std::string Generator::headerfileForInterfaceInPlugin(const std::string& externalRef, bool quotes)
{
    std::string externalRefCpy(externalRef);
    if (externalRefCpy.empty()) {
        externalRefCpy = common::messageClassStr();
    }

    return headerfileForElement(externalRefCpy, quotes, common::emptyString(), true);
}

std::string Generator::headerfileForCustomChecksum(const std::string& name, bool quotes)
{
    static const std::vector<std::string> subNs = {
        common::frameStr(),
        common::checksumStr()
    };

    return headerfileForElement(name, quotes, subNs);
}

std::string Generator::headerfileForCustomLayer(const std::string& name, bool quotes)
{
    static const std::vector<std::string> subNs = {
        common::frameStr(),
        common::layerStr()
    };

    return headerfileForElement(name, quotes, subNs);
}

std::string Generator::headerfileForOptions(const std::string& name, bool quotes)
{
    return headerfileForElement(name, quotes, common::optionsStr());
}

std::string Generator::headerfileForInput(const std::string& name, bool quotes)
{
    return headerfileForElement(name, quotes, common::inputStr());
}

std::string Generator::headerfileForInputInPlugin(const std::string& name, bool quotes)
{
    return headerfileForElement(name, quotes, common::inputStr(), true);
}

std::string Generator::headerfileForDispatch(const std::string& name, bool quotes)
{
    return headerfileForElement(name, quotes, common::dispatchStr());
}

std::string Generator::headerfileForRoot(const std::string& name, bool quotes)
{
    return headerfileForElement(name, quotes);
}

std::string Generator::scopeForMessage(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(externalRef, mainIncluded, classIncluded, common::messageStr());
}

std::string Generator::scopeForMessageInPlugin(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(externalRef, mainIncluded, classIncluded, common::messageStr(), true);
}

std::string Generator::scopeForInterface(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded)
{
    if (!externalRef.empty()) {
        return scopeForElement(externalRef, mainIncluded, classIncluded);
    }

    return scopeForElement(common::messageClassStr(), mainIncluded, classIncluded);
}

std::string Generator::scopeForInterfaceInPlugin(const std::string& externalRef)
{
    if (!externalRef.empty()) {
        return scopeForElement(externalRef, true, true, common::emptyString(), true);
    }

    return scopeForElement(common::messageClassStr(), true, true, common::emptyString(), true);
}

std::string Generator::scopeForFrame(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(externalRef, mainIncluded, classIncluded, common::frameStr());
}

std::string Generator::scopeForFrameInPlugin(const std::string& externalRef)
{
    return scopeForElement(externalRef, true, true, common::frameStr(), true);
}

std::string Generator::scopeForField(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(externalRef, mainIncluded, classIncluded, common::fieldStr());
}

std::string Generator::scopeForFieldInPlugin(const std::string& externalRef)
{
    return scopeForElement(externalRef, false, false, common::fieldStr(), true);
}

std::string Generator::scopeForCustomChecksum(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    static const std::vector<std::string> SubNs = {
        common::frameStr(),
        common::checksumStr()
    };

    return scopeForElement(name, mainIncluded, classIncluded, SubNs);
}

std::string Generator::scopeForCustomLayer(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    static const std::vector<std::string> SubNs = {
        common::frameStr(),
        common::layerStr()
    };

    return scopeForElement(name, mainIncluded, classIncluded, SubNs);
}

std::string Generator::scopeForNamespace(
    const std::string& externalRef,
    bool mainIncluded,
    bool appendSep)
{
    std::string result;
    if (mainIncluded) {
        result += (mainNamespace() + ScopeSep);
    }  
    
    if (!externalRef.empty()) {
        result += ba::replace_all_copy(externalRef, ".", "::");
        if (appendSep) {
            result += ScopeSep;
        }
    }

    return result;
}

std::string Generator::scopeForOptions(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(name, mainIncluded, classIncluded, common::optionsStr());
}

std::string Generator::scopeForInput(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(name, mainIncluded, classIncluded, common::inputStr());
}

std::string Generator::scopeForInputInPlugin(const std::string& externalRef)
{
    return scopeForElement(externalRef, true, true, common::inputStr(), true);
}

std::string Generator::scopeForDispatch(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(name, mainIncluded, classIncluded, common::dispatchStr());
}

std::string Generator::scopeForRoot(
    const std::string& name,
    bool mainIncluded,
    bool classIncluded)
{
    return scopeForElement(name, mainIncluded, classIncluded);
}

std::string Generator::getDefaultOptionsBody() const
{
    return getOptionsBody(&Namespace::getDefaultOptions);
}

std::string Generator::getClientDefaultOptionsBody() const
{
    return getOptionsBody(&Namespace::getClientOptions);
}

std::string Generator::getServerDefaultOptionsBody() const
{
    return getOptionsBody(&Namespace::getServerOptions);
}

std::string Generator::getBareMetalDefaultOptionsBody() const
{
    return getOptionsBody(&Namespace::getBareMetalDefaultOptions);
}

std::string Generator::getMessageIdStr(const std::string& externalRef, std::uintmax_t id) const
{
    if (m_messageIdField == nullptr) {
        return m_mainNamespace + "::" + common::msgIdPrefixStr() + ba::replace_all_copy(externalRef, ".", "_");
    }

    assert(m_messageIdField->kind() == commsdsl::Field::Kind::Enum);
    auto* castedEnumField = static_cast<const EnumField*>(m_messageIdField);

    auto name = castedEnumField->getValueName(static_cast<std::intmax_t>(id));
    if (!name.empty()) {
        return  m_mainNamespace + "::" + common::msgIdPrefixStr() + name;
    }

    return common::numToString(id);
}

const Field* Generator::getMessageIdField() const
{
    return m_messageIdField;
}

Generator::MessageIdMap Generator::getAllMessageIds() const
{
    MessageIdMap result;
    for (auto& n : m_namespaces) {
        auto messages = n->getAllMessages();
        for (auto* m : messages) {
            result.insert(
                std::make_pair(
                    m->id(),
                    common::msgIdPrefixStr() + ba::replace_all_copy(m->externalRef(), ".", "_")
                ));
        }
    }

    return result;
}

const Field* Generator::findField(const std::string& externalRef, bool record)
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
                return ns->name() < n;
            });

    if ((nsIter == m_namespaces.end()) || ((*nsIter)->name() != nsName)) {
        m_logger.error("Internal error: unknown external reference: " + externalRef);
        assert(!"Should not happen");
        return nullptr;
    }

    auto fromPos = 0U;
    if (pos != std::string::npos) {
        fromPos = pos + 1U;
    }
    std::string remStr(externalRef, fromPos);
    auto result = (*nsIter)->findField(remStr, record);
    if (result == nullptr) {
        m_logger.error("Internal error: unknown external reference: " + externalRef);
        assert(!"Should not happen");
    }
    return result;
}

const Interface* Generator::findInterface(const std::string& externalRef)
{
    if (externalRef.empty()) {
        auto allInterfaces = getAllInterfaces();
        assert(!allInterfaces.empty());
        return allInterfaces.front();
    }

    auto ns = refToNs(externalRef);
    auto iter =
        std::find_if(
            m_namespaces.begin(), m_namespaces.end(),
            [&ns](auto& n)
            {
                return n->name() == ns;
            });

    if (iter == m_namespaces.end()) {
        return nullptr;
    }

    return (*iter)->findInterface(std::string(externalRef, ns.size() + 1));
}

const Frame* Generator::findFrame(const std::string& externalRef)
{
    if (externalRef.empty()) {
        auto allFrames = getAllFrames();
        assert(!allFrames.empty());
        return allFrames.front();
    }

    auto ns = refToNs(externalRef);
    auto iter =
        std::find_if(
            m_namespaces.begin(), m_namespaces.end(),
            [&ns](auto& n)
            {
                return n->name() == ns;
            });

    if (iter == m_namespaces.end()) {
        return nullptr;
    }

    return (*iter)->findFrame(std::string(externalRef, ns.size() + 1));

}

bool Generator::parseOptions()
{
    auto outputDir = m_options.getOutputDirectory();
    m_pathPrefix = outputDir;
    if (outputDir.empty()) {
        boost::system::error_code ec;
        m_pathPrefix = bf::current_path(ec);

        if (ec) {
            m_logger.error("Failed to retrieve current directory with reason: " + ec.message());
            return false;
        }
    }

    auto codeInptuDirs = m_options.getCodeInputDirectories();
    m_codeInputDirs.reserve(codeInptuDirs.size());
    for (auto iter = codeInptuDirs.begin(); iter != codeInptuDirs.end(); ++iter) {
        if ((!iter->empty()) && (!bf::is_directory(*iter))) {
            m_logger.error('\"' + *iter + "\" is expected to be directory.");
            return false;
        }

        m_codeInputDirs.push_back(std::move(*iter));
    }

    m_mainNamespace = common::adjustName(m_options.getNamespace());

    if (!parseCustomization()) {
        return false;
    }
    return true;
}

bool Generator::parseCustomization()
{
    static const std::string Map[] = {
        "full",
        "limited",
        "none"
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == (unsigned)CustomizationLevel::NumOfValues,
        "Invalid map");

    auto level = m_options.getCustomizationLevel();
    common::toLower(level);

    auto iter = std::find(std::begin(Map), std::end(Map), level);
    if (iter != std::end(Map)) {
        m_customizationLevel = 
            static_cast<decltype(m_customizationLevel)>(
                std::distance(std::begin(Map), iter));
        return true;
    }

    m_logger.warning("Unexpected customization level requested: \"" + level + "\".");
    if (m_logger.hadWarning()) {
        return false;
    }

    return true;
}

bool Generator::parseSchemaFiles(const FilesList& files)
{
    for (auto& f : files) {
        m_logger.log(commsdsl::ErrorLevel_Info, "Parsing " + f);
        if (!m_protocol.parse(f)) {
            return false;
        }

        if (m_logger.hadWarning()) {
            m_logger.log(commsdsl::ErrorLevel_Error, "Warning treated as error");
            return false;
        }
    }

    if (!m_protocol.validate()) {
        return false;
    }

    if (m_logger.hadWarning()) {
        m_logger.error("Warning treated as error");
        return false;
    }

    auto schema = m_protocol.schema();
    if (m_mainNamespace.empty()) {
        assert(!schema.name().empty());
        m_mainNamespace = common::adjustName(schema.name());
        m_schemaNamespace = m_mainNamespace;
    }

    m_schemaEndian = schema.endian();
    m_schemaVersion = schema.version();
    if (m_options.hasForcedSchemaVersion()) {
        auto newVersion = m_options.getForcedSchemaVersion();
        if (m_schemaVersion < newVersion) {
            m_logger.error("Cannot force version to be greater than " + common::numToString(m_schemaVersion));
            return false;
        }

        m_schemaVersion = newVersion;
    }

    if (MaxDslVersion < schema.dslVersion()) {
        m_logger.error(
            "Required DSL version is too big (" + std::to_string(schema.dslVersion()) +
            "), upgrade your code generator.");
        return false;
    }

    m_minRemoteVersion = m_options.getMinRemoteVersion();

    return true;
}

bool Generator::prepare()
{
    auto namespaces = m_protocol.namespaces();
    m_namespaces.reserve(namespaces.size());
    for (auto& n : namespaces) {
        auto ns = createNamespace(*this, n);

        if (!ns->prepare()) {
            return false;
        }

        m_namespaces.push_back(std::move(ns));
    }

    if (!m_options.versionIndependentCodeRequested()) {
        m_versionDependentCode = anyInterfaceHasVersion();
    }

    if (mustDefineDefaultInterface()) {
        auto& ns = findOrCreateDefaultNamespace();
        if (!ns.addDefaultInterface()) {
            return false;
        }
    }

    bool hasFrame =
        std::any_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& n)
            {
                return n->hasFrame();
            });

    if (!hasFrame) {
        m_logger.error("Schema file(s) must define at least one frame.");
        return false;
    }

    if (!preparePlugins()) {
        return false;
    }

    if (!prepareExternalMessages()) {
        return false;
    }

    m_messageIdField = findMessageIdField();
    return true;
}

bool Generator::writeFiles()
{
    if ((!FieldBase::write(*this)) ||
        (!MsgId::write(*this)) ||
        (!Version::write(*this)) ||
        (!AllMessages::write(*this)) ||
        (!Dispatch::write(*this))) {
        return false;
    }

    for (auto& ns : m_namespaces) {
        if ((!ns->writeInterfaces()) ||
            (!ns->writeMessages()) ||
            (!ns->writeFrames())) {
            return false;
        }

    }

    for (auto& ns : m_namespaces) {
        if (!ns->writeFields()) {
            return false;
        }
    }

    if ((!DefaultOptions::write(*this)) ||
        (!Cmake::write(*this)) ||
        (!Doxygen::write(*this)) ||
        (!License::write(*this)) ||
        (!Test::write(*this)) ||
        (!writeExtraFiles())){
        return false;
    }

    for (auto& p : m_plugins) {
        if (!p->write()) {
            return false;
        }
    }

    return true;
}

bool Generator::createDir(const boost::filesystem::path& path)
{
    auto iter = m_createdDirs.find(path);
    if (iter != m_createdDirs.end()) {
        return true;
    }

    boost::system::error_code ec;
    bf::create_directories(path, ec);
    if (ec) {
        m_logger.error("Failed to create directory \"" + path.string() + "\" with reason: " + ec.message());
        return false;
    }

    m_createdDirs.insert(path);
    return true;
}

boost::filesystem::path Generator::getProtocolDefRootDir() const
{
    return m_pathPrefix / common::includeStr() / m_mainNamespace;
}

bool Generator::mustDefineDefaultInterface() const
{
    return
        std::none_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& n)
            {
                return n->hasInterfaceDefined();
            });
}

bool Generator::anyInterfaceHasVersion()
{
    return
        std::any_of(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& n)
            {
                return n->anyInterfaceHasVersion();
            });
}

const Field* Generator::findMessageIdField() const
{
    for (auto& n : m_namespaces) {
        auto ptr = n->findMessageIdField();
        if (ptr != nullptr) {
            return ptr;
        }
    }
    return nullptr;
}

bool Generator::writeExtraFiles()
{
    if (m_codeInputDirs.empty()) {
        return true;
    }

    for (auto& d : m_codeInputDirs) {
        auto outputDir = m_pathPrefix;
        auto dirStr = d.string();
        auto pos = dirStr.size();
        auto endIter = bf::recursive_directory_iterator();
        for (auto iter = bf::recursive_directory_iterator(d); iter != endIter; ++iter) {
            if (!bf::is_regular_file(iter->status())) {
                continue;
            }

            auto srcPath = iter->path();
            auto ext = srcPath.extension().string();
            auto extIter = std::find(std::begin(ReservedExt), std::end(ReservedExt), ext);
            if (extIter != std::end(ReservedExt)) {
                continue;
            }

            auto pathStr = srcPath.string();
            auto posTmp = pos;
            while (posTmp < pathStr.size()) {
                if (pathStr[posTmp] == bf::path::preferred_separator) {
                    ++posTmp;
                    continue;
                }
                break;
            }

            if (pathStr.size() <= posTmp) {
                continue;
            }

            std::string relPath(pathStr, posTmp);
            auto destPath = outputDir / relPath;

            m_logger.info("Copying " + destPath.string());

            if (!createDir(destPath.parent_path())) {
                return false;
            }

            boost::system::error_code ec;
            bf::copy_file(srcPath, destPath, bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.error("Failed to copy with reason: " + ec.message());
                return false;
            }
        }
    }
    return true;
}

Namespace& Generator::findOrCreateDefaultNamespace()
{
    auto iter =
        std::find_if(
            m_namespaces.begin(), m_namespaces.end(),
            [](auto& n)
            {
                return n->name().empty();
            });

    if (iter != m_namespaces.end()) {
        return **iter;
    }

    auto nsPtr = createNamespace(*this, commsdsl::Namespace(nullptr));
    bool prepared = nsPtr->prepare();
    if (!prepared) {
        assert(!"Should not happen");
    }

    m_namespaces.insert(m_namespaces.begin(), std::move(nsPtr));
    return *(m_namespaces.front());
}

std::string Generator::headerfileForElement(
    const std::string& externalRef,
    bool quotes,
    const std::string& subNs,
    bool plugin)
{
    std::vector<std::string> subNsList;
    if (!subNs.empty()) {
        subNsList.push_back(subNs);
    }
    return headerfileForElement(externalRef, quotes, subNsList, plugin);
}

std::string Generator::headerfileForElement(
    const std::string& externalRef,
    bool quotes,
    const std::vector<std::string>& subNs,
    bool plugin)
{
    std::string result;
    if (quotes) {
        result += '\"';
    }

    if (plugin) {
        result += common::pluginNsStr();
    }
    else {
        result += m_mainNamespace;
    }

    result += '/';
    auto ns = refToNs(externalRef);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += '/';
        }
    }

    for (auto& n : subNs) {
        result += n + '/';
    }

    auto className = common::nameToClassCopy(refToName(externalRef));
    result += className;
    result += common::headerSuffix();
    if (quotes) {
        result += '\"';
    }
    return result;
}

std::string Generator::pluginCommonSources() const
{
    common::StringsList result;
    for (auto& n : m_namespaces) {
        auto subResult = n->pluginCommonSources();
        result.reserve(result.size() + subResult.size());
        result.insert(result.end(), subResult.begin(), subResult.end());
    }

    return common::listToString(result, "\n", common::emptyString());
}

Generator::PluginsAccessList Generator::getPlugins() const
{
    PluginsAccessList result;
    result.reserve(m_plugins.size());
    for (auto& p : m_plugins) {
        result.push_back(p.get());
    }
    return result;
}

const Interface* Generator::getDefaultInterface() const
{
    auto list = getAllInterfaces();

    if (list.empty()) {
        assert(!"Should not happen");
        return nullptr;
    }

    if (1U < list.size()) {
        return nullptr; // More than 1
    }

    return list.front();
}

std::string Generator::getCustomReadForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ReadSuffix, common::fieldStr());
}

std::string Generator::getCustomWriteForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, WriteSuffix, common::fieldStr());
}

std::string Generator::getCustomLengthForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, LengthSuffix, common::fieldStr());
}

std::string Generator::getCustomValidForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ValidSuffix, common::fieldStr());
}

std::string Generator::getCustomRefreshForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, RefreshSuffix, common::fieldStr());
}

std::string Generator::getCustomNameForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, NameSuffix, common::fieldStr());
}

std::string Generator::getExtraPublicForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, PublicSuffix, common::fieldStr());
}

std::string Generator::getExtraProtectedForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ProtectedSuffix, common::fieldStr());
}

std::string Generator::getExtraPrivateForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, PrivateSuffix, common::fieldStr());
}

std::string Generator::getExtraIncludeForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, IncSuffix, common::fieldStr());
}

std::string Generator::getExtraAppendForField(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::fieldStr());
}

std::string Generator::getCustomReadForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ReadSuffix, common::messageStr());
}

std::string Generator::getCustomWriteForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, WriteSuffix, common::messageStr());
}

std::string Generator::getCustomLengthForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, LengthSuffix, common::messageStr());
}

std::string Generator::getCustomValidForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ValidSuffix, common::messageStr());
}

std::string Generator::getCustomRefreshForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, RefreshSuffix, common::messageStr());
}

std::string Generator::getCustomNameForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, NameSuffix, common::messageStr());
}

std::string Generator::getExtraPublicForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, PublicSuffix, common::messageStr());
}

std::string Generator::getExtraProtectedForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, ProtectedSuffix, common::messageStr());
}

std::string Generator::getExtraPrivateForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, PrivateSuffix, common::messageStr());
}

std::string Generator::getExtraIncludeForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, IncSuffix, common::messageStr());
}

std::string Generator::getExtraAppendForMessage(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::messageStr());
}

std::string Generator::getExtraAppendForMessageHeaderInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::messageStr(), true);
}

std::string Generator::getExtraAppendForMessageSrcInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::messageStr(), true, common::srcSuffix());
}

std::string Generator::getExtraPublicForInterface(const std::string& externalRef) const
{
    if (!externalRef.empty()) {
        return getCustomOpForElement(externalRef, PublicSuffix);
    }
    return getCustomOpForElement(common::messageClassStr(), PublicSuffix);
}

std::string Generator::getExtraProtectedForInterface(const std::string& externalRef) const
{
    if (!externalRef.empty()) {
        return getCustomOpForElement(externalRef, ProtectedSuffix);
    }
    return getCustomOpForElement(common::messageClassStr(), ProtectedSuffix);
}

std::string Generator::getExtraPrivateForInterface(const std::string& externalRef) const
{
    if (!externalRef.empty()) {
        return getCustomOpForElement(externalRef, PrivateSuffix);
    }
    return getCustomOpForElement(common::messageClassStr(), PrivateSuffix);
}

std::string Generator::getExtraIncludeForInterface(const std::string& externalRef) const
{
    if (!externalRef.empty()) {
        return getCustomOpForElement(externalRef, IncSuffix);
    }
    return getCustomOpForElement(common::messageClassStr(), IncSuffix);
}

std::string Generator::getExtraAppendForInterface(const std::string& externalRef) const
{
    auto* ref = &common::messageClassStr();
    if (!externalRef.empty()) {
        ref = &externalRef;
    }
    return getCustomOpForElement(*ref, AppendSuffix);
}

std::string Generator::getExtraAppendForInterfaceHeaderInPlugin(const std::string& externalRef) const
{
    auto* ref = &common::messageClassStr();
    if (!externalRef.empty()) {
        ref = &externalRef;
    }
    return getCustomOpForElement(*ref, AppendSuffix, common::emptyString(), true);
}

std::string Generator::getExtraAppendForInterfaceSrcInPlugin(const std::string& externalRef) const
{
    auto* ref = &common::messageClassStr();
    if (!externalRef.empty()) {
        ref = &externalRef;
    }
    return getCustomOpForElement(*ref, AppendSuffix, common::emptyString(), true, common::srcSuffix());
}

std::string Generator::getExtraAppendForFrame(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::frameStr());
}

std::string Generator::getExtraAppendForFrameHeaderInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::frameStr(), true);
}

std::string Generator::getExtraAppendForFrameTransportMessageHeaderInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef + common::transportMessageSuffixStr(), AppendSuffix, common::frameStr(), true);
}

std::string Generator::getExtraAppendForFrameTransportMessageSrcInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef + common::transportMessageSuffixStr(), AppendSuffix, common::frameStr(), true, common::srcSuffix());
}

std::string Generator::getExtraAppendForPluginHeaderInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::pluginStr(), true);
}

std::string Generator::getExtraAppendForPluginSrcInPlugin(const std::string& externalRef) const
{
    return getCustomOpForElement(externalRef, AppendSuffix, common::pluginStr(), true, common::srcSuffix());
}

std::string Generator::getExtraAppendForFile(const std::string& file) const
{
    std::vector<std::string> elems = {file};
    return getExtraAppendForFile(elems);
}

std::string Generator::getExtraAppendForProtocolDefFile(const std::string& file) const
{
    if (file.empty()) {
        return common::emptyString();
    }

    std::vector<std::string> elems = {
        common::includeStr(),
        mainNamespace(),
        file
    };

    return getExtraAppendForFile(elems);
}

std::string Generator::getExtraAppendForFile(const std::vector<std::string>& elems) const
{
    if (elems.empty()) {
        return common::emptyString();
    }

    if (m_codeInputDirs.empty()) {
        return common::emptyString();
    }

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto filePath = *iter;
        for (auto& e : elems) {
            filePath /= e;
        }

        filePath += AppendSuffix;

        std::ifstream stream(filePath.string());
        if (!stream) {
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return content;
    }
    return common::emptyString();
}

Generator::NamespacesScopesList Generator::getNonDefaultNamespacesScopes() const
{
    NamespacesScopesList result;
    auto& mainNs = mainNamespace();
    for (auto& n : m_namespaces) {
        auto scopes = n->getNamespacesScopes();
        result.reserve(result.size() + scopes.size());
        for (auto& s : scopes) {
            assert(!s.empty());
            result.push_back(mainNs + "::" + s);
        }
    }
    return result;
}

Generator::InterfacesAccessList Generator::getAllInterfaces() const
{
    InterfacesAccessList result;
    for (auto& n : m_namespaces) {
        auto subResult = n->getAllInterfaces();
        result.insert(result.end(), subResult.begin(), subResult.end());
    }
    return result;
}

Generator::FramesAccessList Generator::getAllFrames() const
{
    FramesAccessList result;
    for (auto& n : m_namespaces) {
        auto nList = n->getAllFrames();
        result.insert(result.end(), nList.begin(), nList.end());
    }
    return result;
}

const std::string& Generator::getMinCommsVersionStr() const
{
    return MinCommsVersionStr;
}

std::pair<std::string, std::string>
Generator::namespacesForElement(
    const std::string& externalRef,
    const std::string& subNs,
    bool plugin) const
{
    auto ns = refToNs(externalRef);
    auto tokens = splitRefPath(ns);

    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n";

    if (plugin) {
        begStr +=
            "\n"
            "namespace " + common::pluginNsStr() + "\n"
            "{\n";
    }

    for (auto& t : tokens) {
        if (t.empty()) {
            continue;
        }

        begStr += "\n"
                  "namespace " + t + "\n"
                  "{\n";
    }

    std::string endStr;
    if (!subNs.empty()) {
        begStr +=
            "\n"
            "namespace " + subNs + "\n"
            "{\n";

        endStr += "} // namespace " + subNs + "\n\n";
    }


    for (auto iter = tokens.rbegin(); iter != tokens.rend(); ++iter) {
        auto& t = *iter;
        if (t.empty()) {
            continue;
        }

        endStr += "} // namespace " + t + "\n\n";
    }

    if (plugin) {
        endStr +=
            "} // namespace " + common::pluginNsStr() + "\n\n";
    }

    endStr += "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::string Generator::scopeForElement(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded,
    const std::string& subNs,
    bool plugin)
{
    std::vector<std::string> subNsList;
    if (!subNs.empty()) {
        subNsList.push_back(subNs);
    }

    return scopeForElement(externalRef, mainIncluded, classIncluded, subNsList, plugin);
}

std::string Generator::scopeForElement(
    const std::string& externalRef,
    bool mainIncluded,
    bool classIncluded,
    const std::vector<std::string>& subNs,
    bool plugin)
{
    std::string result;
    if (mainIncluded) {
        result += m_mainNamespace;
        result += ScopeSep;
    }

    if (plugin) {
        result += common::pluginNsStr();
        result += ScopeSep;
    }

    auto ns = refToNs(externalRef);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += ScopeSep;
        }
    }

    for (auto& n : subNs) {
        result += n;
        result += ScopeSep;
    }

    if (classIncluded) {
        result += common::nameToClassCopy(refToName(externalRef));
    }
    return result;
}

std::pair<std::string, std::string>
Generator::startProtocolWrite(
    const std::string& externalRef,
    const std::string& subNs)
{
    if (externalRef.empty()) {
        assert(!"Should not happen");
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    auto ns = refToNs(externalRef);
    auto relDirPath = bf::path(common::includeStr()) / m_mainNamespace / refToPath(ns);
    if (!subNs.empty()) {
        relDirPath /= subNs;
    }
    auto dirPath = m_pathPrefix / relDirPath;

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    auto className = refToName(externalRef);
    assert(!className.empty());
    common::nameToClass(className);
    auto fileName = className + common::headerSuffix();
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto overwriteFile = *iter / relDirPath / fileName;
        boost::system::error_code ec;
        if (bf::exists(overwriteFile, ec)) {
            m_logger.info("Skipping generation of " + fullPathStr);
            return std::make_pair(common::emptyString(), common::emptyString());
        }

        auto replaceFile = *iter / relDirPath / (fileName + ReplaceSuffix);
        if (bf::exists(replaceFile, ec)) {
            m_logger.info("Replacing " + fullPathStr + " with " + replaceFile.string());
            bf::copy_file(replaceFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write \"" + fullPathStr + "\": " + ec.message());
            }
            return std::make_pair(common::emptyString(), common::emptyString());
        }

        auto extendFile = *iter / relDirPath / (fileName + ExtendSuffix);
        if (bf::exists(extendFile, ec)) {
            bf::copy_file(extendFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write \"" + fullPathStr + "\": " + ec.message());
            }
            className += common::origSuffixStr();
            fileName = className + common::headerSuffix();
            fullPath = dirPath / fileName;
            fullPathStr = fullPath.string();
        }
    }

    m_logger.info("Generating " + fullPathStr);
    return std::make_pair(std::move(fullPathStr), className);
}

std::pair<std::string, std::string> Generator::startPluginWrite(
    const std::string& externalRef,
    bool header,
    const std::string& subNs)
{
    if (externalRef.empty()) {
        assert(!"Should not happen");
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    auto ns = refToNs(externalRef);
    auto relDirPath = bf::path(common::pluginNsStr()) / refToPath(ns);
    if (!relDirPath.empty()) {
        relDirPath /= subNs;
    }

    auto dirPath = m_pathPrefix / relDirPath;

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    std::string extension;
    if (header) {
        extension = common::headerSuffix();
    }
    else {
        extension = common::srcSuffix();
    }

    auto className = refToName(externalRef);
    assert(!className.empty());
    common::nameToClass(className);

    auto fileName = className + extension;
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto overwriteFile = *iter / relDirPath / fileName;
        boost::system::error_code ec;
        if (bf::exists(overwriteFile, ec)) {
            m_logger.info("Skipping generation of " + fullPathStr);
            return std::make_pair(common::emptyString(), common::emptyString());
        }

        auto replaceFile = *iter / relDirPath / (fileName + ReplaceSuffix);
        if (bf::exists(replaceFile, ec)) {
            m_logger.info("Replacing " + fullPathStr + " with " + replaceFile.string());
            bf::copy_file(replaceFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write " + fullPathStr);
                assert(!"Should not happen");
            }
            return std::make_pair(common::emptyString(), common::emptyString());
        }

        auto extendFile = *iter / relDirPath / (fileName + ExtendSuffix);
        if (bf::exists(extendFile, ec)) {
            bf::copy_file(extendFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write \"" + fullPathStr + "\": " + ec.message());
                assert(!"Should not happen");
            }
            className += common::origSuffixStr();
            fileName = className + extension;
            fullPath = dirPath / fileName;
            fullPathStr = fullPath.string();
        }
    }

    m_logger.info("Generating " + fullPathStr);
    return std::make_pair(std::move(fullPathStr), std::move(className));
}

std::string Generator::startGenericWrite(
    const std::string& name,
    const std::string& subFolder)
{
    std::vector<std::string> subFolders;
    if (!subFolder.empty()) {
        subFolders.push_back(subFolder);
    }
    return startGenericWrite(name, subFolders);
}


std::string Generator::startGenericWrite(
    const std::string& name,
    const std::vector<std::string>& subFolders)
{
    if (name.empty()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto ns = refToNs(name);
    bf::path relDirPath;
    for (auto& f : subFolders) {
        relDirPath /= f;
    }

    auto dirPath = m_pathPrefix / relDirPath;

    if (!createDir(dirPath)) {
        return common::emptyString();
    }

    auto fullPath = dirPath / name;
    auto fullPathStr = fullPath.string();

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto overwriteFile = *iter / relDirPath / name;
        boost::system::error_code ec;
        if (bf::exists(overwriteFile, ec)) {
            m_logger.info("Skipping generation of " + fullPathStr);
            return common::emptyString();
        }

        auto replaceFile = *iter / relDirPath / (name + ReplaceSuffix);
        if (bf::exists(replaceFile, ec)) {
            m_logger.info("Replacing " + fullPathStr + " with " + replaceFile.string());
            bf::copy_file(replaceFile, bf::path(fullPathStr), bf::copy_option::overwrite_if_exists, ec);
            if (ec) {
                m_logger.warning("Failed to write \"" + fullPathStr + "\": " + ec.message());
            }
            return common::emptyString();
        }
    }

    m_logger.info("Generating " + fullPathStr);
    return fullPathStr;
}

std::string Generator::getCustomOpForElement(
    const std::string& externalRef,
    const std::string& suffix,
    const std::string& subNs,
    bool plugin, 
    const std::string& ext) const
{
    if (externalRef.empty()) {
        return common::emptyString();
    }

    if (m_codeInputDirs.empty()) {
        return common::emptyString();
    }

    auto ns = refToNs(externalRef);

    bf::path rootDir;
    if (plugin) {
        rootDir = common::pluginNsStr();
    }
    else {
        rootDir = bf::path(common::includeStr()) / m_schemaNamespace;
    }

    auto relDirPath = rootDir / refToPath(ns);
    if (!relDirPath.empty()) {
        relDirPath /= subNs;
    }

    auto className = refToName(externalRef);
    assert(!className.empty());

    for (auto iter = m_codeInputDirs.rbegin(); iter != m_codeInputDirs.rend(); ++iter) {
        auto filePath = *iter / relDirPath / (className + ext + suffix);
        std::ifstream stream(filePath.string());
        if (!stream) {
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return content;
    }
    return common::emptyString();
}

bool Generator::preparePlugins()
{
    enum ElemIdx
    {
        ElemIdx_frame,
        ElemIdx_interface,
        ElemIdx_name,
        ElemIdx_description,
        ElemIdx_numOfValues
    };

    using Elems = std::vector<std::string>;

    auto processElemsFunc =
        [this](const std::string& elemsStr)
        {
            Elems elems;
            ba::split(elems, elemsStr, ba::is_any_of(":"));
            elems.resize(ElemIdx_numOfValues);
            auto iter =
                std::find_if(
                    m_plugins.begin(), m_plugins.end(),
                    [&elems](auto& p)
                    {
                        return elems[ElemIdx_name] == p->name();
                    });
            if (iter != m_plugins.end()) {
                m_logger.error("Plugin with name \"" + elems[ElemIdx_name] + "\" has already been defined.");
                return false;
            }

            auto ptr =
                createPlugin(
                    *this,
                    elems[ElemIdx_frame],
                    elems[ElemIdx_interface],
                    elems[ElemIdx_name],
                    elems[ElemIdx_description]);
            assert(ptr);

            if (!ptr->prepare()) {
                return false;
            }

            m_plugins.push_back(std::move(ptr));
            return true;
        };

    auto pluginOpts = m_options.getPlugins();
    if (pluginOpts.empty()) {
        return processElemsFunc(common::emptyString());
    }

    for (auto& o : pluginOpts) {
        if (!processElemsFunc(o)) {
            return false;
        }
    }

    return true;

}

bool Generator::prepareExternalMessages()
{
    auto extraBundles = m_options.getExtraInputBundles();
    if (extraBundles.empty()) {
        return true;
    }

    auto dslMessages = getAllDslMessages();
    std::sort(
        dslMessages.begin(), dslMessages.end(),
        [](auto m1, auto m2)
        {
            return m1.externalRef() < m2.externalRef();
        });

    auto dslPlatforms = platforms();
    for (auto& p : dslPlatforms) {
        common::nameToClass(p);
    }

    std::sort(
        dslPlatforms.begin(), dslPlatforms.end(),
        [](const std::string& p1, const std::string& p2)
        {
            return p1 < p2;
        });

    for (auto& b : extraBundles) {
        auto reportErrorFunc =
            [this, &b]()
            {
                m_logger.error("Invalid value of \"" + m_options.extraMessagesBundlesParamStr() + "\" parameter (" + b + ").");
            };

        auto colonPos = b.find(':');
        if ((colonPos == std::string::npos) ||
            (colonPos == 0) ||
            (b.size() <= (colonPos + 1))) {
            reportErrorFunc();
            return false;
        }

        ExtraMessagesInfo info;
        info.m_name = b.substr(0, colonPos);
        assert(!info.m_name.empty());
        auto allAlphaNum =
            std::all_of(info.m_name.begin(), info.m_name.end(),
            [](char ch)
            {
               return std::isalnum(ch) != 0;
            });

        if (!allAlphaNum) {
            m_logger.error("Invalid bundle name specified with \"" + m_options.extraMessagesBundlesParamStr() + "\" parameter (" + b + ").");
            return false;
        }


        common::nameToClass(info.m_name);

        auto platIter =
            std::lower_bound(
                dslPlatforms.begin(), dslPlatforms.end(), info.m_name,
                [](const std::string& pName, const std::string& bName)
                {
                    return pName < bName;
                });

        if ((platIter != dslPlatforms.end()) ||
            (info.m_name == "All")) {
            m_logger.error("Bundle name specified with \"" + m_options.extraMessagesBundlesParamStr() + "\" parameter (" + b + ") reuses name of one of the platforms.");
            return false;
        }

        auto filePath = b.substr(colonPos + 1);
        std::ifstream stream(filePath);
        if (!stream) {
            m_logger.error("Invalid file name specified with \"" + m_options.extraMessagesBundlesParamStr() + "\" parameter (" + b + ").");
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        ba::split(info.m_dslNames, content, ba::is_any_of(" \r\n"), ba::token_compress_on);

        // trim spaces and check the validity of names
        for (auto& m : info.m_dslNames) {
            ba::trim(m);
            if (m.empty()) {
                continue;
            }

            auto iter =
                std::lower_bound(
                    dslMessages.begin(), dslMessages.end(), m,
                    [](commsdsl::Message msg, const std::string& mParam)
                    {
                        return msg.externalRef() < mParam;
                    });

            if ((iter == dslMessages.end()) ||
                (iter->externalRef() != m)) {
                m_logger.error("Invalid message name (" + m + ") specified inside \"" + filePath + "\" file.");
                return false;
            }
        }

        info.m_dslNames.erase(
            std::remove_if(
                info.m_dslNames.begin(), info.m_dslNames.end(),
                [](const std::string& n)
                {
                    return n.empty();
                }),
            info.m_dslNames.end());

        if (info.m_dslNames.empty()) {
            m_logger.error("The file bundle \"" + info.m_name + "\" doesn't contain any messages.");
            return false;
        }

        std::sort(info.m_dslNames.begin(), info.m_dslNames.end());
        m_extraMessages.push_back(std::move(info));
    }

    return true;
}

std::string Generator::getOptionsBody(GetOptionsFunc func) const
{
    std::string result;
    for (auto& n : m_namespaces) {
        auto str = (n.get()->*func)();
        if (str.empty()) {
            continue;
        }

        if (!result.empty()) {
            result += '\n';
        }

        result += str;
    }
    return result;    
}

} // namespace commsdsl2comms
