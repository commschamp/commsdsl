#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "FieldImpl.h"

namespace commsdsl
{

class RefFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    RefFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    RefFieldImpl(const RefFieldImpl& other);

    Field field() const
    {
        return Field(m_field);
    }

    const FieldImpl* fieldImpl() const
    {
        return m_field;
    }

protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual bool isComparableToValueImpl(const std::string& val) const override final;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override final;

private:
    const FieldImpl* m_field = nullptr;
};

} // namespace commsdsl
