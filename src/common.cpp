#include "common.h"

#include <cctype>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace common
{

const std::string& emptyString()
{
    static const std::string Str;
    return Str;
}

const std::string& headerSuffix()
{
    static const std::string Str(".h");
    return Str;
}

const std::string& includeStr()
{
    static const std::string Str("include");
    return Str;
}

const std::string& messageStr()
{
    static const std::string Str("message");
    return Str;
}

void nameToClass(std::string& str)
{
    if (str.empty()) {
        return;
    }

    str[0] = static_cast<char>(std::toupper(static_cast<int>(str[0])));
}

void updateName(std::string& str)
{
    ba::replace_all(str, " ", "_");
    ba::replace_all(str, ".", "_");
}

std::string adjustName(const std::string& str)
{
    std::string result = str;
    updateName(result);
    return result;
}

std::string numToString(std::uintmax_t value)
{
    if (value <= std::numeric_limits<std::uint16_t>::max()) {
        return std::to_string(value);
    }

    if (value <= std::numeric_limits<std::uint32_t>::max()) {
        return std::to_string(value) + "L";
    }

    std::stringstream stream;
    stream << std::hex << "0x" << value << "LL";
    return stream.str();
}

std::string makeMultiline(const std::string& value, unsigned len)
{
    if (value.size() <= len) {
        return value;
    }

    assert(0U < len);
    std::string result;
    result.reserve((value.size() * 3) / 2);
    std::size_t pos = 0;
    while (pos < value.size()) {
        auto nextPos = pos + len;
        if (value.size() <= nextPos) {
            break;
        }

        static const std::string WhiteSpace(" \t\r");
        auto prePos = value.find_last_of(WhiteSpace, nextPos);
        if ((prePos == std::string::npos) || (prePos < pos)) {
            prePos = pos;
        }

        auto postPos = value.find_first_of(WhiteSpace, nextPos + 1);
        if (postPos == std::string::npos) {
            postPos = value.size();
        }

        if ((prePos <= pos) && (value.size() <= postPos)) {
            break;
        }

        auto insertFunc =
            [&result, &pos, &value](std::size_t newPos)
            {
                assert(pos <= newPos);
                assert(newPos <= value.size());
                result.insert(result.end(), value.begin() + pos, value.begin() + newPos);
                result.push_back('\n');
                pos = newPos + 1;
            };

        if (prePos <= pos) {
            insertFunc(postPos);
            continue;
        }

        if (value.size() <= postPos) {
            insertFunc(prePos);
            continue;
        }

        auto preDiff = nextPos - prePos;
        auto postDiff = postPos - nextPos;

        if (preDiff <= postDiff) {
            insertFunc(prePos);
            continue;
        }

        insertFunc(postPos);
    }

    if (pos < value.size()) {
        result.insert(result.end(), value.begin() + pos, value.end());
    }

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
            assert(!"Incorrect template");
            templPos = templ.size();
            break;
        }
        auto afterSuffixPos = suffixPos + Suffix.size();

        std::string key(templ.begin() + prefixPos + Prefix.size(), templ.begin() + suffixPos);
        const std::string* valuePtr = &emptyString();
        auto iter = repl.find(key);
        if (iter != repl.end()) {
            valuePtr = &(iter->second);
        }
        auto& value = *valuePtr;

        auto lineStartPos = 0U;
        auto lastNewLinePos = templ.find_last_of('\n', prefixPos);
        if (lastNewLinePos != std::string::npos) {
            lineStartPos = lastNewLinePos + 1U;
        }

        assert(lastNewLinePos <= prefixPos);
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
                assert(!"Incorrect template");
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
        auto updatedValue = ba::replace_all_copy(value, "\n", repSep);
        result += updatedValue;
    }

    if (templPos < templ.size()) {
        result.insert(result.end(), templ.begin() + templPos, templ.end());
    }
    return result;
}

} // namespace common

} // namespace commsdsl2comms
