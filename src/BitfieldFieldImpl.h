#pragma once


#include "FieldImpl.h"

namespace bbmp
{

class BitfieldFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    BitfieldFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual ObjKind objKindImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool validateImpl() override;
    virtual std::size_t lengthImpl() const override;

private:
    FieldsList m_members;
};

} // namespace bbmp
