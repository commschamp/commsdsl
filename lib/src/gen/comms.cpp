#include "commsdsl/gen/comms.h"

#include "commsdsl/gen/strings.h"

#include <cctype>
#include <vector>

namespace commsdsl
{

namespace gen
{

namespace comms
{

namespace 
{

const std::string ScopeSep("::");

std::string scopeForElement(
    const std::string& name, 
    const Generator& generator, 
    const std::vector<std::string>& subElems,
    bool addMainNamespace, 
    bool addElement)
{
    std::string result;
    if (addMainNamespace) {
        result = generator.mainNamespace();
    }

    for (auto& elem : subElems) {
        if (!result.empty()) {
            result.append(ScopeSep);
        }

        result.append(elem);
    }

    if (addElement) {
        if (!result.empty()) {
            result.append(ScopeSep);
        }        
        result.append(name);
    }

    return result;
}

} // namespace 
    

std::string className(const std::string& name)
{
    auto result = name;
    if (!result.empty()) {
        result[0] = static_cast<char>(std::toupper(static_cast<int>(result[0])));
    }

    return result;
}    

std::string scopeFor(
    const Elem& elem, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    std::string result;
    auto* parent = elem.getParent();
    if (parent != nullptr) {
        result = scopeFor(*parent, generator, addMainNamespace, true);
    }
    else if (addMainNamespace) {
        result = generator.mainNamespace();
    }

    do {
        if (!addElement) {
            break;
        }

        if (elem.name().empty()) {
            break;
        }


        if (!result.empty()) {
            result.append(ScopeSep);
        }

        if (elem.elemType() == Elem::Type_Namespace) {
            result.append(elem.name());
            break;
        }

        result.append(className(elem.name()));

    } while (false);

    return result;
}

std::string scopeForInterface(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems;
    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string scopeForOptions(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::optionsStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

std::string scopeForInput(
    const std::string& name, 
    const Generator& generator, 
    bool addMainNamespace, 
    bool addElement)
{
    static const std::vector<std::string> SubElems = {
        strings::inputStr()
    };

    return scopeForElement(name, generator, SubElems, addMainNamespace, addElement);
}

} // namespace comms

} // namespace gen

} // namespace commsdsl
