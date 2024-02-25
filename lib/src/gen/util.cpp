#include "commsdsl/gen/util.h"

#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>

namespace commsdsl
{

namespace gen
{

namespace util
{

namespace 
{

static const char PathSep = '/';

#ifdef WIN32
static const char WinPathSep = '\\';
#endif 

bool isPathSep(char ch)
{
    if (ch == PathSep) {
        return true;
    }

#ifdef WIN32
    if (ch == WinPathSep) {
        return true;
    }
#endif    

    return false;
}

void cleanSpaces(std::string& code)
{
    bool removeChar = false;
    std::size_t processedCount = 0U;

    code.erase(
        std::remove_if(
            code.begin(), code.end(),
            [&](const char& ch)
            {
                auto idx = static_cast<unsigned>(std::distance(code.c_str(), &ch));
                if (idx < processedCount) {
                    return removeChar;
                }

                if (ch != ' ') {
                    processedCount = idx + 1U;
                    removeChar = false;                    
                    return removeChar;
                }

                auto endLinePos = code.find_first_of("\n", idx + 1U);
                if (code.size() <= endLinePos) {
                    processedCount = code.size();
                    removeChar = false;
                    return removeChar;
                }

                processedCount = endLinePos;
                auto notSpacePos = code.find_first_not_of(" \t", idx + 1);                
                removeChar = (endLinePos <= notSpacePos);
                return removeChar;
            }),
        code.end());
}

void cleanExtraNewLines(std::string& code)
{
    code.erase(
        std::remove_if(
            code.begin(), code.end(),
            [&](const char& ch)
            {
                auto idx = static_cast<unsigned>(std::distance(code.c_str(), &ch));
                if (ch != '\n') {
                    return false;
                }

                auto notEndlPos = code.find_first_not_of("\n", idx);   
                if (code.size() <= notEndlPos) {
                    return idx < (code.size() - 1U);
                }

                auto endlCount = notEndlPos - idx;
                return endlCount > 2U; // No more than 2 lines
            }),
        code.end());
}

void cleanNewLinesBeforeCloseBracket(std::string& code)
{
    std::size_t processedCount = 0U;

    code.erase(
        std::remove_if(
            code.begin(), code.end(),
            [&](const char& ch)
            {
                auto idx = static_cast<unsigned>(std::distance(code.c_str(), &ch));
                if (idx < processedCount) {
                    return false;
                }

                processedCount = idx + 1U;
                if (ch != '\n') {
                    return false;
                }

                auto nextLinePos = code.find_first_not_of("\n", idx + 1);
                if (code.size() <= nextLinePos) {
                    processedCount = code.size();
                    return false;
                }

                assert(idx < nextLinePos);

                auto newLinesCount = (nextLinePos - idx);
                
                assert (newLinesCount <= 2U);
                if (newLinesCount <= 1U) {
                    return false;
                }

                std::size_t nonSpacePos = code.find_first_not_of(" \t", nextLinePos);
                if (code.size() <= nonSpacePos) {
                    processedCount = code.size();
                    return false;
                }    

                assert(idx < nonSpacePos);

                processedCount = nonSpacePos;
                if (code[nonSpacePos] != '}') {
                    return false;
                }           

                // Check the closing namespace
                static const std::string NamespaceClose("} // namespace");
                if (((nonSpacePos + NamespaceClose.size()) < code.size()) &&
                    (std::equal(NamespaceClose.begin(), NamespaceClose.end(), &code[nonSpacePos]))) {
                    // Don't strip lines when closing namespace
                    return false;
                }

                return true;           
            }),
        code.end());    
}

void doTidyCode(std::string& code)
{
    cleanSpaces(code);
    cleanExtraNewLines(code);
    cleanNewLinesBeforeCloseBracket(code);
}


} // namespace 



std::string strReplace(const std::string& str, const std::string& what, const std::string& with)
{
    std::string result;
    std::size_t pos = 0U;
    while (pos < str.size()) {
        auto nextPos = str.find(what, pos);
        if (str.size() <= nextPos) {
            result.append(str, pos, std::string::npos);
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

std::vector<std::string> strSplitByAnyChar(
    const std::string& str, 
    const std::string& splitChars,
    bool compressed)
{
    std::vector<std::string> result;
    std::size_t pos = 0U;
    while (pos < str.size()) {
        auto nextPos = str.find_first_of(splitChars, pos);

        if (compressed && (nextPos == pos)) {
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

std::string strInsertIndent(const std::string& str)
{
    return strings::indentStr() + strReplace(str, "\n", "\n" + strings::indentStr());
}

unsigned strToUnsigned(const std::string& str)
{
    try {
        return static_cast<unsigned>(std::stoul(str));
    }
    catch (...) {
        return 0U;
    }
}

bool strStartsWith(const std::string& str, const std::string& prefix)
{
    if (str.size() < prefix.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

std::string strToUpper(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    std::transform(
        str.begin(), str.end(), std::back_inserter(result),
        [](char ch)
        {
            return static_cast<char>(std::toupper(ch));
        });  
    return result;
}

std::string strToLower(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    std::transform(
        str.begin(), str.end(), std::back_inserter(result),
        [](char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });  
    return result;
}

std::string strToMacroName(const std::string& str)
{
    std::string result;
    for (char ch : str) {
        if ((!result.empty()) && ('A' <= ch) && (ch <= 'Z')) {
            result += '_';
        }

        result += static_cast<char>(std::toupper(ch));
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

const std::string& boolToString(bool value)
{
    if (value) {
        static const std::string TrueStr("true");
        return TrueStr;
    }

    static const std::string FalseStr("false");
    return FalseStr;
}

std::string pathAddElem(const std::string& path, const std::string& elem)
{
    std::string result = path;
    if ((!result.empty()) && (!isPathSep(result.back()))) {
        result.push_back(PathSep);
    }

    result.append(elem);
    return result;
}

std::string pathUp(const std::string& path)
{
    auto sepPos = path.rfind(PathSep);
    do {
        if (sepPos != std::string::npos) {
            break;
        }

#ifdef WIN32
        sepPos = path.rfind(WinPathSep);
        if (sepPos != std::string::npos) {
            break;
        }        
#endif     

        return strings::emptyString();
    } while (false);
    return path.substr(0, sepPos);
}

std::string processTemplate(const std::string& templ, const ReplacementMap& repl, bool tidyCode)
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
            [[maybe_unused]] static constexpr bool Incorrect_template = false;
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
                [[maybe_unused]] static constexpr bool Incorrect_template = false;
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

    if (tidyCode) {
        doTidyCode(result);
    }
    return result;
}

std::string strListToString(
    const StringsList& list,
    const std::string& join,
    const std::string& last)
{
    std::string result;
    for (auto& e : list) {
        result += e;
        if (&e != &list.back()) {
            result += join;
        }
        else {
            result += last;
        }
    }
    return result;
}

void addToStrList(std::string&& value, StringsList& list)
{
    auto iter = std::find(list.begin(), list.end(), value);
    if (iter == list.end()) {
        list.push_back(std::move(value));
    }
}

void addToStrList(const std::string& value, StringsList& list)
{
    auto iter = std::find(list.begin(), list.end(), value);
    if (iter == list.end()) {
        list.push_back(value);
    }
}

std::string strMakeMultiline(const std::string& value, unsigned len)
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

        auto insertFunc =
            [&result, &pos, &value](std::size_t newPos)
            {
                assert(pos <= newPos);
                assert(newPos <= value.size());
                result.insert(result.end(), value.begin() + pos, value.begin() + newPos);
                result.push_back('\n');
                pos = newPos + 1;
            };        

        auto newLinePos = value.find_last_of("\n", nextPos);
        if ((newLinePos != std::string::npos) && (pos <= newLinePos)) {
            insertFunc(newLinePos);
            continue;
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

std::string readFileContents(const std::string& filePath)
{
    std::string result;
    std::ifstream stream(filePath);
    if (stream) {
        result.assign(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));
    }
    
    return result;
}

bool isFileReadable(const std::string& filePath)
{
    std::ifstream stream(filePath);
    return static_cast<bool>(stream);
}

const std::string& displayName(const std::string& dslDisplayName, const std::string& dslName)
{
    if (dslDisplayName.empty()) {
        return dslName;
    }

    if (dslDisplayName == strings::forceEmptyDisplayNameStr()) {
        return strings::emptyString();
    }

    return dslDisplayName;
}

} // namespace util

} // namespace gen

} // namespace commsdsl
