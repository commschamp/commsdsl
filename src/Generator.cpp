#include "Generator.h"

#include <boost/algorithm/string.hpp>

#include "Namespace.h"
#include "common.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

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
    static_cast<void>(sinceVersion);
    static_cast<void>(deprecatedSince);
    static_cast<void>(deprecatedRemoved);
    // TODO
    return true;
}

std::pair<std::string, std::string> Generator::startMessageProtocolWrite(
    const std::string& externalRef,
    const std::vector<std::string>& platforms)
{
    // TODO: check parameters
    static_cast<void>(externalRef);
    static_cast<void>(platforms);

    // TODO: check replacement

    // TODO: check suffix
    std::string suffix;

    auto ns = refToNs(externalRef);
    auto className = refToName(externalRef);
    assert(!className.empty());
    className += suffix;
    common::nameToClass(className);
    auto fileName = className + common::headerSuffix();
    auto dirPath = m_pathPrefix / common::includeStr() / m_mainNamespace / refToPath(ns) / common::messageStr();
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    m_logger.info("Generating " + fullPathStr);

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    return std::make_pair(std::move(fullPathStr), className);
}

std::pair<std::string, std::string> Generator::namespacesForMessage(
    const std::string& externalRef)
{
    auto ns = refToNs(externalRef);
    auto tokens = splitRefPath(ns);

    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n\n"
        "namespace " + common::messageStr() + "\n"
        "{\n";

    for (auto& t : tokens) {
        if (t.empty()) {
            continue;
        }

        begStr += "\n"
                  "namespace " + t + "\n"
                  "{\n";
    }

    std::string endStr;
    for (auto iter = tokens.rbegin(); iter != tokens.rend(); ++iter) {
        auto& t = *iter;
        if (t.empty()) {
            continue;
        }

        endStr += "} // namespace " + t + "\n\n";
    }

    endStr +=
        "} // namespace " + common::messageStr() + "\n\n"
        "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::string Generator::headerfileForMessage(const std::string& externalRef)
{
    std::string result = "\"" + m_mainNamespace + '/' + common::messageStr() + '/';
    auto ns = refToNs(externalRef);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += '/';
        }
    }

    auto className = refToName(externalRef);
    result += className;
    result += common::headerSuffix();
    result += '\"';
    return result;
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

    m_mainNamespace = common::adjustName(m_options.getNamespace());
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
        m_logger.log(commsdsl::ErrorLevel_Error, "Warning treated as error");
        return false;
    }

    auto schema = m_protocol.schema();
    if (m_mainNamespace.empty()) {
        assert(!schema.name().empty());
        m_mainNamespace = common::adjustName(schema.name());
    }

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
    return true;
}

bool Generator::writeFiles()
{
    for (auto& ns : m_namespaces) {
        if (!ns->writeMessages()) {
            return false;
        }

        // TODO: write
    }

    // TODO: write fields
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

} // namespace commsdsl2comms
