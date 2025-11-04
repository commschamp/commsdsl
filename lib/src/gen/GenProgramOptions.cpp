#include "commsdsl/gen/GenProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>

namespace util = commsdsl::gen::util;

namespace commsdsl
{

namespace gen
{

namespace
{

const std::string GenHelpOptStr("help");
const std::string GenFullHelpOptStr("h," + GenHelpOptStr);
const std::string GenVersionOptStr("version");
const std::string GenQuietStr("quiet");
const std::string GenFullQuietStr("q," + GenQuietStr);
const std::string GenDebugStr("debug");
const std::string GenFullDebugStr("d," + GenDebugStr);
const std::string GenWarnAsErrStr("warn-as-err");
const std::string GenFullWarnAsErrStr("w," + GenWarnAsErrStr);
const std::string GenOutputDirStr("output-dir");
const std::string GenFullOutputDirStr("o," + GenOutputDirStr);
const std::string GenInputFilesListStr("input-files-list");
const std::string GenFullInputFilesListStr("i," + GenInputFilesListStr);
const std::string GenInputFilesPrefixStr("input-files-prefix");
const std::string GenFullInputFilesPrefixStr("p," + GenInputFilesPrefixStr);
const std::string GenCodeInputDirStr("code-input-dir");
const std::string GenFullCodeInputDirStr("c," + GenCodeInputDirStr);
const std::string GenMultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string GenFullMultipleSchemasEnabledStr("s," + GenMultipleSchemasEnabledStr);
const std::string GenNamespaceStr("namespace");
const std::string GenFullNamespaceStr("n," + GenNamespaceStr);
const std::string GenMinRemoteVerStr("min-remote-version");
const std::string GenFullMinRemoteVerStr("m," + GenMinRemoteVerStr);
const std::string GenForceVerStr("force-schema-version");
const std::string GenCodeVerStr("code-version");
const std::string GenFullCodeVerStr("V," + GenCodeVerStr);

}

class GenProgramOptionsImpl
{
public:
    using GenStringsList = GenProgramOptions::GenStringsList;

    void genAdd(const std::string& optStr, const std::string& desc, bool hasParam)
    {
        genAddInternal(optStr, desc, hasParam);
    }

    void genAdd(const std::string& optStr, const std::string& desc, const std::string& defaultValue)
    {
        genAddInternal(optStr, desc, true, defaultValue);
    }

    void genRemove(const std::string& optStr)
    {
        auto tokens = util::genStrSplitByAnyChar(optStr, ",");
        m_opts.erase(
            std::remove_if(
                m_opts.begin(), m_opts.end(),
                [&tokens](auto& oPtr)
                {
                    return
                        std::any_of(
                            tokens.begin(), tokens.end(),
                            [&oPtr](auto& t)
                            {
                                return (oPtr->m_shortOpt == t) || (oPtr->m_longOpt == t);
                            });
                }),
            m_opts.end());

    }

    void genParse(int argc, const char** argv)
    {
        genPrepareOpts();

        assert(0 < argc);
        m_app = argv[0];
        OptInfo* opt = nullptr;
        for (auto idx = 1; idx < argc; ++idx) {
            const char* nextToken = argv[idx];
            assert(nextToken != nullptr);
            std::string nextStr(nextToken);
            if (nextStr.empty()) {
                assert(false); // Should not happen
                continue;
            }

            if (opt != nullptr) {
                assert(opt->m_hasParam);
                opt->m_value = std::move(nextStr);
                opt = nullptr;
                continue;
            }

            if (nextStr[0] != '-') {
                m_args.push_back(nextStr);
                continue;
            }

            if (nextStr.size() < 2) {
                genReportUnknownOption(nextStr);
                continue;
            }

            auto processOptFunc =
                [this, &opt, &nextStr](const std::string& optStr, OptInfosMap& map)
                {
                    auto iter = map.find(optStr);
                    if (iter == map.end()) {
                        genReportUnknownOption(nextStr);
                        return;
                    }

                    opt = iter->second;
                    opt->m_wasUsed = true;
                    if (!opt->m_hasParam) {
                        opt = nullptr;
                    }
                };

            if (nextStr[1] == '-') {
                auto longOpt = nextStr.substr(2);
                processOptFunc(longOpt, m_longOpts);
                continue;
            }

            auto shortOpt = nextStr.substr(1);
            processOptFunc(shortOpt, m_shortOpts);
        }
    }

    bool genIsOptUsed(const std::string& optStr) const
    {
        if ((optStr.size() == 1) && genIsOptUsedInternal(optStr, m_shortOpts)) {
            return true;
        }

        return genIsOptUsedInternal(optStr, m_longOpts);
    }

    const std::string& genValue(const std::string& optStr) const
    {
        if ((optStr.size() == 1) && (genFindOptInfo(optStr, m_shortOpts) != nullptr)) {
            return genValueInternal(optStr, m_shortOpts);
        }

        return genValueInternal(optStr, m_longOpts);
    }

    const GenStringsList& genArgs() const
    {
        return m_args;
    }

    const std::string& genApp() const
    {
        return m_app;
    }

    std::string genHelpStr() const
    {
        util::GenStringsList opts;
        for (auto& optPtr : m_opts) {
            assert(optPtr);

            std::string left;
            if (!optPtr->m_shortOpt.empty()) {
                left += "-" + optPtr->m_shortOpt + ' ';
            }

            if (!optPtr->m_longOpt.empty()) {
                if (!optPtr->m_shortOpt.empty()) {
                    left += "[";
                }

                left += "--" + optPtr->m_longOpt;
                if (!optPtr->m_shortOpt.empty()) {
                    left += "]";
                }

                left += ' ';
            }

            if (optPtr->m_hasParam) {
                left += "arg ";

                if (!optPtr->m_value.empty()) {
                    left += "(=" + optPtr->m_value + ") ";
                }
            }

            static const unsigned MaxLeftLen = 38;
            static const std::string Ind = '\n' + std::string(38, ' ');
            std::string right;
            if (left.size() <= MaxLeftLen) {
                left.append(std::string(MaxLeftLen - left.size(), ' '));
            }
            else {
                right += Ind;
            }

            right += util::genStrReplace(util::genStrMakeMultiline(optPtr->m_desc, 60), "\n", Ind);
            opts.push_back(left + right);
        }

        static const std::string Templ =
            "Options:\n"
            "  #^#OPTS_LIST#$#\n";

        util::GenReplacementMap repl = {
            std::make_pair("OPTS_LIST", util::genStrListToString(opts, "\n"))
        };

        return util::genProcessTemplate(Templ, repl);
    }

private:
    struct OptInfo
    {
        std::string m_shortOpt;
        std::string m_longOpt;
        std::string m_desc;
        std::string m_value;
        bool m_hasParam = false;
        bool m_wasUsed = false;
    };
    using OptInfoPtr = std::unique_ptr<OptInfo>;
    using OptInfosList = std::vector<OptInfoPtr>;
    using OptInfosMap = std::map<std::string, OptInfo*>;

    void genAddInternal(const std::string& optStr, const std::string& desc, bool hasParam = false, const std::string& defaultValue = std::string())
    {
        auto tokens = util::genStrSplitByAnyChar(optStr, ",");

        auto opt = std::make_unique<OptInfo>();
        opt->m_desc = desc;
        opt->m_hasParam = hasParam;
        opt->m_value = defaultValue;

        for (auto idx = 0U; idx < tokens.size(); ++idx) {
            auto& t = tokens[idx];

            assert(!t.empty());
            if ((idx == 0) && (t.size() == 1)) {
                opt->m_shortOpt = t;
                continue;
            }

            if (!opt->m_longOpt.empty()) {
                break;
            }

            opt->m_longOpt = t;
        }

        m_opts.push_back(std::move(opt));
    }

    void genPrepareOpts()
    {
        m_shortOpts.clear();
        m_longOpts.clear();
        for (auto& opt : m_opts) {
            if (!opt->m_shortOpt.empty()) {
                m_shortOpts[opt->m_shortOpt] = opt.get();
            }

            if (!opt->m_longOpt.empty()) {
                m_longOpts[opt->m_longOpt] = opt.get();
            }
        }
    }

    void genReportUnknownOption(const std::string& opt)
    {
        std::cerr << "WARNING: Unknown option \"" << opt << "\"!" << std::endl;
    }

    static const OptInfo* genFindOptInfo(const std::string& optStr, const OptInfosMap& map)
    {
        auto iter = map.find(optStr);
        if (iter == map.end()) {
            return nullptr;
        }

        auto* optInfo = iter->second;
        assert(optInfo != nullptr);
        return optInfo;
    }

    static bool genIsOptUsedInternal(const std::string& optStr, const OptInfosMap& map)
    {
        auto* optInfo = genFindOptInfo(optStr, map);
        if (optInfo == nullptr) {
            return false;
        }

        return optInfo->m_wasUsed;
    }

    static const std::string& genValueInternal(const std::string& optStr, const OptInfosMap& map)
    {
        auto* optInfo = genFindOptInfo(optStr, map);
        if ((optInfo == nullptr) || (!optInfo->m_hasParam)) {
            static const std::string EmptyStr;
            return EmptyStr;
        }

        return optInfo->m_value;
    }

    OptInfosList m_opts;
    OptInfosMap m_shortOpts;
    OptInfosMap m_longOpts;
    GenStringsList m_args;
    std::string m_app;
};

GenProgramOptions::GenProgramOptions() :
    m_impl(std::make_unique<GenProgramOptionsImpl>())
{
}

GenProgramOptions::~GenProgramOptions() = default;

GenProgramOptions& GenProgramOptions::genAddCommonOptions()
{
    return
        (*this)
            (GenFullHelpOptStr, "Show this help")
            (GenVersionOptStr, "Print version string and exit.")
            (GenFullQuietStr, "Quiet, show only warnings and errors.")
            (GenFullDebugStr, "Show debug logging.")
            (GenFullWarnAsErrStr, "Treat warnings as errors.")
            (GenFullOutputDirStr, "Output directory path. When not provided current is used.", true)
            (GenFullInputFilesListStr, "File containing list of input files.", true)
            (GenFullInputFilesPrefixStr, "Prefix for the values from the list file.", true)
            (GenFullCodeInputDirStr, "Directory with code updates.", true)
            (GenFullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")
            (GenFullNamespaceStr,
                "Force main namespace change. Defaults to schema name. "
                "In case of having multiple schemas the renaming happends to the last protocol one. "
                "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
                true)
            (GenFullMinRemoteVerStr, "Set minimal supported remote version. Defaults to 0.", true)
            (GenForceVerStr,
                "Force schema version. Must not be greater than version specified in schema file.", true)

            ;
}

GenProgramOptions& GenProgramOptions::genAddCodeVersionOptions()
{
    return
        (*this)
            (GenFullCodeVerStr,
                "Specify semantic version of the generated protocol code using <major>.<minor>.<patch> "
                "format to make this information available in the generated code", true)
        ;
}

GenProgramOptions& GenProgramOptions::genRemoveMinRemoteVersionOptions()
{
    m_impl->genRemove(GenFullMinRemoteVerStr);
    return *this;
}

GenProgramOptions& GenProgramOptions::operator()(const std::string& optStr, const std::string& desc, bool hasParam)
{
    m_impl->genAdd(optStr, desc, hasParam);
    return *this;
}

GenProgramOptions& GenProgramOptions::operator()(const std::string& optStr, const std::string& desc, const std::string& defaultValue)
{
    m_impl->genAdd(optStr, desc, defaultValue);
    return *this;
}

void GenProgramOptions::genParse(int argc, const char** argv)
{
    m_impl->genParse(argc, argv);
}

bool GenProgramOptions::genIsOptUsed(const std::string& optStr) const
{
    return m_impl->genIsOptUsed(optStr);
}

const std::string& GenProgramOptions::genValue(const std::string& optStr) const
{
    return m_impl->genValue(optStr);
}

const GenProgramOptions::GenStringsList& GenProgramOptions::genArgs() const
{
    return m_impl->genArgs();
}

const std::string& GenProgramOptions::genApp() const
{
    return m_impl->genApp();
}

std::string GenProgramOptions::genHelpStr() const
{
    return m_impl->genHelpStr();
}

bool GenProgramOptions::genHelpRequested() const
{
    return genIsOptUsed(GenHelpOptStr);
}

bool GenProgramOptions::genVersionRequested() const
{
    return genIsOptUsed(GenVersionOptStr);
}

bool GenProgramOptions::genQuietRequested() const
{
    return genIsOptUsed(GenQuietStr);
}

bool GenProgramOptions::genDebugRequested() const
{
    return genIsOptUsed(GenDebugStr);
}

bool GenProgramOptions::genWarnAsErrRequested() const
{
    return genIsOptUsed(GenWarnAsErrStr);
}

const std::string& GenProgramOptions::genGetOutputDirectory() const
{
    return genValue(GenOutputDirStr);
}

GenProgramOptions::GenStringsList GenProgramOptions::genGetInputFiles() const
{
    std::vector<std::string> result;
    do {
        auto& fileName = genValue(GenInputFilesListStr);
        if (fileName.empty()) {
            break;
        }

        std::ifstream stream(fileName);
        if (!stream) {
            break;
        }

        std::string contents(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));

        result = util::genStrSplitByAnyChar(contents, "\r\n");

        auto& prefix = genValue(GenInputFilesPrefixStr);
        if (prefix.empty()) {
            break;
        }

        for (auto& f : result) {
            f = util::genPathAddElem(prefix, f);
        }
    } while (false);

    auto& otherFiles = genArgs();
    result.insert(result.end(), otherFiles.begin(), otherFiles.end());
    return result;
}

const std::string& GenProgramOptions::genGetCodeInputDirectory() const
{
    return genValue(GenCodeInputDirStr);
}

bool GenProgramOptions::genMultipleSchemasEnabled() const
{
    return genIsOptUsed(GenMultipleSchemasEnabledStr);
}

bool GenProgramOptions::genHasNamespaceOverride() const
{
    return genIsOptUsed(GenNamespaceStr);
}

const std::string& GenProgramOptions::genGetNamespace() const
{
    return genValue(GenNamespaceStr);
}

unsigned GenProgramOptions::genGetMinRemoteVersion() const
{
    if (!genIsOptUsed(GenMinRemoteVerStr)) {
        return 0U;
    }

    return util::genStrToUnsigned(genValue(GenMinRemoteVerStr));
}

bool GenProgramOptions::genHasForcedSchemaVersion() const
{
    return genIsOptUsed(GenForceVerStr);
}

unsigned GenProgramOptions::genGetForcedSchemaVersion() const
{
    return commsdsl::gen::util::genStrToUnsigned(genValue(GenForceVerStr));
}

const std::string& GenProgramOptions::genGetCodeVersion() const
{
    return genValue(GenCodeVerStr);
}

} // namespace gen

} // namespace commsdsl