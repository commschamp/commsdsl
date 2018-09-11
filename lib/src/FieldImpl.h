#pragma once

#include <memory>
#include <map>
#include <functional>
#include <string>

#include "commsdsl/Field.h"
#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"

namespace commsdsl
{

class ProtocolImpl;
class FieldImpl : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<FieldImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using FieldsList = std::vector<Ptr>;
    using Kind = Field::Kind;
    using SemanticType = Field::SemanticType;

    virtual ~FieldImpl() = default;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ProtocolImpl& protocol);
    Ptr clone() const
    {
        return cloneImpl();
    }

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();
    bool verifySiblings(const FieldsList& fields) const
    {
        return verifySiblingsImpl(fields);
    }

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

    Kind kind() const
    {
        return kindImpl();
    }

    SemanticType semanticType() const
    {
        return m_state.m_semanticType;
    }

    bool isPseudo() const
    {
        return m_state.m_pseudo;
    }

    bool isDisplayReadOnly() const
    {
        return m_state.m_displayReadOnly;
    }    

    bool isDisplayHidden() const
    {
        return m_state.m_displayHidden;
    }    

    bool isCustomizable() const
    {
        return m_state.m_customizable;
    }    

    std::size_t minLength() const
    {
        return minLengthImpl();
    }

    std::size_t maxLength() const
    {
        return maxLengthImpl();
    }

    std::size_t bitLength() const
    {
        return bitLengthImpl();
    }

    static XmlWrap::NamesList supportedTypes();

    static bool validateMembersNames(
            const FieldsList& fields,
            Logger& logger);

    bool validateMembersNames(const FieldsList& fields);

    const XmlWrap::NamesList& extraPropsNames() const
    {
        return extraPropsNamesImpl();
    }

    const XmlWrap::NamesList& extraPossiblePropsNames() const
    {
        return extraPossiblePropsNamesImpl();
    }

    const XmlWrap::NamesList& extraChildrenNames() const
    {
        return extraChildrenNamesImpl();
    }

    bool isBitfieldMember() const;
    bool isBundleMember() const;
    bool isMessageMember() const;

    std::string externalRef() const;

    bool isComparableToValue(const std::string& val) const
    {
        return isComparableToValueImpl(val);
    }

    bool isComparableToField(const FieldImpl& field) const;

    const PropsMap& extraAttributes() const
    {
        return m_state.m_extraAttrs;
    }

    PropsMap& extraAttributes()
    {
        return m_state.m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_state.m_extraChildren;
    }

    ContentsList& extraChildren()
    {
        return m_state.m_extraChildren;
    }


protected:
    FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    FieldImpl(const FieldImpl&);

    ProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    void setName(const std::string& val)
    {
        m_state.m_name = &val;
    }

    void setDisplayName(const std::string& val)
    {
        m_state.m_displayName = &val;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    virtual ObjKind objKindImpl() const override final;
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const;
    virtual bool reuseImpl(const FieldImpl& other);
    virtual bool parseImpl();
    virtual bool verifySiblingsImpl(const FieldsList& fields) const;
    virtual std::size_t minLengthImpl() const = 0;
    virtual std::size_t maxLengthImpl() const;
    virtual std::size_t bitLengthImpl() const;
    virtual bool isComparableToValueImpl(const std::string& val) const;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateNoPropInstance(const std::string& str);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);

    static const XmlWrap::NamesList& commonProps();
    static const XmlWrap::NamesList& commonChildren();
    const FieldImpl* findSibling(const FieldsList& fields, const std::string& sibName) const;
    static Kind getNonRefFieldKind(const FieldImpl& field);
    bool checkDetachedPrefixAllowed() const;

private:

    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    struct ReusableState
    {
        const std::string* m_name = nullptr;
        const std::string* m_displayName = nullptr;
        const std::string* m_description = nullptr;
        PropsMap m_extraAttrs;
        ContentsList m_extraChildren;
        SemanticType m_semanticType = SemanticType::None;
        bool m_pseudo = false;
        bool m_displayReadOnly = false;
        bool m_displayHidden = false;
        bool m_customizable = false;
    };

    bool checkReuse();
    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateVersions();
    bool updateSemanticType();
    bool updatePseudo();
    bool updateDisplayReadOnly();
    bool updateDisplayHidden();
    bool updateCustomizable();
    bool updateExtraAttrs(const XmlWrap::NamesList& names);
    bool updateExtraChildren(const XmlWrap::NamesList& names);

    bool verifySemanticType() const;
    bool verifyName() const;

    static const CreateMap& createMap();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    ReusableState m_state;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace commsdsl
