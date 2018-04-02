#pragma once

#include <memory>
#include <map>
#include <functional>
#include <string>

#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"

namespace bbmp
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

    void setNode(::xmlNodePtr val)
    {
        m_node = val;
    }

    bool parse();

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

    std::size_t length() const
    {
        return lengthImpl();
    }

    std::size_t bitLength() const
    {
        return bitLengthImpl();
    }

    static XmlWrap::NamesList supportedTypes();
    static bool validateMembersVersions(
            const Object& obj,
            const FieldsList& fields,
            Logger& logger);
    bool validateMembersVersions(const FieldsList& fields);

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

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    virtual ObjKind objKindImpl() const override final;
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const;
    virtual bool parseImpl();
    virtual std::size_t lengthImpl() const = 0;
    virtual std::size_t bitLengthImpl() const;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateNoPropInstance(const std::string& str);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);

    static const XmlWrap::NamesList& commonProps();
    static const XmlWrap::NamesList& commonChildren();

private:

    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateSinceVersion();
    bool updateDeprecated();

    static const CreateMap& createMap();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    Object* m_parent = nullptr;
    PropsMap m_props;

    const std::string* m_name = nullptr;
    const std::string* m_displayName = nullptr;
    const std::string* m_description = nullptr;
    PropsMap m_unknownAttrs;
    ContentsList m_unknownChildren;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace bbmp
