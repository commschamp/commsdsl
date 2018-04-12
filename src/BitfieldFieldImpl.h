#pragma once

#include "bbmp/Endian.h"
#include "bbmp/BitfieldField.h"
#include "FieldImpl.h"

namespace bbmp
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

    const Members& members() const
    {
        return m_membersList;
    }

protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual std::size_t lengthImpl() const override;

private:
    bool updateEndian();
    bool updateMembers();

    Endian m_endian = Endian_NumOfValues;
    FieldsList m_members;
    Members m_membersList;
};

} // namespace bbmp
