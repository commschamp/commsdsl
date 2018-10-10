#pragma once

#include <limits>

#include "commsdsl/VariantField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class VariantFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    VariantFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    VariantFieldImpl(const VariantFieldImpl& other);
    using Members = VariantField::Members;

    const FieldsList& members() const
    {
        return m_members;
    }

    Members membersList() const;

    std::size_t defaultMemberIdx() const
    {
        return m_state.m_defaultIdx;
    }

    bool displayIdxReadOnlyHidden() const
    {
        return m_state.m_idxHidden;
    }

protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;

private:
    bool updateMembers();
    bool updateDefaultMember();
    bool updateIdxHidden();

    struct ReusableState
    {
        std::size_t m_defaultIdx = std::numeric_limits<std::size_t>::max();
        bool m_idxHidden = false;
    };

    ReusableState m_state;
    FieldsList m_members;
};

} // namespace commsdsl
