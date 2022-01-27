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

const std::string& commonSuffixStr()
{
    static const std::string Str("Common");
    return Str;     
}

const std::string& cppHeaderSuffixStr()
{
    static const std::string Str(".h");
    return Str;     
}

const std::string& cppSourceSuffixStr()
{
    static const std::string Str(".cpp");
    return Str;     
}

const std::string& fieldNamespaceStr()
{
    static const std::string Str("field");
    return Str;   
}

const std::string& messageNamespaceStr()
{
    static const std::string Str("message");
    return Str;   
}

const std::string& frameNamespaceStr()
{
    static const std::string Str("frame");
    return Str;   
}

const std::string& layerNamespaceStr()
{
    static const std::string Str("layer");
    return Str;   
}

const std::string& optionsNamespaceStr()
{
    static const std::string Str("options");
    return Str;   
}

const std::string& includeDirStr()
{
    static const std::string Str("include");
    return Str;   
}

const std::string& nameFileSuffixStr()
{
    static const std::string Str(".name");
    return Str;     
}

const std::string& readFileSuffixStr()
{
    static const std::string Str(".read");
    return Str;     
}

const std::string& refreshFileSuffixStr()
{
    static const std::string Str(".refresh");
    return Str;     
}

const std::string& writeFileSuffixStr()
{
    static const std::string Str(".write");
    return Str;     
}

const std::string& publicFileSuffixStr()
{
    static const std::string Str(".public");
    return Str;     
}

const std::string& protectedFileSuffixStr()
{
    static const std::string Str(".protected");
    return Str;     
}

const std::string& privateFileSuffixStr()
{
    static const std::string Str(".private");
    return Str;     
}

const std::string& incFileSuffixStr()
{
    static const std::string Str(".inc");
    return Str;     
}

const std::string& lengthFileSuffixStr()
{
    static const std::string Str(".length");
    return Str;     
}

const std::string& validFileSuffixStr()
{
    static const std::string Str(".valid");
    return Str;     
}

const std::string& forceEmptyDisplayNameStr()
{
    static const std::string Str("_");
    return Str;
}

const std::string& fieldBaseClassStr()
{
    static const std::string Str("FieldBase");
    return Str;    
}

const std::string& defaultOptionsClassStr()
{
    static const std::string Str("DefaultOptions");
    return Str;    
}

const std::string& indentStr()
{
    static const std::string Str("    ");
    return Str;    
}

const std::string& doxygenPrefixStr()
{
    static const std::string Str("/// ");
    return Str;      
}

const std::string& versionOptionalFieldSuffixStr()
{
    static const std::string Str("Field");
    return Str;      
}

const std::string& origSuffixStr()
{
    static const std::string Str("Orig");
    return Str;      
}

} // namespace strings

} // namespace gen

} // namespace commsdsl
