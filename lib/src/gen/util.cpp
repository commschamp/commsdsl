#include "commsdsl/gen/util.h"

#include <limits>
#include <sstream>
#include <iomanip>

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
    if (result.back() != Sep) {
        result.push_back(Sep);
    }

    result.append(elem);
    return result;
}

} // namespace util

} // namespace gen

} // namespace commsdsl
