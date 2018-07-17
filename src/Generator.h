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

namespace commsdsl2comms
{

class Generator
{
public:
    using FilesList = std::vector<std::string>;
    using MessageIdMap = std::multimap<std::uintmax_t, std::string>;

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

    std::string protocolDefRootDir();

    bool isAnyPlatformSupported(
        const std::vector<std::string>& platforms);

    std::pair<std::string, std::string>
    startMessageProtocolWrite(
        const std::string& externalRef,
        const std::vector<std::string>& platforms);

    std::pair<std::string, std::string>
    startFrameProtocolWrite(
        const std::string& externalRef);

    std::pair<std::string, std::string>
    startInterfaceProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFieldProtocolWrite(const std::string& externalRef);

    std::string startFieldPluginHeaderWrite(const std::string& externalRef);
    std::string startFieldPluginSrcWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startDefaultOptionsWrite();

    std::pair<std::string, std::string>
    namespacesForMessage(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForFrame(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForField(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForFieldInPlugin(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForInterface(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForRoot() const;

    std::string headerfileForMessage(const std::string& externalRef, bool quotes = true);

    std::string headerfileForFrame(const std::string& externalRef, bool quotes = true);

    std::string headerfileForField(const std::string& externalRef, bool quotes = true);

    std::string headerfileForFieldInPlugin(const std::string& externalRef, bool quotes = true);

    std::string headerfileForInterface(const std::string& externalRef);

    std::string headerfileForCustomChecksum(const std::string& name, bool quotes = true);

    std::string headerfileForCustomLayer(const std::string& name, bool quotes = true);

    std::string scopeForMessage(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool classIncluded = false);

    std::string scopeForFrame(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool messageIncluded = false);

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

    std::string scopeForNamespace(const std::string& externalRef);

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

    bool versionDependentCode() const
    {
        return m_versionDependentCode;
    }

    std::string getDefaultOptionsBody() const;

    std::string getMessageIdStr(const std::string& externalRef, uintmax_t id) const;

    const Field* getMessageIdField() const;

    MessageIdMap getAllMessageIds() const;

    const Field* findField(const std::string& externalRef, bool record = true);

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
private:

    using NamespacesList = Namespace::NamespacesList;

    bool parseOptions();
    bool parseSchemaFiles(const FilesList& files);
    bool prepare();
    bool writeFiles();
    bool createDir(const boost::filesystem::path& path);
    boost::filesystem::path getProtocolDefRootDir() const;
    bool mustDefineDefaultInterface() const;
    bool anyInterfaceHasVersion();
    const Field* findMessageIdField() const;
    bool writeExtraFiles();

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

    std::string startPluginWrite(
        const std::string& externalRef,
        bool header,
        const std::string subNs = common::emptyString());

    ProgramOptions& m_options;
    Logger& m_logger;
    commsdsl::Protocol m_protocol;
    NamespacesList m_namespaces;
    boost::filesystem::path m_pathPrefix;
    boost::filesystem::path m_codeInputDir;
    std::set<boost::filesystem::path> m_createdDirs;
    std::string m_mainNamespace;
    commsdsl::Endian m_schemaEndian = commsdsl::Endian_NumOfValues;
    unsigned m_schemaVersion = 0U;
    unsigned m_minRemoteVersion = 0U;
    const Field* m_messageIdField = nullptr;
    bool m_versionDependentCode = false;
};

} // namespace commsdsl2comms
