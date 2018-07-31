#include "Plugin.h"

#include <cassert>
#include <fstream>

#include "Generator.h"

namespace commsdsl2comms
{

namespace
{

const std::string ProtSuffix("Protocol");

} // namespace

bool Plugin::prepare()
{
    m_framePtr = m_generator.findFrame(m_frame);
    if (m_framePtr == nullptr) {
        m_generator.logger().error("Frame \"" + m_frame + "\" hasn't been defined.");
        return false;
    }

    m_interfacePtr = m_generator.findInterface(m_interface);
    if (m_interfacePtr == nullptr) {
        m_generator.logger().error("Interface \"" + m_interface + "\" hasn't been defined.");
        return false;
    }

    return true;
}

bool Plugin::write()
{
    // TODO:
    return
        writeProtocolHeader();
}

bool Plugin::writeProtocolHeader()
{
    auto className = protClassName();
    auto filePath = m_generator.startProtocolPluginHeaderWrite(className);

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#include \"comms_champion/Protocol.h\"\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "class #^#CLASS_NAME#$#Impl;\n"
        "class #^#CLASS_NAME#$# : public comms_champion::Protocol\n"
        "{\n"
        "public:\n"
        "    Protocol();\n"
        "    virtual ~Protocol();\n\n"
        "protected:\n"
        "    virtual const QString& nameImpl() const override;\n"
        "    virtual MessagesList readImpl(const comms_champion::DataInfo& dataInfo, bool final) override;\n"
        "    virtual comms_champion::DataInfoPtr writeImpl(comms_champion::Message& msg) override;\n"
        "    virtual MessagesList createAllMessagesImpl() override;\n"
        "    virtual comms_champion::MessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override;\n"
        "    virtual UpdateStatus updateMessageImpl(comms_champion::Message& msg) override;\n"
        "    virtual comms_champion::MessagePtr cloneMessageImpl(const comms_champion::Message& msg) override;\n"
        "    virtual comms_champion::MessagePtr createInvalidMessageImpl() override;\n"
        "    virtual comms_champion::MessagePtr createRawDataMessageImpl() override;\n"
        "    virtual comms_champion::MessagePtr createExtraInfoMessageImpl() override;\n\n"
        "private:\n"
        "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
    ;

    auto namespaces = m_generator.namespacesForProtocolInPlugin(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Plugin::protClassName() const
{
    auto* nameToUse = &m_name;
    if (nameToUse->empty()) {
        nameToUse = &m_generator.schemaName();
    }

    return common::nameToClassCopy(common::updateNameCopy(*nameToUse)) + ProtSuffix;
}

} // namespace commsdsl2comms

