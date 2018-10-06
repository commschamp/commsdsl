#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <cstdint>

#include <boost/filesystem.hpp>

#include "commsdsl/Protocol.h"
#include "Logger.h"
#include "ProgramOptions.h"
#include "Namespace.h"
#include "Plugin.h"
#include "CustomizationLevel.h"

namespace commsdsl2comms
{

class Generator
{
public:
    using FilesList = std::vector<std::string>;
    using MessageIdMap = std::multimap<std::uintmax_t, std::string>;
    using PluginsAccessList = std::vector<const Plugin*>;
    using NamespacesScopesList = Namespace::NamespacesScopesList;
    using PlatformsList = commsdsl::Protocol::PlatformsList;

    Generator(ProgramOptions& options, Logger& logger)
      : m_options(options), m_logger(logger)
    {
    }

    bool generate(const FilesList& files);

    Logger& logger()
    {
        return m_logger;
    }

    bool doesElementExist(
        unsigned sinceVersion,
        unsigned deprecatedSince,
        bool deprecatedRemoved) const;

    bool isElementOptional(unsigned sinceVersion,
        unsigned deprecatedSince = commsdsl::Protocol::notYetDeprecated(),
        bool deprecatedRemoved = true) const;

    const PlatformsList& platforms() const
    {
        return m_protocol.platforms();
    }

    std::string protocolDefRootDir();

    std::string outputDir();

    std::string pluginDir();

    std::pair<std::string, std::string>
    startMessageProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFrameProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string> startFrameTransportMessageProtocolHeaderWrite(
        const std::string& externalRef);
    std::pair<std::string, std::string> startFrameTransportMessageProtocolSrcWrite(
        const std::string& externalRef);
    std::pair<std::string, std::string> startFrameProtocolHeaderWrite(
        const std::string& externalRef);

    std::pair<std::string, std::string>
    startInterfaceProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFieldProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFieldPluginHeaderWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFieldPluginSrcWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startInterfacePluginHeaderWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startInterfacePluginSrcWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startMessagePluginHeaderWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startMessagePluginSrcWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startProtocolPluginHeaderWrite(const std::string& name);

    std::pair<std::string, std::string>
    startProtocolPluginSrcWrite(const std::string& name);

    std::string startProtocolPluginJsonWrite(const std::string& name);

    std::pair<std::string, std::string>
    startGenericProtocolWrite(const std::string& name);

    std::pair<std::string, std::string>
    startGenericPluginHeaderWrite(const std::string& name);

    std::string startProtocolDocWrite(const std::string& name);

    std::pair<std::string, std::string>
    startGenericPluginSrcWrite(const std::string& name);

    std::pair<std::string, std::string>
    namespacesForMessage(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForMessageInPlugin(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForFrame(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForFrameInPlugin(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForField(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForFieldInPlugin(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForInterface(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForInterfaceInPlugin(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForPluginDef(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForRoot() const;

    std::pair<std::string, std::string>
    namespacesForPlugin() const;

    std::string headerfileForMessage(const std::string& externalRef, bool quotes = true);

    std::string headerfileForMessageInPlugin(const std::string& externalRef, bool quotes = true);

    std::string headerfileForFrame(const std::string& externalRef, bool quotes = true);

    std::string headerfileForFrameInPlugin(const std::string& externalRef, bool quotes = true);

    std::string headerfileForField(const std::string& externalRef, bool quotes = true);

    std::string headerfileForFieldInPlugin(const std::string& externalRef, bool quotes = true);

    std::string headerfileForInterface(const std::string& externalRef);

    std::string headerfileForInterfaceInPlugin(const std::string& externalRef, bool quotes = true);

    std::string headerfileForCustomChecksum(const std::string& name, bool quotes = true);

    std::string headerfileForCustomLayer(const std::string& name, bool quotes = true);

    std::string scopeForMessage(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForMessageInPlugin(
        const std::string& externalRef,
        bool mainIncluded = true,
        bool classIncluded = true);

    std::string scopeForInterface(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForInterfaceInPlugin(const std::string& externalRef);

    std::string scopeForFrame(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool messageIncluded = false);

    std::string scopeForFrameInPlugin(const std::string& externalRef);

    std::string scopeForField(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForFieldInPlugin(const std::string& externalRef);

    std::string scopeForCustomChecksum(
        const std::string& name,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForCustomLayer(
        const std::string& name,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForNamespace(
        const std::string& externalRef,
        bool mainIncluded = true,
        bool appendSep = true);

    const std::string& mainNamespace() const
    {
        return m_mainNamespace;
    }

    commsdsl::Endian schemaEndian() const
    {
        return m_schemaEndian;
    }

    unsigned schemaVersion() const
    {
        return m_schemaVersion;
    }

    const std::string& schemaName() const
    {
        return m_protocol.schema().name();
    }

    bool versionDependentCode() const
    {
        return m_versionDependentCode;
    }

    std::string getDefaultOptionsBody() const;
    std::string getClientDefaultOptionsBody() const;
    std::string getServerDefaultOptionsBody() const;

    std::string getMessageIdStr(const std::string& externalRef, uintmax_t id) const;

    const Field* getMessageIdField() const;

    MessageIdMap getAllMessageIds() const;

    const Field* findField(const std::string& externalRef, bool record = true);
    const Interface* findInterface(const std::string& externalRef);
    const Frame* findFrame(const std::string& externalRef);

    std::string headerfileForElement(
        const std::string& externalRef,
        bool quotes,
        const std::string& subNs = common::emptyString(),
        bool plugin = false);

    std::string headerfileForElement(
        const std::string& externalRef,
        bool quotes,
        const std::vector<std::string>& subNs,
        bool plugin = false);


    commsdsl::Protocol::MessagesList getAllDslMessages() const
    {
        return m_protocol.allMessages();
    }

    std::string commsChampionTag() const
    {
        return m_options.getCommsChampionTag();
    }

    CustomizationLevel customizationLevel() const
    {
        return m_customizationLevel;
    }

    std::string pluginCommonSources() const;

    PluginsAccessList getPlugins() const;

    const Interface* getDefaultInterface() const;

    std::string getCustomReadForField(const std::string& externalRef) const;
    std::string getCustomWriteForField(const std::string& externalRef) const;
    std::string getCustomLengthForField(const std::string& externalRef) const;
    std::string getCustomValidForField(const std::string& externalRef) const;
    std::string getCustomRefreshForField(const std::string& externalRef) const;
    std::string getCustomNameForField(const std::string& externalRef) const;
    std::string getExtraPublicForField(const std::string& externalRef) const;
    std::string getExtraProtectedForField(const std::string& externalRef) const;
    std::string getExtraPrivateForField(const std::string& externalRef) const;
    std::string getExtraIncludeForField(const std::string& externalRef) const;
    std::string getExtraAppendForField(const std::string& externalRef) const;

    std::string getCustomReadForMessage(const std::string& externalRef) const;
    std::string getCustomWriteForMessage(const std::string& externalRef) const;
    std::string getCustomLengthForMessage(const std::string& externalRef) const;
    std::string getCustomValidForMessage(const std::string& externalRef) const;
    std::string getCustomRefreshForMessage(const std::string& externalRef) const;
    std::string getCustomNameForMessage(const std::string& externalRef) const;
    std::string getExtraPublicForMessage(const std::string& externalRef) const;
    std::string getExtraProtectedForMessage(const std::string& externalRef) const;
    std::string getExtraPrivateForMessage(const std::string& externalRef) const;
    std::string getExtraIncludeForMessage(const std::string& externalRef) const;
    std::string getExtraAppendForMessage(const std::string& externalRef) const;
    std::string getExtraAppendForMessageHeaderInPlugin(const std::string& externalRef) const;
    std::string getExtraAppendForMessageSrcInPlugin(const std::string& externalRef) const;

    std::string getExtraPublicForInterface(const std::string& externalRef) const;
    std::string getExtraProtectedForInterface(const std::string& externalRef) const;
    std::string getExtraPrivateForInterface(const std::string& externalRef) const;
    std::string getExtraIncludeForInterface(const std::string& externalRef) const;
    std::string getExtraAppendForInterface(const std::string& externalRef) const;
    std::string getExtraAppendForInterfaceHeaderInPlugin(const std::string& externalRef) const;
    std::string getExtraAppendForInterfaceSrcInPlugin(const std::string& externalRef) const;

    std::string getExtraAppendForFrame(const std::string& externalRef) const;
    std::string getExtraAppendForFrameHeaderInPlugin(const std::string& externalRef) const;
    std::string getExtraAppendForFrameTransportMessageHeaderInPlugin(const std::string& externalRef) const;
    std::string getExtraAppendForFrameTransportMessageSrcInPlugin(const std::string& externalRef) const;

    std::string getExtraAppendForPluginHeaderInPlugin(const std::string& externalRef) const;    
    std::string getExtraAppendForPluginSrcInPlugin(const std::string& externalRef) const;    

    std::string getExtraAppendForFile(const std::string& file) const;
    std::string getExtraAppendForFile(const std::vector<std::string>& elems) const;

    NamespacesScopesList getNonDefaultNamespacesScopes() const;

private:

    using NamespacesList = Namespace::NamespacesList;
    using PluginsList = std::vector<PluginPtr>;
    using InterfacesList = Namespace::InterfacesAccessList;
    using FramesList = Namespace::FramesAccessList;

    bool parseOptions();
    bool parseCustomization();
    bool parseSchemaFiles(const FilesList& files);
    bool prepare();
    bool writeFiles();
    bool createDir(const boost::filesystem::path& path);
    boost::filesystem::path getProtocolDefRootDir() const;
    bool mustDefineDefaultInterface() const;
    bool anyInterfaceHasVersion();
    const Field* findMessageIdField() const;
    bool writeExtraFiles();
    Namespace& findOrCreateDefaultNamespace();

    std::pair<std::string, std::string>
    namespacesForElement(
        const std::string& externalRef,
        const std::string& subNs = common::emptyString(),
        bool plugin = false) const;

    std::string scopeForElement(
        const std::string& externalRef,
        bool mainIncluded,
        bool classIncluded,
        const std::string& subNs = common::emptyString(),
        bool plugin = false);

    std::string scopeForElement(
        const std::string& externalRef,
        bool mainIncluded,
        bool classIncluded,
        const std::vector<std::string>& subNs,
        bool plugin = false);

    std::pair<std::string, std::string> startProtocolWrite(
        const std::string& externalRef,
        const std::string& subNs = common::emptyString());

    std::pair<std::string, std::string> startPluginWrite(
        const std::string& externalRef,
        bool header,
        const std::string& subNs = common::emptyString());

    std::string startGenericWrite(
        const std::string& name,
        const std::string& subFolder = common::emptyString());

    std::string startGenericWrite(
        const std::string& name,
        const std::vector<std::string>& subFolders);

    std::string getCustomOpForElement(
        const std::string& externalRef,
        const std::string& suffix,
        const std::string& subNs = common::emptyString(),
        bool plugin = false,
        const std::string& ext = common::headerSuffix()) const;

    bool preparePlugins();
    InterfacesList getAllInterfaces() const;
    FramesList getAllFrames() const;

    typedef std::string (Namespace::*GetOptionsFunc)() const;
    std::string getOptionsBody(GetOptionsFunc func) const;

    ProgramOptions& m_options;
    Logger& m_logger;
    commsdsl::Protocol m_protocol;
    NamespacesList m_namespaces;
    PluginsList m_plugins;
    boost::filesystem::path m_pathPrefix;
    std::vector<boost::filesystem::path> m_codeInputDirs;
    std::set<boost::filesystem::path> m_createdDirs;
    std::string m_mainNamespace;
    commsdsl::Endian m_schemaEndian = commsdsl::Endian_NumOfValues;
    unsigned m_schemaVersion = 0U;
    unsigned m_minRemoteVersion = 0U;
    CustomizationLevel m_customizationLevel = CustomizationLevel::Limited;
    const Field* m_messageIdField = nullptr;
    bool m_versionDependentCode = false;
};

} // namespace commsdsl2comms
