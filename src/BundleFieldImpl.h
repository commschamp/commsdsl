#pragma once


#include "FieldImpl.h"

namespace bbmp
{

class BundleFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    BundleFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual ObjKind objKindImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool validateImpl() override;
    virtual std::size_t lengthImpl() const override;

private:
    using MembersList = std::vector<FieldImplPtr>;
    MembersList m_members;
};

} // namespace bbmp
