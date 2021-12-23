#include "commsdsl/gen/strings.h"

#include <limits>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace commsdsl
{

namespace gen
{

namespace strings
{

const std::string& emptyString()
{
    static const std::string Str;
    return Str;
}    

const std::string& msgIdEnumNameStr()
{
    static const std::string Str("MsgId");
    return Str;
}

} // namespace strings

} // namespace gen

} // namespace commsdsl
