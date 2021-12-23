#include "commsdsl/gen/util.h"

#include "commsdsl/gen/strings.h"

#include <limits>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace commsdsl
{

namespace gen
{

namespace util
{

std::string strReplace(const std::string& str, const std::string& what, const std::string& with)
{
    std::string result;
    std::size_t pos = 0U;
    while (pos < str.size()) {
        auto nextPos = str.find(what, pos);
        if (str.size() <= nextPos) {
            result.append(str, pos);
            break;
        }

        result.append(str, pos, nextPos - pos);
        result.append(with);
        pos = nextPos + what.size();
    }
    return result;
}    

std::string strToName(const std::string& value)
{
    auto result = strReplace(value, ".", "_");
    result = strReplace(result, "-", "_");
    result = strReplace(result, " ", "_");
    return result;
}

std::vector<std::string> strSplitByAnyCharCompressed(
    const std::string& str, 
    const std::string& splitChars)
{
    std::vector<std::string> result;
    std::size_t pos = 0U;
    while (pos < str.size()) {
        auto nextPos = str.find_first_of(splitChars, pos);

        if (nextPos == pos) {
            ++pos;
            continue;
        }

        if (str.size() <= nextPos) {
            nextPos = str.size();
        }

        auto count = nextPos - pos;
        result.push_back(std::string(str, pos, count));
        pos = nextPos + 1;
    }
    return result;
}

std::string numToString(std::uintmax_t value, unsigned hexWidth)
{
    if (hexWidth == 0U) {
        if (value <= std::numeric_limits<std::uint16_t>::max()) {
            return std::to_string(value) + "U";
        }

        if (value <= std::numeric_limits<std::uint32_t>::max()) {
            return std::to_string(value) + "UL";
        }
    }

    std::stringstream stream;
    stream << std::hex << "0x" << std::uppercase <<
              std::setfill('0') << std::setw(hexWidth) << value;
    if ((0U < hexWidth) && (value <= std::numeric_limits<std::uint16_t>::max())) {
        stream << "U";
    }
    else if ((0U < hexWidth) && (value <= std::numeric_limits<std::uint32_t>::max())) {
        stream << "UL";
    }
    else {
        stream << "ULL";
    }
    return stream.str();
}

std::string numToString(std::intmax_t value)
{
    if ((std::numeric_limits<std::int16_t>::min() <= value) &&
        (value <= std::numeric_limits<std::int16_t>::max())) {
        return std::to_string(value);
    }

    if ((std::numeric_limits<std::int32_t>::min() <= value) &&
        (value <= std::numeric_limits<std::int32_t>::max())) {
        return std::to_string(value) + "L";
    }

    if (0 < value) {
        std::stringstream stream;
        stream << std::hex << "0x" << value << "LL";
        return stream.str();
    }

    std::stringstream stream;
    stream << std::hex << "-0x" << -value << "LL";
    return stream.str();
}

std::string numToString(unsigned value, unsigned hexWidth)
{
    return numToString(static_cast<std::uintmax_t>(value), hexWidth);
}

std::string pathAddElem(const std::string& path, const std::string& elem)
{
#ifdef WIN32
    static const char Sep = '\\';
#else
    static const char Sep = '/';
#endif         

    std::string result = path;
    if ((!result.empty()) && (result.back() != Sep)) {
        result.push_back(Sep);
    }

    result.append(elem);
    return result;
}

std::string processTemplate(const std::string& templ, const ReplacementMap& repl)
{
    std::string result;
    result.reserve(templ.size() * 2U);
    std::size_t templPos = 0U;
    while (templPos < templ.size()) {
        static const std::string Prefix("#^#");
        auto prefixPos = templ.find(Prefix, templPos);
        if (prefixPos == std::string::npos) {
            break;
        }

        static const std::string Suffix("#$#");
        auto suffixPos = templ.find(Suffix, prefixPos + Prefix.size());
        if (suffixPos == std::string::npos) {
            static constexpr bool Incorrect_template = false;
            static_cast<void>(Incorrect_template);
            assert(Incorrect_template);            
            templPos = templ.size();
            break;
        }
        auto afterSuffixPos = suffixPos + Suffix.size();

        std::string key(templ.begin() + prefixPos + Prefix.size(), templ.begin() + suffixPos);
        const std::string* valuePtr = &commsdsl::gen::strings::emptyString();
        auto iter = repl.find(key);
        if (iter != repl.end()) {
            valuePtr = &(iter->second);
        }
        auto& value = *valuePtr;

        std::size_t lineStartPos = 0U;
        std::size_t lastNewLinePos = templ.find_last_of('\n', prefixPos);
        if (lastNewLinePos != std::string::npos) {
            lineStartPos = lastNewLinePos + 1U;
        }

        assert(lineStartPos <= prefixPos);
        auto indent = prefixPos - lineStartPos;

        // Check empty row
        std::size_t posToCopyUntil = prefixPos;
        std::size_t nextTemplPos = afterSuffixPos;
        do {
            if (!value.empty()) {
                break;
            }

            static const std::string WhiteSpaces(" \t\r");
            std::string preStr(templ.begin() + lineStartPos, templ.begin() + prefixPos);
            if ((!preStr.empty()) &&
                (preStr.find_first_not_of(WhiteSpaces) != std::string::npos)) {
                break;
            }

            auto nextNewLinePos = templ.find_first_of('\n', suffixPos + Suffix.size());
            if (nextNewLinePos == std::string::npos) {
                static constexpr bool Incorrect_template = false;
                static_cast<void>(Incorrect_template);
                assert(Incorrect_template);  
                break;
            }

            std::string postStr(templ.begin() + afterSuffixPos, templ.begin() + nextNewLinePos);
            if ((!postStr.empty()) &&
                (postStr.find_first_not_of(WhiteSpaces) != std::string::npos)) {
                break;
            }

            posToCopyUntil = lineStartPos;
            nextTemplPos = nextNewLinePos + 1;
        } while (false);

        result.insert(result.end(), templ.begin() + templPos, templ.begin() + posToCopyUntil);
        templPos = nextTemplPos;

        if (value.empty()) {
            continue;
        }

        if (indent == 0U) {
            result += value;
            continue;
        }

        std::string repSep("\n");
        repSep.reserve(repSep.size() + indent);
        std::fill_n(std::back_inserter(repSep), indent, ' ');
        auto updatedValue = strReplace(value, "\n", repSep);
        result += updatedValue;
    }

    if (templPos < templ.size()) {
        result.insert(result.end(), templ.begin() + templPos, templ.end());
    }
    return result;
}

} // namespace util

} // namespace gen

} // namespace commsdsl
