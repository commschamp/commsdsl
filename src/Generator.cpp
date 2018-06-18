#include "Generator.h"

#include <boost/algorithm/string.hpp>

#include "Namespace.h"
#include "FieldBase.h"
#include "DefaultOptions.h"
#include "MsgId.h"
#include "Interface.h"
#include "common.h"
#include "EnumField.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ScopeSep("::");

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

std::string Generator::protocolDefRootDir()
{
    auto dir = getProtocolDefRootDir();
    if (!createDir(dir)) {
        m_logger.error("Failed to create \"" + dir.string() + "\" directory.");
        return common::emptyString();
    }

    return dir.string();
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

std::pair<std::string, std::string> Generator::startInterfaceProtocolWrite(
    const std::string& externalRef)
{
    std::string externalRefCpy(externalRef);

    if (externalRefCpy.empty()) {
        // Default interface
        externalRefCpy = "Message";
    }

    // TODO: check suffix
    std::string suffix;

    auto ns = refToNs(externalRefCpy);
    auto className = refToName(externalRefCpy);
    assert(!className.empty());
    className += suffix;
    common::nameToClass(className);
    auto fileName = className + common::headerSuffix();
    auto dirPath = m_pathPrefix / common::includeStr() / m_mainNamespace / refToPath(ns);
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    m_logger.info("Generating " + fullPathStr);

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    return std::make_pair(std::move(fullPathStr), className);
}

std::pair<std::string, std::string> Generator::startFieldProtocolWrite(
    const std::string& externalRef)
{
    if (externalRef.empty()) {
        assert(!"Should not happen");
        return std::make_pair(common::emptyString(), common::emptyString());
    }


    // TODO: check suffix
    std::string suffix;

    auto ns = refToNs(externalRef);
    auto className = refToName(externalRef);
    assert(!className.empty());
    className += suffix;
    common::nameToClass(className);
    auto fileName = className + common::headerSuffix();
    auto dirPath = m_pathPrefix / common::includeStr() / m_mainNamespace / refToPath(ns) / common::fieldStr();
    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    m_logger.info("Generating " + fullPathStr);

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    return std::make_pair(std::move(fullPathStr), className);
}

std::pair<std::string, std::string> Generator::startDefaultOptionsWrite()
{
    // TODO: check suffix
    std::string suffix;

    auto className = common::defaultOptionsStr() + suffix;
    auto fileName = className + common::headerSuffix();
    auto dirPath = getProtocolDefRootDir();

    auto fullPath = dirPath / fileName;
    auto fullPathStr = fullPath.string();

    m_logger.info("Generating " + fullPathStr);

    if (!createDir(dirPath)) {
        return std::make_pair(common::emptyString(), common::emptyString());
    }

    return std::make_pair(std::move(fullPathStr), className);

}

std::pair<std::string, std::string> Generator::namespacesForMessage(
    const std::string& externalRef) const
{
    auto ns = refToNs(externalRef);
    auto tokens = splitRefPath(ns);

    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n";

    for (auto& t : tokens) {
        if (t.empty()) {
            continue;
        }

        begStr += "\n"
                  "namespace " + t + "\n"
                  "{\n";
    }

    begStr +=
        "\n"
        "namespace " + common::messageStr() + "\n"
        "{\n";

    std::string endStr = "} // namespace " + common::messageStr() + "\n\n";
    for (auto iter = tokens.rbegin(); iter != tokens.rend(); ++iter) {
        auto& t = *iter;
        if (t.empty()) {
            continue;
        }

        endStr += "} // namespace " + t + "\n\n";
    }

    endStr += "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::pair<std::string, std::string> Generator::namespacesForField(
    const std::string& externalRef) const
{
    auto ns = refToNs(externalRef);
    auto tokens = splitRefPath(ns);

    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
        "{\n";

    for (auto& t : tokens) {
        if (t.empty()) {
            continue;
        }

        begStr += "\n"
                  "namespace " + t + "\n"
                  "{\n";
    }

    begStr +=
        "\n"
        "namespace " + common::fieldStr() + "\n"
        "{\n";

    std::string endStr = "} // namespace " + common::fieldStr() + "\n\n";
    for (auto iter = tokens.rbegin(); iter != tokens.rend(); ++iter) {
        auto& t = *iter;
        if (t.empty()) {
            continue;
        }

        endStr += "} // namespace " + t + "\n\n";
    }

    endStr += "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
}

std::pair<std::string, std::string> Generator::namespacesForInterface(
    const std::string& externalRef) const
{
    if (externalRef.empty()) {
        return namespacesForRoot();
    }

    auto ns = refToNs(externalRef);
    auto tokens = splitRefPath(ns);

    std::string begStr =
        "namespace " + m_mainNamespace + "\n"
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

    endStr += "} // namespace " + m_mainNamespace + "\n\n";

    return std::make_pair(std::move(begStr), std::move(endStr));
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

std::string Generator::headerfileForMessage(const std::string& externalRef)
{
    std::string result = "\"" + m_mainNamespace + '/';
    auto ns = refToNs(externalRef);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += '/';
        }
    }

    result += common::messageStr() + '/';

    auto className = common::nameToClassCopy(refToName(externalRef));
    result += className;
    result += common::headerSuffix();
    result += '\"';
    return result;
}

std::string Generator::headerfileForField(const std::string& externalRef, bool quotes)
{
    std::string result;
    if (quotes) {
        result += '\"';
    }

    result += m_mainNamespace + '/';
    auto ns = refToNs(externalRef);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += '/';
        }
    }

    result += common::fieldStr() + '/';

    auto className = common::nameToClassCopy(refToName(externalRef));
    result += className;
    result += common::headerSuffix();

    if (quotes) {
        result += '\"';
    }

    return result;
}

std::string Generator::headerfileForInterface(const std::string& externalRef)
{
    std::string externalRefCpy(externalRef);
    if (externalRefCpy.empty()) {
        externalRefCpy = "Message";
    }

    std::string result = "\"" + m_mainNamespace + '/';
    auto ns = refToNs(externalRefCpy);
    if (!ns.empty()) {
        auto tokens = splitRefPath(ns);
        for (auto& t : tokens) {
            result += t;
            result += '/';
        }
    }

    auto className = common::nameToClassCopy(refToName(externalRefCpy));
    result += className;
    result += common::headerSuffix();
    result += '\"';
    return result;
}


std::string Generator::scopeForMessage(
    const std::string& externalRef,
    bool mainIncluded,
    bool messageIncluded)
{
    std::string result;
    if (mainIncluded) {
        result += m_mainNamespace;
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

    result += common::messageStr();
    result += ScopeSep;
    if (messageIncluded) {
        result += common::nameToClassCopy(refToName(externalRef));
    }
    return result;
}

std::string Generator::scopeForField(
    const std::string& externalRef,
    bool mainIncluded,
    bool fieldIncluded)
{
    std::string result;
    if (mainIncluded) {
        result += m_mainNamespace;
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

    result += common::fieldStr();
    result += ScopeSep;
    if (fieldIncluded) {
        result += common::nameToClassCopy(refToName(externalRef));
    }

    return result;
}

std::string Generator::scopeForNamespace(const std::string& externalRef)
{
    std::string result = mainNamespace() + ScopeSep;
    if (!externalRef.empty()) {
        result += ba::replace_all_copy(externalRef, ".", "::");
        result += ScopeSep;
    }
    return result;
}

std::string Generator::getDefaultOptionsBody() const
{
    std::string result;
    for (auto& n : m_namespaces) {
        auto str = n->getDefaultOptions();
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

const Field* Generator::findField(const std::string& externalRef)
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
    auto result = (*nsIter)->findField(remStr);
    if (result == nullptr) {
        m_logger.error("Internal error: unknown external reference: " + externalRef);
        assert(!"Should not happen");
    }
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
        m_logger.error("Warning treated as error");
        return false;
    }

    auto schema = m_protocol.schema();
    if (m_mainNamespace.empty()) {
        assert(!schema.name().empty());
        m_mainNamespace = common::adjustName(schema.name());
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

    m_messageIdField = findMessageIdField();
    return true;
}

bool Generator::writeFiles()
{
    if ((!FieldBase::write(*this)) ||
        (!MsgId::write(*this))) {
        return false;
    }

    if (mustDefineDefaultInterface()) {
        Interface interface(*this, commsdsl::Interface(nullptr));
        if (!interface.write()) {
            return false;
        }
    }

    for (auto& ns : m_namespaces) {
        if ((!ns->writeInterfaces()) ||
            (!ns->writeMessages())) {
            return false;
        }

        // TODO: write others
    }

    for (auto& ns : m_namespaces) {
        if (!ns->writeFields()) {
            return false;
        }
    }

    if (!DefaultOptions::write(*this)) {
        return false;
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


} // namespace commsdsl2comms
