#pragma once

#include <vector>
#include <string>
#include <set>

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

    std::pair<std::string, std::string>
    startMessageProtocolWrite(
        const std::string& externalRef,
        const std::vector<std::string>& platforms);

    std::pair<std::string, std::string>
    namespacesForMessage(const std::string& externalRef);

    std::string headerfileForMessage(const std::string& externalRef);

    const std::string& mainNamespace() const
    {
        return m_mainNamespace;
    }

private:

    using NamespacesList = Namespace::NamespacesList;

    bool parseOptions();
    bool parseSchemaFiles(const FilesList& files);
    bool prepare();
    bool writeFiles();
    bool createDir(const boost::filesystem::path& path);

    ProgramOptions& m_options;
    Logger& m_logger;
    commsdsl::Protocol m_protocol;
    NamespacesList m_namespaces;
    boost::filesystem::path m_pathPrefix;
    std::set<boost::filesystem::path> m_createdDirs;
    std::string m_mainNamespace;
};

} // namespace commsdsl2comms
