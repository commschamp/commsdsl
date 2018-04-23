#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "FieldImpl.h"

namespace bbmp
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

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other);
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual bool isComparableToValueImpl(const std::string& val) const override;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override;

private:
    const FieldImpl* m_field = nullptr;
};

} // namespace bbmp
