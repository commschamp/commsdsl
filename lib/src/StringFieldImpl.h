#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "FieldImpl.h"

namespace commsdsl
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

    std::size_t length() const
    {
        return m_state.m_length;
    }

    bool hasPrefixField() const
    {
        return (m_state.m_extPrefixField != nullptr) || static_cast<bool>(m_prefixField);
    }

    Field prefixField() const
    {
        if (m_state.m_extPrefixField != nullptr) {
            return Field(m_state.m_extPrefixField);
        }

        return Field(m_prefixField.get());
    }

    const std::string& detachedPrefixFieldName() const
    {
        return m_state.m_detachedPrefixField;
    }

    bool hasZeroTermSuffix() const
    {
        return m_state.m_haxZeroSuffix;
    }

protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;

private:
    bool updateDefaultValue();
    bool updateEncoding();
    bool updateLength();
    bool updatePrefix();
    bool updateZeroTerm();
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();
    const FieldImpl* getPrefixField() const;

    struct State
    {
        std::string m_defaultValue;
        std::string m_encoding;
        std::size_t m_length = 0U;
        const FieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        bool m_haxZeroSuffix = false;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace commsdsl
