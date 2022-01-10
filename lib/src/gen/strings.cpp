#include "commsdsl/gen/strings.h"

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

const std::string& cmakeListsFileStr()
{
    static const std::string Str("CMakeLists.txt");
    return Str;
}

const std::string& optionsStr()
{
    static const std::string Str("options");
    return Str;
}

const std::string& inputStr()
{
    static const std::string Str("input");
    return Str;
}

const std::string& defaultOptionsStr()
{
    static const std::string Str("DefaultOptions");
    return Str;
}

const std::string& allMessagesStr()
{
    static const std::string Str("AllMessages");
    return Str;    
}

const std::string& messageClassStr()
{
    static const std::string Str("Message");
    return Str;    
}

} // namespace strings

} // namespace gen

} // namespace commsdsl
