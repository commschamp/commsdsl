#include "commsdsl/gen/ProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <vector>
#include <cassert>
#include <map>
#include <iostream>

namespace commsdsl
{

namespace gen
{

class ProgramOptionsImpl
{
public:
    using ArgsList = ProgramOptions::ArgsList;

    void add(const std::string& optStr, const std::string& desc, bool hasParam)
    {
        addInternal(optStr, desc, hasParam);
    }

    void add(const std::string& optStr, const std::string& desc, const std::string& defaultValue)
    {
        addInternal(optStr, desc, true, defaultValue);
    }

    void parse(int argc, const char** argv)
    {
        prepareOpts();

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
                reportUnknownOption(nextStr);
                continue;
            }

            auto processOptFunc = 
                [this, &opt, &nextStr](const std::string& optStr, OptInfosMap& map)
                {
                    auto iter = map.find(optStr);
                    if (iter == map.end()) {
                        reportUnknownOption(nextStr);
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

    bool isOptUsed(const std::string& optStr) const
    {
        if ((optStr.size() == 1) && isOptUsedInternal(optStr, m_shortOpts)) {
            return true;
        }

        return isOptUsedInternal(optStr, m_longOpts);
    }

    const std::string& value(const std::string& optStr) const
    {
        if ((optStr.size() == 1) && (findOptInfo(optStr, m_shortOpts) != nullptr)) {
            return valueInternal(optStr, m_shortOpts);
        }

        return valueInternal(optStr, m_longOpts);
    }

    const ArgsList& args() const
    {
        return m_args;
    }

    std::string helpStr() const
    {
        util::StringsList opts;
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

            
            right += util::strReplace(util::strMakeMultiline(optPtr->m_desc, 40), "\n", Ind);
            opts.push_back(left + right);
        }

        static const std::string Templ =
            "Options:\n"
            "  #^#OPTS_LIST#$#\n";

        util::ReplacementMap repl = {
            std::make_pair("OPTS_LIST", util::strListToString(opts, "\n"))
        };
        
        return util::processTemplate(Templ, repl);
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

    void addInternal(const std::string& optStr, const std::string& desc, bool hasParam = false, const std::string& defaultValue = std::string())
    {
        auto tokens = util::strSplitByAnyCharCompressed(optStr, ",");

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

    void prepareOpts()
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

    void reportUnknownOption(const std::string& opt)
    {
        std::cerr << "WARNING: Unknown option \"" << opt << "\"!" << std::endl;
    }

    static const OptInfo* findOptInfo(const std::string& optStr, const OptInfosMap& map)
    {
        auto iter = map.find(optStr);
        if (iter == map.end()) {
            return nullptr;
        }

        auto* optInfo = iter->second;
        assert(optInfo != nullptr);
        return optInfo;
    }

    static bool isOptUsedInternal(const std::string& optStr, const OptInfosMap& map)
    {
        auto* optInfo = findOptInfo(optStr, map);
        if (optInfo == nullptr) {
            return false;
        }

        return optInfo->m_wasUsed;
    }

    static const std::string& valueInternal(const std::string& optStr, const OptInfosMap& map)
    {
        auto* optInfo = findOptInfo(optStr, map);
        if ((optInfo == nullptr) || (!optInfo->m_hasParam)) {
            static const std::string EmptyStr;
            return EmptyStr;
        }

        return optInfo->m_value;
    }    

    OptInfosList m_opts;
    OptInfosMap m_shortOpts;
    OptInfosMap m_longOpts;
    ArgsList m_args;
};
    
ProgramOptions::ProgramOptions() : 
    m_impl(std::make_unique<ProgramOptionsImpl>())
{
}
 
ProgramOptions::~ProgramOptions() = default;

ProgramOptions& ProgramOptions::addHelpOption()
{
    return (*this)("h,help", "This help");
}

ProgramOptions& ProgramOptions::operator()(const std::string& optStr, const std::string& desc, bool hasParam)
{
    m_impl->add(optStr, desc, hasParam);
    return *this;
}

ProgramOptions& ProgramOptions::operator()(const std::string& optStr, const std::string& desc, const std::string& defaultValue)
{
    m_impl->add(optStr, desc, defaultValue);
    return *this;
}

void ProgramOptions::parse(int argc, const char** argv)
{
    m_impl->parse(argc, argv);
}

bool ProgramOptions::isOptUsed(const std::string& optStr) const
{
    return m_impl->isOptUsed(optStr);
}

bool ProgramOptions::helpRequested() const
{
    return isOptUsed("h");
}

const std::string& ProgramOptions::value(const std::string& optStr) const
{
    return m_impl->value(optStr);
}

const ProgramOptions::ArgsList& ProgramOptions::args() const
{
    return m_impl->args();
}

std::string ProgramOptions::helpStr() const
{
    return m_impl->helpStr();
}

} // namespace gen

} // namespace commsdsl