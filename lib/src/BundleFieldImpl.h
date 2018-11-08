#pragma once

#include "commsdsl/BundleField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class BundleFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    BundleFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    BundleFieldImpl(const BundleFieldImpl& other);
    using Members = BundleField::Members;

    const FieldsList& members() const
    {
        return m_members;
    }

    Members membersList() const;

protected:

    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl &other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;

private:
    bool updateMembers();

    FieldsList m_members;
};

} // namespace commsdsl
