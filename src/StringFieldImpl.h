#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "FieldImpl.h"

namespace bbmp
{

class StringFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:

    StringFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    StringFieldImpl(const StringFieldImpl& other);

    const std::string& defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const std::string& encodingStr() const
    {
        return m_state.m_encoding;
    }

    unsigned length() const
    {
        return m_state.m_length;
    }

    bool hasPrefixField() const
    {
        return static_cast<bool>(m_prefixField);
    }

    Field prefixField() const
    {
        return Field(m_prefixField.get());
    }

    bool hasZeroTermSuffix() const
    {
        return m_state.m_haxZeroSuffix;
    }


protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;

private:
    bool updateDefaultValue();
    bool updateEncoding();
    bool updateLength();
    bool updatePrefix();
    bool updateZeroTerm();
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();

    struct State
    {
        std::string m_defaultValue;
        std::string m_encoding;
        std::size_t m_length = 0U;
        bool m_haxZeroSuffix = false;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace bbmp
