#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/DataField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class DataFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:

    using ValueType = DataField::ValueType;

    DataFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    DataFieldImpl(const DataFieldImpl& other);

    const ValueType& defaultValue() const
    {
        return m_state.m_defaultValue;
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


protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;

private:
    bool updateDefaultValue();
    bool updateLength();
    bool updatePrefix();
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();
    const FieldImpl* getPrefixField() const;

    struct State
    {
        ValueType m_defaultValue;
        std::size_t m_length = 0U;
        const FieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace commsdsl
