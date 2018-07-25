#include "ListField.h"

#include <type_traits>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#MEMBERS_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<>,\n"
    "        #^#ELEMENT#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::ArrayList<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<>,\n"
    "            #^#ELEMENT#$##^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#MEMBERS_DEF#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<>,\n"
    "        #^#ELEMENT#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "};\n"
);

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH");
}

} // namespace

bool ListField::prepareImpl()
{
    auto obj = listFieldDslObj();

    do {
        auto elementField = obj.elementField();
        assert(elementField.valid());
        if (!elementField.externalRef().empty()) {
            break;
        }

        m_element = create(generator(), elementField);
        if (!m_element->prepare(dslObj().sinceVersion())) {
            return false;
        }
    } while (false);

    do {
        if (!obj.hasCountPrefixField()) {
            break;
        }

        auto prefix = obj.countPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_countPrefix = create(generator(), prefix);
        if (!m_countPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }
    } while (false);

    do {
        if (!obj.hasLengthPrefixField()) {
            break;
        }

        assert(!m_countPrefix);
        assert(!obj.hasCountPrefixField());
        auto prefix = obj.lengthPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_lengthPrefix = create(generator(), prefix);
        if (!m_lengthPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }
    } while (false);

    do {
        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        auto prefix = obj.elemLengthPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_elemLengthPrefix = create(generator(), prefix);
        if (!m_elemLengthPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }
    } while (false);

    return true;
}

void ListField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/ArrayList.h",
    };

    common::mergeIncludes(List, includes);

    auto obj = listFieldDslObj();
    do {
        if (m_element) {
            m_element->updateIncludes(includes);
            break;
        }

        auto elementField = obj.elementField();
        assert(elementField.valid());
        auto extRef = elementField.externalRef();
        assert(!extRef.empty());
        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_countPrefix) {
            m_countPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasCountPrefixField()) {
            break;
        }

        auto extRef = obj.countPrefixField().externalRef();
        assert(!extRef.empty());

        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_lengthPrefix) {
            m_lengthPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasLengthPrefixField()) {
            break;
        }

        auto extRef = obj.lengthPrefixField().externalRef();
        assert(!extRef.empty());

        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_elemLengthPrefix) {
            m_elemLengthPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        auto prefixField = obj.elemLengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);
}

void ListField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    if (m_element) {
        return;
    }

    auto obj = listFieldDslObj();
    auto elemField = obj.elementField();
    assert(elemField.valid());
    auto extRef = elemField.externalRef();
    assert(!extRef.empty());
    auto* elemFieldPtr = generator().findField(extRef, false);
    assert(elemFieldPtr != nullptr);
    elemFieldPtr->updatePluginIncludes(includes);
}

std::size_t ListField::maxLengthImpl() const
{
    auto obj = listFieldDslObj();
    if (obj.fixedCount() != 0U) {
        return Base::maxLengthImpl();
    }

    return std::numeric_limits<std::size_t>::max();
}

std::string ListField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("ELEMENT", getElement()));
    replacements.insert(std::make_pair("MEMBERS_DEF", getMembersDef(scope)));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements.insert(std::make_pair("COMMA", ","));
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string ListField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    std::string memberScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";
    StringsList options;

    auto addToOptionsFunc = 
        [&memberScope, &options](const FieldPtr& f)
        {
            if (!f) {
                return;
            }

            auto o = f->getDefaultOptions(memberScope);
            if (!o.empty()) {
                options.push_back(o);
            }
        };

    addToOptionsFunc(m_element);
    addToOptionsFunc(m_lengthPrefix);
    addToOptionsFunc(m_elemLengthPrefix);

    if (options.empty()) {
        return common::emptyString();
    }
    
    const std::string Templ =
        "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# string.\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#OPTIONS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", common::listToString(options, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string ListField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(value);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"List field is not expected to be comparable");
    return common::emptyString();
}

std::string ListField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(field);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"List field is not expected to be comparable");
    return common::emptyString();
}

std::string ListField::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    if (!m_element) {
        return common::emptyString();
    }

    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr();
    if (!externalRef().empty()) {
        fullScope += "<>";
    }
    fullScope += "::";

    if (isElemForcedSerialisedHiddenInPlugin()) {
        forcedSerialisedHidden = true;
        serHiddenParam = false;
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROPS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROPS", m_element->getPluginCreatePropsFunc(fullScope, forcedSerialisedHidden, serHiddenParam)));
    return common::processTemplate(Templ, replacements);
}

std::string ListField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    auto obj = listFieldDslObj();
    common::StringsList props;
    bool elemForcedSerHidden = isElemForcedSerialisedHiddenInPlugin();
    if (elemForcedSerHidden) {
        serHiddenParam = false;
    }

    if (m_element) {
        auto func =
            common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::createProps_" + common::nameToAccessCopy(m_element->name()) + "(";
        if (serHiddenParam) {
            func += common::serHiddenStr();
        }
        func += ")";
        props.push_back(".add(" + func + ")");
    }
    else {
        auto elemField = obj.elementField();
        assert(elemField.valid());
        auto extRef = elemField.externalRef();
        assert(!extRef.empty());
        auto scope = generator().scopeForFieldInPlugin(extRef);
        auto func = scope + "createProps_" + common::nameToAccessCopy(elemField.name()) +
        "(Field::ValueType::value_type::name()";
        if (serHiddenParam) {
            func += ", " + common::serHiddenStr();
        }
        func += ")";
        props.push_back(".add(" + func + ")");
    }

    do {
        if (elemForcedSerHidden) {
            break;
        }

        if ((!obj.hasCountPrefixField()) && (!obj.hasLengthPrefixField())) {
            props.push_back(".serialisedHidden()");
            break;
        }

        props.push_back(".prefixName(\"" + getPrefixName() + "\")");
        props.push_back(".showPrefix()");
    } while (false);
    return common::listToString(props, "\n", common::emptyString());
}

std::string ListField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkFixedSizeOpt(options);
    checkCountPrefixOpt(options);
    checkLengthPrefixOpt(options);
    checkElemLengthPrefixOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string ListField::getElement() const
{
    if (m_element) {
        auto str =
            "typename " + common::nameToClassCopy(name()) +
            common::membersSuffixStr();

        if (!externalRef().empty()) {
            str += "<TOpt>";
        }

        str += "::";
        str += common::nameToClassCopy(m_element->name());
        return str;
    }

    auto obj = listFieldDslObj();
    auto elementField = obj.elementField();
    assert(elementField.valid());
    auto extRef = elementField.externalRef();
    assert(!extRef.empty());
    return generator().scopeForField(extRef, true, true) + "<TOpt>";
}

std::string ListField::getMembersDef(const std::string& scope) const
{
    auto membersScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";

    StringsList defs;
    auto recordFieldFunc =
        [this](commsdsl::Field f)
        {
            assert(f.valid());
            auto extRef = f.externalRef();
            assert(!extRef.empty());
            auto* fieldPtr = generator().findField(extRef, true);
            static_cast<void>(fieldPtr);
            assert(fieldPtr != nullptr);
        };


    auto obj = listFieldDslObj();
    do {
        if (m_element) {
            defs.push_back(m_element->getClassDefinition(membersScope));
            break;
        }

        recordFieldFunc(obj.elementField());
    } while (false);

    do {
        if (m_countPrefix) {
            defs.push_back(m_countPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasCountPrefixField()) {
            break;
        }

        recordFieldFunc(obj.countPrefixField());
    } while (false);

    do {
        if (m_lengthPrefix) {
            defs.push_back(m_lengthPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasLengthPrefixField()) {
            break;
        }

        recordFieldFunc(obj.lengthPrefixField());
    } while (false);

    do {
        if (m_elemLengthPrefix) {
            defs.push_back(m_elemLengthPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        recordFieldFunc(obj.elemLengthPrefixField());
    } while (false);


    if (defs.empty()) {
        return common::emptyString();
    }

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    static const std::string Templ =
        "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# list.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#MEMBERS_DEF#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("MEMBERS_DEF", common::listToString(defs, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

void ListField::checkFixedSizeOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    auto fixedCount = obj.fixedCount();
    if (fixedCount == 0U) {
        return;
    }

    auto str =
        "comms::option::SequenceFixedSize<" +
        common::numToString(static_cast<std::uintmax_t>(fixedCount)) +
        ">";
    list.push_back(std::move(str));
}

void ListField::checkCountPrefixOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasCountPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_countPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_countPrefix->name());
    }
    else {
        auto prefixField = obj.countPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::SequenceSizeFieldPrefix<" + prefixName + '>');
}

void ListField::checkLengthPrefixOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_lengthPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_lengthPrefix->name());
    }
    else {
        auto prefixField = obj.lengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void ListField::checkElemLengthPrefixOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasElemLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_elemLengthPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_elemLengthPrefix->name());
    }
    else {
        auto prefixField = obj.elemLengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    std::string opt = "SequenceElemSerLengthFieldPrefix";
    if (obj.elemFixedLength()) {
        opt = "SequenceElemFixedSerLengthFieldPrefix";
    }

    list.push_back("comms::option::" + opt + "<" + prefixName + '>');
}

bool ListField::isElemForcedSerialisedHiddenInPlugin() const
{
    auto obj = listFieldDslObj();
    return obj.hasElemLengthPrefixField();
}

std::string ListField::getPrefixName() const
{
    auto obj = listFieldDslObj();
    do {
        if (!obj.hasCountPrefixField()) {
            break;
        }

        if (m_countPrefix) {
            return m_countPrefix->displayName();
        }

        auto countField = obj.countPrefixField();
        assert(countField.valid());
        auto* name = &countField.displayName();
        if (name->empty()) {
            name = &countField.name();
        }
        return *name;
    } while (false);

    assert(obj.hasLengthPrefixField());

    if (m_lengthPrefix) {
        return m_lengthPrefix->displayName();
    }

    auto lengthPrefix = obj.countPrefixField();
    assert(lengthPrefix.valid());
    auto* name = &lengthPrefix.displayName();
    if (name->empty()) {
        name = &lengthPrefix.name();
    }
    return *name;

}

} // namespace commsdsl2comms
