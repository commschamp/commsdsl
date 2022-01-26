#include "CommsField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;

namespace commsdsl2new
{

namespace 
{

void convertToCommonIncludePath(std::string& filePath)
{
    assert(2 <= filePath.size());
    assert(filePath.back() == 'h');
    filePath.insert(filePath.size() - strings::cppHeaderSuffixStr().size(), strings::commonSuffixStr());
}

} // namespace 
    

CommsField::CommsField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

CommsField::~CommsField() = default;

bool CommsField::commsPrepare()
{
    auto& generator = m_field.generator();
    auto codePathPrefix = comms::inputCodePathFor(m_field, generator);
    m_customRead = util::readFileContents(codePathPrefix + strings::readFileSuffixStr());
    m_customRefresh = util::readFileContents(codePathPrefix + strings::refreshFileSuffixStr());
    m_customWrite = util::readFileContents(codePathPrefix + strings::writeFileSuffixStr());
    return true;
}

bool CommsField::commsWrite() const
{
    auto* parent = m_field.getParent();
    if (parent == nullptr) {
        assert(false); // Should not happen
        return false;
    } 

    auto type = parent->elemType();
    if (type != commsdsl::gen::Elem::Type::Type_Namespace)
    {
        // Skip write for non-global fields,
        // The code generation will be driven by other means
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_referenced)) {
        // Not referenced fields do not need to be written
        return true;
    }

    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

CommsField::IncludesList CommsField::commsCommonIncludes() const
{
    return commsCommonIncludesImpl();
}


std::string CommsField::commsCommonCode() const
{
    static const std::string Templ = 
        "/// @brief Common types and functions for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#NAME#$#Common\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n"
    ;

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, generator)},
        {"NAME", comms::className(m_field.name())},
        {"BODY", commsCommonCodeBodyImpl()},
    };

    return util::processTemplate(Templ, repl);
}

CommsField::IncludesList CommsField::commsDefIncludes() const
{
    auto& generator = m_field.generator();

    IncludesList list = {
        "comms/options.h",
        comms::relHeaderPathForField(strings::fieldBaseClassStr(), generator),
    };

    if (comms::isGlobalField(m_field)) {
        auto relHeader = comms::relHeaderPathFor(m_field, generator);
        convertToCommonIncludePath(relHeader);
        list.push_back(relHeader);
        list.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator));
    }

    if (commsIsVersionOptional()) {
        list.push_back("comms/field/Optional.h");
    }

    auto extraList = commsDefIncludesImpl();
    list.insert(list.end(), extraList.begin(), extraList.end());
    
    return list;
}

std::string CommsField::commsDefCode() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#FIELD#$#\n"
        "#^#OPTIONAL#$#\n"
    ;

    //auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"MEMBERS", commsDefMembersCodeImpl()},
        {"FIELD", commsFieldDefCodeInternal()},
        {"OPTIONAL", commsOptionalDefCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

CommsField::IncludesList CommsField::commsCommonIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsCommonCodeBodyImpl() const
{
    return strings::emptyString();
}

CommsField::IncludesList CommsField::commsDefIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsDefMembersCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDoxigenDetailsImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsExtraDoxigenImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsBaseClassDefImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefPublicCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefProtectedCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefPrivateCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefReadFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefWriteFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefRefreshFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefLengthFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefValidFuncBodyImpl() const
{
    return strings::emptyString();
}

bool CommsField::commsIsLimitedCustomizableImpl() const
{
    return false;
}

std::string CommsField::commsCommonNameFuncCode() const
{
    auto& generator = m_field.generator();
    auto customNamePath = comms::inputCodePathFor(m_field, generator) + strings::nameFileSuffixStr();

    auto customFunc = util::readFileContents(customNamePath);
    if (!customFunc.empty()) {
        return customFunc;
    }

    static const std::string Templ = 
        "/// @brief Name of the @ref #^#SCOPE#$# field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"#^#NAME#$#\";\n"
        "}\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, generator)},
        {"NAME", util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name())},
    };

    return util::processTemplate(Templ, repl);
}

bool CommsField::commsIsVersionOptional() const
{
    auto& generator = m_field.generator();
    if (!generator.versionDependentCode()) {
        return false;
    }

    auto& dslObj = m_field.dslObj();
    if (!generator.isElementOptional(dslObj.sinceVersion(), dslObj.deprecatedSince(), dslObj.isDeprecatedRemoved())) {
        return false;
    }

    auto* parent = m_field.getParent();
    assert(parent != nullptr);
    if (comms::sinceVersionOf(*parent) < dslObj.sinceVersion()) {
        return true;
    }
    
    if ((dslObj.deprecatedSince() < commsdsl::parse::Protocol::notYetDeprecated()) &&
        (dslObj.isDeprecatedRemoved())) {
        return true;
    }

    return false;    
}

std::string CommsField::commsFieldBaseParams(commsdsl::parse::Endian endian) const
{
    auto& generator = m_field.generator();
    auto schemaEndian = generator.schemaEndian();
    assert(endian < commsdsl::parse::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::parse::Endian_NumOfValues);

    if ((schemaEndian == endian) ||
        (commsdsl::parse::Endian_NumOfValues <= endian)) {
        return strings::emptyString();
    }

    return comms::dslEndianToOpt(endian);
}

void CommsField::commsAddFieldDefOptions(commsdsl::gen::util::StringsList& opts) const
{
    if (comms::isGlobalField(m_field)) {
        opts.push_back("TExtraOpts...");
    }

    if (commsIsFieldCustomizable()) {
        opts.push_back("typename TOpt::" + comms::scopeFor(m_field, m_field.generator(), false, true));
    }

    if (m_forcedFailOnInvalid) {
        opts.push_back("comms::option::def::FailOnInvalid<comms::ErrorStatus::ProtocolError>");
    }
    else if (m_field.dslObj().isFailOnInvalid()) {
        util::addToStrList("comms::option::def::FailOnInvalid<>", opts);
    }

    if (!m_customRead.empty()) {
        util::addToStrList("comms::option::def::HasCustomRead", opts);
    }

    if (!m_customRefresh.empty()) {
        util::addToStrList("comms::option::def::HasCustomRefresh", opts);
    }

    if (!m_customWrite.empty()) {
        util::addToStrList("comms::option::def::HasCustomWrite", opts);
    }    

    if (m_forcedPseudo || m_field.dslObj().isPseudo()) {
        util::addToStrList("comms::option::def::EmptySerialization", opts);
    }
}

bool CommsField::commsIsFieldCustomizable() const
{
    auto& generator = static_cast<CommsGenerator&>(m_field.generator());
    auto level = generator.getCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }

    if (m_field.dslObj().isCustomizable()) {
        return true;
    }

    if (level == CommsGenerator::CustomizationLevel::None) {
        return false;
    }

    return commsIsLimitedCustomizableImpl();
}

bool CommsField::commsWriteCommonInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = commsdsl::gen::comms::headerPathFor(m_field, generator);
    convertToCommonIncludePath(filePath);

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }

    auto includes = commsCommonIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    @ref #^#FIELD_SCOPE#$# field.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_SCOPE", comms::scopeFor(m_field, generator)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", commsCommonCode()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

bool CommsField::commsWriteDefInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = commsdsl::gen::comms::headerPathFor(m_field, generator);

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto includes = commsDefIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#FIELD_NAME#$#\"</b> field.\n"
        "\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#EXTRA_INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_NAME", util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name())},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"EXTRA_INCLUDES", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::incFileSuffixStr())},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", commsDefCode()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

std::string CommsField::commsFieldDefCodeInternal() const
{
    static const std::string Templ = 
        "#^#BRIEF#$#\n"
        "#^#DETAILS#$#\n"
        "#^#EXTRA_DOC#$#\n"
        "#^#DEPRECATED#$#\n"
        "#^#PARAMS#$#\n"
        "class #^#NAME#$##^#SUFFIX#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n"
        "#^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n"
    ;

    //auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"BRIEF", commsFieldBriefInternal()},
        {"DETAILS", commsDocDetailsInternal()},
        {"EXTRA_DOC", commsExtraDocInternal()},
        {"DEPRECATED", commsDeprecatedDocInternal()},
        {"PARAMS", commsTemplateParamsInternal()},
        {"NAME", comms::className(m_field.name())},
        {"BASE", commsBaseClassDefImpl()},
        {"PUBLIC", commsDefPublicCodeInternal()},
        {"PROTECTED", commsDefProtectedCodeInternal()},
        {"PRIVATE", commsDefPrivateCodeInternal()},
    };

    if (commsIsVersionOptional()) {
        repl.insert({{"SUFFIX", strings::versionOptionalFieldSuffixStr()}});
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsOptionalDefCodeInternal() const
{
    if (!commsIsVersionOptional()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "/// @brief Definition of version dependent\n"
        "///     <b>#^#NAME#$#</b> field."
        "#^#PARAMS#$#\n"
        "struct #^#CLASS_NAME#$# : public\n"
        "    comms::field::Optional<\n"
        "        #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#,\n"
        "        comms::option::def::#^#DEFAULT_MODE_OPT#$#,\n"
        "        comms::option::def::#^#VERSIONS_OPT#$#\n"
        "    >\n"
        "{\n"
        "    /// @brief Name of the field.\n"
        "    static const char* name()\n"
        "    {\n"
        "        return #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#::name();\n"
        "    }\n"
        "};\n"; 


        auto& generator = m_field.generator();
        auto& dslObj = m_field.dslObj();
        bool fieldExists = 
            generator.doesElementExist(
                dslObj.sinceVersion(),
                dslObj.deprecatedSince(),
                dslObj.isDeprecatedRemoved());

        std::string defaultModeOpt = "ExistsByDefault";
        if (!fieldExists) {
            defaultModeOpt = "MissingByDefault";
        }

        std::string versionOpt = "ExistsSinceVersion<" + util::numToString(dslObj.sinceVersion()) + '>';
        if (dslObj.isDeprecatedRemoved()) {
            assert(dslObj.deprecatedSince() < commsdsl::parse::Protocol::notYetDeprecated());
            if (dslObj.sinceVersion() == 0U) {
                versionOpt = "ExistsUntilVersion<" + util::numToString(dslObj.deprecatedSince() - 1) + '>';
            }
            else {
                versionOpt =
                    "ExistsBetweenVersions<" +
                    util::numToString(dslObj.sinceVersion()) +
                    ", " +
                    util::numToString(dslObj.deprecatedSince() - 1) +
                    '>';
            }
        }

    util::ReplacementMap repl = {
        {"NAME", util::displayName(dslObj.displayName(), dslObj.name())},
        {"PARAMS", commsTemplateParamsInternal()},
        {"CLASS_NAME", comms::className(dslObj.name())},
        {"DEFAULT_MODE_OPT", std::move(defaultModeOpt)},
        {"VERSIONS_OPT", std::move(versionOpt)},
    };

    if (comms::isGlobalField(m_field)) {
        repl.insert({{"FIELD_PARAMS", "<TOpt, TExtraOpts...>"}});
    }

    return util::processTemplate(Templ, repl); 
}

std::string CommsField::commsFieldBriefInternal() const
{
    if (commsIsVersionOptional()) {
        return "/// @brief Inner field of @ref " + comms::className(m_field.name()) + " optional.";
    }

    return
        "/// @brief Definition of <b>\"" +
        util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name()) +
        "\"</b> field.";
}

std::string CommsField::commsDocDetailsInternal() const
{
    std::string result;
    do {
        auto& desc = m_field.dslObj().description();       
        auto extraDetails = commsDoxigenDetailsImpl();
        if (desc.empty() && extraDetails.empty()) {
            break;
        }

        result += "/// @details\n";

        if (!desc.empty()) {
            auto multiDesc = util::strMakeMultiline(desc);
            multiDesc = util::strInsertIndent(multiDesc);
            result += strings::doxygenPrefixStr() + util::strReplace(multiDesc, "\n", "\n" + strings::doxygenPrefixStr());
        }

        if (extraDetails.empty()) {
            break;
        }   

        result += '\n';      

        if (!desc.empty()) {
            result += strings::doxygenPrefixStr();
            result += '\n';
        }      

        auto multiExtra = util::strMakeMultiline(extraDetails);
        multiExtra = util::strInsertIndent(multiExtra);
        result += strings::doxygenPrefixStr() + util::strReplace(multiExtra, "\n", "\n" + strings::doxygenPrefixStr());
    } while (false);
    return result;
}

std::string CommsField::commsExtraDocInternal() const
{
    std::string result;
    auto doc = commsExtraDoxigenImpl();
    if (!doc.empty()) {
        result += strings::doxygenPrefixStr() + util::strReplace(doc, "\n", "\n" + strings::doxygenPrefixStr());
    }
    return result;
}

std::string CommsField::commsDeprecatedDocInternal() const
{
    std::string result;
    auto deprecatedVersion = m_field.dslObj().deprecatedSince();
    auto& generator = m_field.generator();
    if (generator.isElementDeprecated(deprecatedVersion)) {
        result += "/// @deprecated Since version " + std::to_string(deprecatedVersion) + '\n';
    }

    return result;
}

std::string CommsField::commsTemplateParamsInternal() const
{
    std::string result;
    if (comms::isGlobalField(m_field)) {
        result += "/// @tparam TOpt Protocol options.\n";
        result += "/// @tparam TExtraOpts Extra options.\n";
        result += "template <typename TOpt = ";
        result += comms::scopeForOptions(strings::defaultOptionsStr(), m_field.generator());
        result += ", typename... TExtraOpts>";
    }

    return result;    
}

std::string CommsField::commsDefPublicCodeInternal() const
{
    static const std::string Templ = {
        "public:\n"
        "    #^#FIELD_DEF#$#\n"
        "    #^#NAME#$#\n"
        "    #^#READ#$#\n"
        "    #^#WRITE#$#\n"
        "    #^#REFRESH#$#\n"
        "    #^#LENGTH#$#\n"
        "    #^#VALID#$#\n"
        "    #^#EXTRA_PUBLIC#$#\n"
    };

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"FIELD_DEF", commsDefPublicCodeImpl()},
        {"NAME", commsDefNameFuncCodeInternal()},
        {"READ", commsDefReadFuncCodeInternal()},
        {"WRITE", commsDefWriteFuncCodeInternal()},
        {"REFRESH", commsDefRefreshFuncCodeInternal()},
        {"LENGTH", commsDefLengthFuncCodeInternal()},
        {"VALID", commsDefValidFuncCodeInternal()},
        {"EXTRA_PUBLIC", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::publicFileSuffixStr())},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefProtectedCodeInternal() const
{
    static const std::string Templ = 
        "protected:\n"
        "    #^#FIELD#$#\n"
        "    #^#CUSTOM#$#\n";

    util::ReplacementMap repl;

    auto field = commsDefProtectedCodeImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::protectedFileSuffixStr());
    
    if (!field.empty()) {
        repl.insert({{"FIELD", std::move(field)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefPrivateCodeInternal() const
{
    static const std::string Templ = 
        "private:\n"
        "    #^#FIELD#$#\n"
        "    #^#CUSTOM#$#\n";

    util::ReplacementMap repl;

    auto field = commsDefPrivateCodeImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::privateFileSuffixStr());
    
    if (!field.empty()) {
        repl.insert({{"FIELD", std::move(field)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefNameFuncCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Name of the field.\n"
        "static const char* name()\n"
        "{\n"
        "    return #^#SCOPE#$#Common::name();\n"
        "}\n";

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, generator)},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefReadFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    auto body = commsDefReadFuncBodyImpl();
    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated read functionality.\n"
            "template <typename TIter>\n"
            "comms::ErrorStatus read#^#SUFFIX#$#(TIter& iter, std::size_t len)\n"
            "{\n"
            "    #^#BODY#$#"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customRead.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customRead.empty()) {
        repl.insert({{"CUSTOM", m_customRead}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefWriteFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    auto body = commsDefWriteFuncBodyImpl();
    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated write functionality.\n"
            "template <typename TIter>\n"
            "comms::ErrorStatus write#^#SUFFIX#$#(TIter& iter, std::size_t len) const\n"
            "{\n"
            "    #^#BODY#$#"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customWrite.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customWrite.empty()) {
        repl.insert({{"CUSTOM", m_customWrite}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefRefreshFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    auto body = commsDefRefreshFuncBodyImpl();
    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated refresh functionality.\n"
            "template <typename TIter>\n"
            "bool refresh#^#SUFFIX#$#() const\n"
            "{\n"
            "    #^#BODY#$#"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customRefresh.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customRefresh.empty()) {
        repl.insert({{"CUSTOM", m_customRefresh}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefLengthFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    auto body = commsDefLengthFuncBodyImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::lengthFileSuffixStr());
    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated length functionality.\n"
            "template <typename TIter>\n"
            "std::size_t length#^#SUFFIX#$#() const\n"
            "{\n"
            "    #^#BODY#$#"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!custom.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefValidFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    auto body = commsDefValidFuncBodyImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::validFileSuffixStr());
    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated validity check functionality.\n"
            "template <typename TIter>\n"
            "bool valid#^#SUFFIX#$#() const\n"
            "{\n"
            "    #^#BODY#$#"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!custom.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefMembersCodeInternal() const
{
    auto body = commsDefMembersCodeImpl();
    if (body.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for all the member fields of\n"
        "///     @ref #^#CLASS_NAME#$# field.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n";    

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(m_field.name())},
        {"BODY", std::move(body)}
    };

    if (comms::isGlobalField(m_field)) {
        auto prefix = 
            "/// @tparam TOpt Protocol options.\n"
            "template <typename TOpt = " + comms::scopeForOptions(strings::defaultOptionsStr(), m_field.generator()) + ">";
        
        repl.insert({{"EXTRA_PREFIX", std::move(prefix)}});
    }

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2new
