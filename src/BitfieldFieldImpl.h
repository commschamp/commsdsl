#pragma once

#include "commsdsl/Endian.h"
#include "commsdsl/BitfieldField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class BitfieldFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    BitfieldFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    BitfieldFieldImpl(const BitfieldFieldImpl& other);
    using Members = BitfieldField::Members;

    Endian endian() const
    {
        return m_endian;
    }

    const FieldsList& members() const
    {
        return m_members;
    }

    Members membersList() const;


protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;

private:
    bool updateEndian();
    bool updateMembers();

    Endian m_endian = Endian_NumOfValues;
    FieldsList m_members;
};

} // namespace commsdsl
