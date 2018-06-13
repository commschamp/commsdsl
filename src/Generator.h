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

    bool isElementOptional(
        unsigned sinceVersion,
        unsigned deprecatedSince = commsdsl::Protocol::notYetDeprecated()) const;

    std::string protocolDefRootDir();

    std::pair<std::string, std::string>
    startMessageProtocolWrite(
        const std::string& externalRef,
        const std::vector<std::string>& platforms);

    std::pair<std::string, std::string>
    startInterfaceProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startFieldProtocolWrite(const std::string& externalRef);

    std::pair<std::string, std::string>
    startDefaultOptionsWrite();

    std::pair<std::string, std::string>
    namespacesForMessage(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForField(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForInterface(const std::string& externalRef) const;

    std::pair<std::string, std::string>
    namespacesForRoot() const;

    std::string headerfileForMessage(const std::string& externalRef);

    std::string headerfileForField(const std::string& externalRef, bool quotes = true);

    std::string headerfileForInterface(const std::string& externalRef);

    std::string scopeForMessage(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool messageIncluded = false);

    std::string scopeForField(
        const std::string& externalRef,
        bool mainIncluded = false,
        bool fieldIncluded = false);

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

    std::string getDefaultOptionsBody() const;

    std::string getMessageIdStr(const std::string& externalRef, uintmax_t id) const;

    const Field* findMessageIdField() const;

    MessageIdMap getAllMessageIds() const;

    const Field* findField(const std::string& externalRef);
private:

    using NamespacesList = Namespace::NamespacesList;

    bool parseOptions();
    bool parseSchemaFiles(const FilesList& files);
    bool prepare();
    bool writeFiles();
    bool createDir(const boost::filesystem::path& path);
    boost::filesystem::path getProtocolDefRootDir() const;
    bool mustDefineDefaultInterface() const;

    ProgramOptions& m_options;
    Logger& m_logger;
    commsdsl::Protocol m_protocol;
    NamespacesList m_namespaces;
    boost::filesystem::path m_pathPrefix;
    std::set<boost::filesystem::path> m_createdDirs;
    std::string m_mainNamespace;
    commsdsl::Endian m_schemaEndian = commsdsl::Endian_NumOfValues;
    unsigned m_schemaVersion = 0U;
    MessageIdMap m_messageIds;
};

} // namespace commsdsl2comms
